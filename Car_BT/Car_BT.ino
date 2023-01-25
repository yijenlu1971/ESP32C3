#include <Arduino.h>
#include <Wire.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>


#ifdef __U8G2__
#include <U8g2lib.h>
U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // EastRising 0.42" OLED
#endif


#define SERVICE_UUID          "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID   "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer *pServer = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;


#define SDA_PIN 	5
#define SCL_PIN 	6
#define MPU_ADDR  0x68
#define PWM_FREQUENCY     500

int16_t ax, ay, az, gx, gy, gz;     //定义三轴加速度，三轴陀螺仪的变量

hw_timer_t * timer0 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
int tmrCnt = 0, btCnt = 0;


//TB6612引脚定义
const int right_R1 = 20;  // D8
const int right_R2 = 21;	// D12
const int PWM_R = 4;      // D10
const int left_L1 = 7;		// D7
const int left_L2 = 8;		// D6
const int PWM_L = 3;      // D9

///////////////////////角度参数//////////////////////////////
//float angle_X;  //由加速度计算关于X轴的倾斜角度变量
//float angle_Y;  //由加速度计算关于Y轴的倾斜角度变量
float angle0 = -1; //实际测量出的角度（理想时是0度）
float Gyro_x, Gyro_y, Gyro_z;  //用于陀螺仪算出的各轴角速度
///////////////////////角度参数//////////////////////////////

///////////////////////Kalman_Filter////////////////////////////
float Q_angle = 0.001;  //陀螺仪噪声的协方差
float Q_gyro = 0.003;   //陀螺仪漂移噪声的协方差
float R_angle = 0.5;    //加速度计的协方差
char C_0 = 1;
float dt = 0.005; //dt的取值为滤波器采样时间
float K1 = 0.05;  //含有卡尔曼增益的函数，用于计算最优估计值的偏差
float K_0, K_1, t_0, t_1;
float angle_err;
float q_bias;    //陀螺仪漂移

float angle;
float angle_speed;

float Pdot[4] = { 0, 0, 0, 0};
float P[2][2] = {{ 1, 0 }, { 0, 1 }};
float PCt_0, PCt_1, E;
//////////////////////Kalman_Filter/////////////////////////

//////////////////////PID参数///////////////////////////////
double kp = 34, ki = 0, kd = 0.62;                      //角度环参数
double kp_speed = 3.6, ki_speed = 0.080, kd_speed = 0;  //速度环参数
double setp0 = 0; //角度平衡点
int PD_pwm;  //角度输出
float pwm1 = 0, pwm2 = 0;

//////////////////中断测速计数/////////////////////////////
#define PinA_left   0  //外部中断 D5
#define PinA_right  1  //外部中断 D4
volatile long count_right = 0;//用于计算霍尔编码器计算的脉冲值(volatile long类型是为了确保数值有效）
volatile long count_left = 0;

//////////////////////脉冲计算/////////////////////////
int rpluse = 0;
int lpluse = 0;
int pulseright, pulseleft;
////////////////////////////////PI变量参数//////////////////////////
float speeds_filterold = 0;
float positions = 0;
double PI_pwm;
int cc;
int speedout;
float speeds_filter;


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("onConnect");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("onDisconnect");
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);

        Serial.println();
        Serial.println("*********");
      }
    }
};

