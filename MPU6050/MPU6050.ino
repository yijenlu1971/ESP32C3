#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#define SDA_PIN 5
#define SCL_PIN 6

// 如何獲取小車動態的方式，採用最多的就是通過加速度計與陀螺儀
// 測試出小車的三軸加速度和三軸陀螺儀的數值，並在COM port列印出来。
// 其中加速度範圍是+-2g；陀螺儀範圍是+-250°/S

#define MPU_ADDR  0x68

U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // EastRising 0.42" OLED

long accelX, accelY, accelZ;      // 定义为全局变量，可直接在函数内部使用
float gForceX, gForceY, gForceZ;
 
long gyroX, gyroY, gyroZ;
float rotX, rotY, rotZ;
 
void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(200000);

  u8g2.begin();
  setupMPU();
}
 
void loop() {
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font

  recordAccelRegisters();
  recordGyroRegisters();
  printData();

  delay(100);
}
 
void setupMPU(){
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
 
void recordAccelRegisters() {
  // REGISTER 0x3B~0x40/REGISTER 59~64
  Wire.beginTransmission(MPU_ADDR); //I2C address of the MPU
  Wire.write(0x3B); //Starting register for Accel Readings
  Wire.endTransmission();
  Wire.requestFrom(MPU_ADDR, 6); //Request Accel Registers (3B - 40)

  // 使用了左移<<和位运算|。Wire.read()一次读取1bytes，并在下一次调用时自动读取下一个地址的数据
  while(Wire.available() < 6);  // Waiting for all the 6 bytes data to be sent from the slave machine （必须等待所有数据存储到缓冲区后才能读取） 
  accelX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX （自动存储为定义的long型值）
  accelY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  accelZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ
  processAccelData();
}
 
void processAccelData(){
  gForceX = accelX / 16384.0;     //float = long / float
  gForceY = accelY / 16384.0; 
  gForceZ = accelZ / 16384.0;
}
 
void recordGyroRegisters() 
{
  // REGISTER 0x43~0x48/REGISTER 67~72
  Wire.beginTransmission(MPU_ADDR); //I2C address of the MPU
  Wire.write(0x43); //Starting register for Gyro Readings
  Wire.endTransmission();
  Wire.requestFrom(MPU_ADDR, 6); //Request Gyro Registers (43 ~ 48)
  while(Wire.available() < 6);
  gyroX = Wire.read()<<8|Wire.read(); //Store first two bytes into accelX
  gyroY = Wire.read()<<8|Wire.read(); //Store middle two bytes into accelY
  gyroZ = Wire.read()<<8|Wire.read(); //Store last two bytes into accelZ
  processGyroData();
}
 
void processGyroData() {
  rotX = gyroX / 131.0;
  rotY = gyroY / 131.0; 
  rotZ = gyroZ / 131.0;
}
 
void printData() {
  char buf[128];

  Serial.print("Gyro (deg)");
  Serial.print(" X=");
  Serial.print(rotX);
  Serial.print(" Y=");
  Serial.print(rotY);
  Serial.print(" Z=");
  Serial.print(rotZ);
  Serial.print(" Accel (g)");
  Serial.print(" X=");
  Serial.print(gForceX);
  Serial.print(" Y=");
  Serial.print(gForceY);
  Serial.print(" Z=");
  Serial.println(gForceZ);

  // (x,y)
  sprintf(buf, "D:%5.2f,%5.2f", rotX, rotY);
  u8g2.drawStr(0, 10, buf);  // write something to the internal memory
  sprintf(buf, "    %5.2f", rotZ);
  u8g2.drawStr(0, 20, buf);  // write something to the internal memory
  sprintf(buf, "G:%5.2f.%5.2f", gForceX, gForceY);
  u8g2.drawStr(0, 30, buf);  // write something to the internal memory
  sprintf(buf, "    %5.2f", gForceZ);
  u8g2.drawStr(0, 40, buf);  // write something to the internal memory
  u8g2.sendBuffer();          // transfer internal memory to the display
}
