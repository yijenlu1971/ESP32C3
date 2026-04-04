#include <Arduino.h>
#include "Arduino.h"

//#define __MICS_ENABLE__

#define MQ_RLOAD    1.0
#define MICS_RLOAD  10.0

#define RXPIN     18    // GPIO 18 => RX for Serial1
#define TXPIN     17    // GPIO 17 => TX for Serial1
#define BUF_SIZE  32
#define ADC_NUM   3

const int adcPin[ADC_NUM] = {4, 5, 6};   // GPIO-1 屬於 ADC1_CH0, 以此類推
const int ledPin = 2;
int tmrCnt = 0, bufLen = 0, prvVal[ADC_NUM] = {0}, prvVol[ADC_NUM] = {0}, prvZE08 = 0;
unsigned int prvPPM[ADC_NUM] = {(unsigned int)-1, (unsigned int)-1, (unsigned int)-1};
bool bOn = false;
unsigned char zeBuf[BUF_SIZE];

float MQ4_get_ppm(unsigned int miniVol);
float MQ7_get_ppm(unsigned int miniVol);
float MQ135_get_ppm(unsigned int miniVol);
void ZE08_Proc();

void setup() {

  pinMode(ledPin, OUTPUT);
  Serial.begin(115200); // for debug
  Serial1.begin(9600, SERIAL_8N1, RXPIN, TXPIN);  // Rx = 18, Tx = 17 will work for ESP32-S3

  //set the resolution to 12 bits (0-4095)
  analogReadResolution(12);

  // 設定衰減度 (11dB 讓輸入範圍達到約 0-3.1V)
  // ADC_0db: 0-1.1V, ADC_2_5db: 0-1.5V, ADC_6db: 0-2.2V, ADC_11db: 0-3.1V
  //analogSetAttenuation(ADC_11db);
}

void loop() {
#ifdef __MICS_ENABLE__
  String dev[3] = {"CO", "NH3", "NO2"};
#else
  String dev[3] = {"MQ4 CH4", "MQ7 CO", "MQ135"};
#endif
  int i, analogValue[ADC_NUM], analogVolts[ADC_NUM];
  unsigned int temp;
  float curPPM;

  for(i = 0; i < ADC_NUM; i++)
  {
    // This function will return analog raw value (non-calibrated)
    // analogValue[i] = analogRead(adcPin[i]);
    /*if( prvVal[i] != analogValue[i] )
    {
      prvVal[i] = analogValue[i];
      Serial.printf("%d ADC Value=%d\n", i, analogValue[i]);
    }*/

    // This function is used to get ADC raw value for a given pin/ADC channel and 
    // convert it to calibrated result in millivolts
    analogVolts[i] = analogReadMilliVolts(adcPin[i]);
    if( prvVol[i] != analogVolts[i] )
    {
      prvVol[i] = analogVolts[i];
      //Serial.printf("%d MiniVolt=%d\n", i, analogVolts[i]);
      //Serial.printf("%d R=%7.3f\n", i, 5000.0f/analogVolts[i] - 1.0); // Clean-air, 計算R0

#ifdef __MICS_ENABLE__
      curPPM = MiCS6814_get_ppm(i, analogVolts[i]);
      temp = (unsigned int) (curPPM * 1000);
#else
      if( i == 0 ) curPPM = MQ4_get_ppm(analogVolts[i]);
      if( i == 1 ) curPPM = MQ7_get_ppm(analogVolts[i]);
      if( i == 2 ) curPPM = MQ135_get_ppm(analogVolts[i]);
      temp = (unsigned int) curPPM;
#endif

      if( temp != prvPPM[i] )
      {
        prvPPM[i] = temp;
#ifdef __MICS_ENABLE__
        Serial.printf("%s = %.3f ppm\n", dev[i].c_str(), curPPM);
#else
        Serial.printf("%s = %d ppm\n", dev[i].c_str(), temp);
#endif
      }
    }
  }

  ZE08_Proc();

  if( ++tmrCnt >= 2 )
  {
    digitalWrite(ledPin, bOn ? LOW : HIGH);
    bOn = !bOn;
    tmrCnt = 0;
  }

  delay(500);  // delay in between reads for clear read from serial
}

void ZE08_Proc()
{
  unsigned int i, n, sum;

  if ( Serial1.available() )
  {
    while ( Serial1.available() ) { zeBuf[bufLen++] = Serial1.read(); }

    if( bufLen >= 9 )
    {
      for(i = 0; i < bufLen; i++)
      {
        if( zeBuf[i] == 0xFF )
        {
          if( i + 8 <= bufLen )
          {
            sum = 0;
            for(n = i + 1; n < i + 8; n++) 
            {
              sum += zeBuf[n]; // Checksum
//              Serial.print(zeBuf[n], HEX); Serial.print(',');
            }
//            Serial.println(zeBuf[n], HEX);

            sum = (~sum & 0xFF) + 1;
            if( sum == zeBuf[i + 8] )
            {
              sum = (zeBuf[i+4] << 8) + zeBuf[i+5];
              if( sum != prvZE08 )
              {
                prvZE08 = sum;
                Serial.printf("ZE08 CH2O = %d ppb\n", sum);
              }
            }
            
            for(n = i + 9; n < bufLen; n++) zeBuf[n-(i+9)] = zeBuf[n];
            bufLen -= 9;
          }
          else
          {
            return;
          }
        }
      }
    }
  }
}

float MQ4_get_ppm(unsigned int miniVol)
{
  const float rz = 4.4, parA = 991.4, parB = 2.88;

  float rs = (5000 - miniVol) * MQ_RLOAD / miniVol;
  float ratio = rs / rz;
  return (parA * pow(ratio, -parB));
}
float MQ7_get_ppm(unsigned int miniVol)
{
  const float rz = 1.9, parA = 99.04, parB = 1.568;

  float rs = (5000 - miniVol) * MQ_RLOAD / miniVol;
  float ratio = rs / rz;
  return (parA * pow(ratio, -parB));
}
float MQ135_get_ppm(unsigned int miniVol)
{
  const float rz = 3.59, parA = 116.602, parB = 2.769;

  float rs = (5000 - miniVol) * MQ_RLOAD / miniVol;
  float ratio = rs / rz;
  return (parA * pow(ratio, -parB));
}

float MiCS6814_get_ppm(unsigned char type, unsigned int miniVol)
{
  const float rz[3] = {2.13, 2.13, 300.0};
  float rs = (5000 - miniVol) * MICS_RLOAD / miniVol;
  float ratio = rs / rz[type];

  if( type == 0 ) // CO
  {
    return pow (ratio, -1.179) * 4.385;
  }
  else if( type == 1 ) // NH3
  {
    return pow (ratio, -1.67) / 1.47;
  }
  else if( type == 2 ) // NO2
  {
    return pow (ratio, 1.007) / 6.855;
  }

  return -1.0;
}