void setup() 
{
  Serial.begin(115200);

  //设置控制电机的引脚为输出状态
  pinMode(right_R1, OUTPUT);
  pinMode(right_R2, OUTPUT);
  pinMode(left_L1, OUTPUT);
  pinMode(left_L2, OUTPUT);
  pinMode(PWM_R, OUTPUT);
  pinMode(PWM_L, OUTPUT);

  analogWriteFrequency(PWM_FREQUENCY);
  analogWriteResolution(8);

  //赋初始状态值
  digitalWrite(right_R1, 1);
  digitalWrite(right_R2, 0);
  digitalWrite(left_L1,  1);
  digitalWrite(left_L2, 0);
  analogWrite(PWM_R, 0);
  analogWrite(PWM_L, 0);

  pinMode(PinA_left, INPUT);  //测速码盘输入
  pinMode(PinA_right, INPUT);

  //////////////////////////////////////////////////////////////////
  // Bluetooth LE
  BLEDevice::init("ESP32_Car");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setValue("Hello World says Neil");
  pService->start();

  BLEAdvertisementData advData;
  advData.setName("ESP32_Car");
  advData.setCompleteServices(BLEUUID(SERVICE_UUID));

  BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  pAdvertising->setAdvertisementData(advData);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMaxPreferred(0x12);
  pAdvertising->start();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
  //////////////////////////////////////////////////////////////////

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  delay(500);
  MPU6050_Init();
  delay(2);

#ifdef __U8G2__
  u8g2.begin();
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
#endif
  timer0 = timerBegin(0, 80, true);
  timerAttachInterrupt(timer0, &_callback0, true);
  timerAlarmWrite(timer0, 5000, true); // 5ms执行一次 (value in microseconds)
  timerAlarmEnable(timer0);

  //外部中断，用于计算车轮转速
  attachInterrupt(PinA_left, Code_left, CHANGE);  //PinA_left引脚的电平发生改变触发外部中断，执行子函数 Code_left
  attachInterrupt(PinA_right, Code_right, CHANGE); //PinA_right引脚的电平发生改变触发外部中断，执行子函数 Code_right
}

void loop() {
#ifdef __U8G2__
  char buf[128];
#endif
  MPU6050_getMotion6(&ax, &ay, &az, &gx, &gy, &gz); // I2C 不能放到中斷裡面執行

  if( ++tmrCnt >= 250 ) {
    tmrCnt = 0;
/*  Serial.println(angle);
    Serial.print(PD_pwm); Serial.print(", "); Serial.println(PI_pwm);
    Serial.print(pwm1); Serial.print(", "); Serial.println(pwm2);
*/
#ifdef __U8G2__
    u8g2.clearBuffer();
    sprintf(buf, "A: %7.2f", angle);
    u8g2.drawStr(0, 10, buf); // write something to the internal memory
    sprintf(buf, "S: %7.2f", speeds_filter);
    u8g2.drawStr(0, 20, buf);
    u8g2.sendBuffer();        // transfer internal memory to the display
#endif
  }

  if( ++btCnt >= 10 ) {
    btCnt = 0;

    if (deviceConnected) {
    }

    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
      Serial.println("start advertising");
      oldDeviceConnected = deviceConnected;
      pServer->startAdvertising(); // restart advertising
    }

    // connecting
    if (deviceConnected && !oldDeviceConnected) {
      // do stuff here on connecting
      oldDeviceConnected = deviceConnected;
    }
  }

  delay(4);
}

/////////////////////霍尔计数/////////////////////////
//左测速码盘计数
void Code_left() 
{
  count_left ++;
} 
//右测速码盘计数
void Code_right() 
{
  count_right ++;
} 
////////////////////脉冲计算///////////////////////
void countpluse()
{
  lpluse = count_left;
  count_left = 0;

  rpluse = count_right;
  count_right = 0;

  if( (pwm1 * pwm2) >= 0 )
  {
    //判断平衡车小车运动方向 如果后退时（PWM即电机电压为负）脉冲数为负数
    if( pwm1 < 0 ) {
      rpluse = -rpluse;
      lpluse = -lpluse;
    }
  }
  else
  {
    //小车运动方向判断 左旋转 右脉冲数为正数 左脉冲数为负数
    if( pwm1 < 0 ) {
      rpluse = rpluse;
      lpluse = -lpluse;      
    }
    else {
      rpluse = -rpluse;
      lpluse = lpluse;
    }
  }

  //每5ms进入中断时，脉冲数叠加
  pulseright += rpluse;
  pulseleft += lpluse;
}

/////////////////////////////////中断////////////////////////////
void ARDUINO_ISR_ATTR _callback0()
{
  countpluse(); //脉冲叠加子函数

  portENTER_CRITICAL_ISR(&timerMux);
  angle_calculate(ax, ay, az, gx, gy, gz, 
          dt, Q_angle, Q_gyro, R_angle, C_0, K1); //获取angle 角度和卡曼滤波
  portEXIT_CRITICAL_ISR(&timerMux);

  PD(); //角度环 PD控制
  anglePWM();

  if( ++cc >= 8 ) //5*8=40，40ms进入一次速度的PI算法
  {
    speedpiout();   
    cc = 0;
  }
}
///////////////////////////////////////////////////////////

/////////////////////////////倾角计算///////////////////////
void angle_calculate(int16_t ax, int16_t ay, int16_t az,
    int16_t gx, int16_t gy, int16_t gz,
    float dt, float Q_angle, float Q_gyro, float R_angle, float C_0, float K1)
{
  float Angle = -atan2(ay, az) * (180/ PI);  //車輪轉動的角度計算公式, 负号为方向处理
  Gyro_x = -gx / 131;           //車軸的角速度, 负号为方向处理
  Kalman_Filter(Angle, Gyro_x); //卡曼滤波
  Gyro_z = -gz / 131;           //垂直轴線的角速度
}
////////////////////////////////////////////////////////////////

///////////////////////////////KalmanFilter/////////////////////
void Kalman_Filter(double angle_m, double gyro_m)
{
  angle += (gyro_m - q_bias) * dt;          //先验估计
  angle_err = angle_m - angle;
  
  Pdot[0] = Q_angle - P[0][1] - P[1][0];    //先验估计误差协方差的微分
  Pdot[1] = - P[1][1];
  Pdot[2] = - P[1][1];
  Pdot[3] = Q_gyro;
  
  P[0][0] += Pdot[0] * dt;    //先验估计误差协方差微分的积分
  P[0][1] += Pdot[1] * dt;
  P[1][0] += Pdot[2] * dt;
  P[1][1] += Pdot[3] * dt;
  
  //矩阵乘法的中间变量
  PCt_0 = C_0 * P[0][0];
  PCt_1 = C_0 * P[1][0];
  //分母
  E = R_angle + C_0 * PCt_0;
  //增益值
  K_0 = PCt_0 / E;
  K_1 = PCt_1 / E;
  
  t_0 = PCt_0;  //矩阵乘法的中间变量
  t_1 = C_0 * P[0][1];
  
  P[0][0] -= K_0 * t_0;    //后验估计误差协方差
  P[0][1] -= K_0 * t_1;
  P[1][0] -= K_1 * t_0;
  P[1][1] -= K_1 * t_1;
  
  q_bias += K_1 * angle_err;    //后验估计
  angle_speed = gyro_m - q_bias;   //输出值的微分，得出最优角速度
  angle += K_0 * angle_err; ////后验估计，得出最优角度
}

//////////////////角度PD////////////////////
void PD()
{
  PD_pwm = kp * (angle + angle0) + kd * angle_speed; //PD 角度环控制
}

//////////////////速度PI////////////////////
void speedpiout()
{
  float speeds = (pulseleft + pulseright) * 1.0;	//车速 脉冲值
  pulseright = pulseleft = 0;      //清零
  speeds_filterold *= 0.7;         //一阶互补滤波
  speeds_filter = speeds_filterold + speeds * 0.3;
  speeds_filterold = speeds_filter;
  positions += speeds_filter;
  positions = constrain(positions, -3550, 3550);  //抗积分饱和
  PI_pwm = ki_speed * (setp0 - positions) + kp_speed * (setp0 - speeds_filter);	//速度环控制 PI
}
//////////////////速度PI////////////////////


////////////////////////////PWM终值/////////////////////////////
void anglePWM()
{
  pwm2 = -PD_pwm - PI_pwm ; //赋给电机PWM的最终值
  pwm1 = -PD_pwm - PI_pwm ;

  //限定PWM值不能超过255
  if( pwm1 > 255 )  pwm1 = 255;
  if( pwm1 < -255 ) pwm1 = -255;
  if( pwm2 > 255 )  pwm2 = 255;
  if( pwm2 < -255 ) pwm2 = -255;

  //自平衡小车倾斜角度大于45度，电机就会停转
  if( angle > 80 || angle < -80 ) pwm1 = pwm2 = 0;

  //根据PWM的正负来确定电机的转向和转速
  if( pwm2 >= 0 )
  {
    digitalWrite(left_L1, LOW);
    digitalWrite(left_L2, HIGH);
    analogWrite(PWM_L, pwm2);
  }
  else
  {
    digitalWrite(left_L1, HIGH);
    digitalWrite(left_L2, LOW);
    analogWrite(PWM_L, -pwm2);
  }

  if( pwm1 >= 0 )
  {
    digitalWrite(right_R1, LOW);
    digitalWrite(right_R2, HIGH);
    analogWrite(PWM_R, pwm1);
  }
  else
  {
    digitalWrite(right_R1, HIGH);
    digitalWrite(right_R2, LOW);
    analogWrite(PWM_R, -pwm1);
  }
}

void MPU6050_Init(){
  // REGISTER 0x6B/REGISTER 107:Power Management 1
  Wire.beginTransmission(MPU_ADDR); //This is the I2C address of the MPU (b1101000/b1101001 for AC0 low/high datasheet Sec. 9.2)
  Wire.write(0x6B); //Accessing the register 6B/107 - Power Management (Sec. 4.30) 
  Wire.write(0b00000000); //Setting SLEEP register to 0, using the internal 8 Mhz oscillator
  Wire.endTransmission();

  // REGISTER 0x1b/REGISTER 27:Gyroscope Configuration
  Wire.beginTransmission(MPU_ADDR); //I2C address of the MPU
  Wire.write(0x1B); //Accessing the register 1B - Gyroscope Configuration (Sec. 4.4) 
  Wire.write(0x00000000); //Setting the gyro to full scale +/- 250deg./s (转化为rpm:250/360 * 60 = 41.67rpm) 最高可以转化为2000deg./s 
  Wire.endTransmission();
  
  // REGISTER 0x1C/REGISTER 28:ACCELEROMETER CONFIGURATION
  Wire.beginTransmission(MPU_ADDR); //I2C address of the MPU
  Wire.write(0x1C); //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5) 
  Wire.write(0b00000000); //Setting the accel to +/- 2g（if choose +/- 16g，the value would be 0b00011000）
  Wire.endTransmission();
}

void MPU6050_getMotion6(int16_t *ax, int16_t *ay, int16_t *az, int16_t *gx, int16_t *gy, int16_t *gz)
{
  int len;
  long accelX, accelY, accelZ;
  long gyroX, gyroY, gyroZ;

  // REGISTER 0x3B~0x40/REGISTER 59~64
  Wire.beginTransmission(MPU_ADDR); //I2C address of the MPU
  Wire.write(0x3B); //Starting register for Accel Readings
  Wire.endTransmission();
  len = Wire.requestFrom(MPU_ADDR, 6); //Request Accel Registers (3B - 40)

//  if( len < 6 ) { Serial.print("Accel:"); Serial.println(len);}
  // 使用了左移<<和位运算|。Wire.read()一次读取1bytes，并在下一次调用时自动读取下一个地址的数据
  accelX = (Wire.read() << 8) | Wire.read(); //Store first two bytes into accelX （自动存储为定义的long型值）
  accelY = (Wire.read() << 8) | Wire.read(); //Store middle two bytes into accelY
  accelZ = (Wire.read() << 8) | Wire.read(); //Store last two bytes into accelZ

  // REGISTER 0x43~0x48/REGISTER 67~72
  Wire.beginTransmission(MPU_ADDR); //I2C address of the MPU
  Wire.write(0x43); //Starting register for Gyro Readings
  Wire.endTransmission();
  len = Wire.requestFrom(MPU_ADDR, 6); //Request Gyro Registers (43 ~ 48)

//  if( len < 6 ) { Serial.print("Gyro:"); Serial.println(len);}
  gyroX = (Wire.read() << 8) | Wire.read(); //Store first two bytes into accelX
  gyroY = (Wire.read() << 8) | Wire.read(); //Store middle two bytes into accelY
  gyroZ = (Wire.read() << 8) | Wire.read(); //Store last two bytes into accelZ

  portENTER_CRITICAL_ISR(&timerMux);
  *ax = accelX; *ay = accelY; *az = accelZ;
  *gx = gyroX; *gy = gyroY; *gz = gyroZ;
  portEXIT_CRITICAL_ISR(&timerMux);
  len = len;
}
