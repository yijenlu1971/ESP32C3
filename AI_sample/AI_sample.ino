#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "ESP_I2S.h"
#include "esp_check.h"
#include "Wire.h"
#include "es8311.h"
#include "TCA9554.h"
#include "base64.h"


#define I2C_SDA         8
#define I2C_SCL         7

#define I2S_NUM         I2S_NUM_0 
#define I2S_MCK_PIN     44
#define I2S_BCK_PIN     13
#define I2S_LRCK_PIN    15
#define I2S_DOUT_PIN    16
#define I2S_DIN_PIN     14

#define EXAMPLE_SAMPLE_RATE     (16000)
#define EXAMPLE_MCLK_MULTIPLE   (256)  // If not using 24-bit data width, 256 should be enough
#define EXAMPLE_MCLK_FREQ_HZ    (EXAMPLE_SAMPLE_RATE * EXAMPLE_MCLK_MULTIPLE)
#define EXAMPLE_VOICE_VOLUME    (90)

I2SClass  i2s;
TCA9554   TCA(0x20);  // for I/O extensions
WiFiClientSecure    client;
HTTPClient          https;
DynamicJsonDocument doc(4096);

// ===========================
// Enter your WiFi credentials
// ===========================
const char *ssid = "TP-Link";
const char *password = "0976511xxx";
const String url = "https://speech.googleapis.com/v1/speech:recognize?key=AIzaSyAqz9vyAyGEQA1IQmq8rC27zMxqN9Wxxxx";

// ===========================
// for wave
#define BUF_QSIZE   15
#define REC_SEC     1

int buf_Idx = 0;
uint8_t *auBuf, *wav_buffer[BUF_QSIZE];
size_t auBufLen = 0, wav_size[BUF_QSIZE];

// ===========================
// for button I/O
const byte buttonPin = 0; 
volatile byte ioState = LOW;
unsigned long btnDbTime;
int iRecWav = 0;

void BtnChg() {
  unsigned long currTime = millis();
  volatile byte currState = digitalRead(buttonPin);

  if( (currTime > btnDbTime + 50) && (ioState != currState) )
  {
    btnDbTime = currTime;
    ioState = currState;
    if (!ioState) iRecWav++;
  }
}

static esp_err_t es8311_codec_init(void) {
  es8311_handle_t es_handle = es8311_create(I2C_NUM_0, ES8311_ADDRRES_0);
  ESP_RETURN_ON_FALSE(es_handle, ESP_FAIL, TAG, "es8311 create failed");
  const es8311_clock_config_t es_clk = {
    .mclk_inverted = false,
    .sclk_inverted = false,
    .mclk_from_mclk_pin = true,
    .mclk_frequency = EXAMPLE_MCLK_FREQ_HZ,
    .sample_frequency = EXAMPLE_SAMPLE_RATE
  };

  ESP_ERROR_CHECK(es8311_init(es_handle, &es_clk, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16));
  ESP_RETURN_ON_ERROR(es8311_voice_volume_set(es_handle, EXAMPLE_VOICE_VOLUME, NULL), TAG, "set es8311 volume failed");
  ESP_RETURN_ON_ERROR(es8311_microphone_config(es_handle, false), TAG, "set es8311 microphone failed");
  ESP_RETURN_ON_ERROR(es8311_microphone_gain_set(es_handle, ES8311_MIC_GAIN_12DB), TAG, "set es8311 microphone gain failed");

  return ESP_OK;
}

void setup() {
  Serial.begin(115200);

  // for button I/O
  pinMode(buttonPin, INPUT);
  ioState = digitalRead(buttonPin);
  btnDbTime = millis();
  attachInterrupt(digitalPinToInterrupt(buttonPin), BtnChg, CHANGE);

  Wire.begin(I2C_SDA, I2C_SCL);
  //// for I/O extensions
  TCA.begin();
  TCA.pinMode1(7, OUTPUT); // EXIO7 - PA control
  TCA.write1(7, HIGH);
  Serial.println(TCA.read1(7));

  //// for I2S ES8311
  i2s.setPins(I2S_BCK_PIN, I2S_LRCK_PIN, I2S_DOUT_PIN, I2S_DIN_PIN, I2S_MCK_PIN);
  // Initialize the I2S bus in standard mode
  Serial.println(
    i2s.begin(I2S_MODE_STD, EXAMPLE_SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH) ?
    "I2S Initialized" : "Failed to initialize I2S bus!" );
  es8311_codec_init();

  //// WiFi setting
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void loop() {
  int i, copyLen;
  uint8_t *tmpBuf;

  if (iRecWav > 0)
  {
    if (iRecWav == 1)
    {
      Serial.println("RecWave...");
      wav_buffer[buf_Idx] = i2s.recordWAV(REC_SEC, &wav_size[buf_Idx]);
      
      if( wav_buffer[buf_Idx] != NULL)
      {
        auBufLen += wav_size[buf_Idx];

        if (++buf_Idx >= BUF_QSIZE) iRecWav++;
      }
    }
    else if (iRecWav == 2)
    {
      Serial.println("Stop RecWave!!!");
      auBuf = (uint8_t *)malloc(auBufLen);

      copyLen = 0;
      for(i = 0; i < buf_Idx; i++)
      {
        tmpBuf = wav_buffer[i];
        memcpy( &auBuf[copyLen], &tmpBuf[44], wav_size[i]-44 );
        copyLen += (wav_size[i]-44);

        //i2s.playWAV(wav_buffer[i], wav_size[i]);
        free(wav_buffer[i]);
      }

      SpeechToText_Json(auBuf, auBufLen);
      auBufLen = 0;
      buf_Idx = 0;
      iRecWav = 0;
    }
  }
  else
  {
    /*size_t freeHeap = ESP.getFreeHeap();
    Serial.printf("剩餘總 Heap: %d bytes (%.2f KB)\n", freeHeap, freeHeap / 1024.0);
    size_t maxAlloc = ESP.getMaxAllocHeap();
    Serial.printf("最大可分配區塊: %d bytes (%.2f KB)\n", maxAlloc, maxAlloc / 1024.0);*/

    delay(1000);
  }
}

void SpeechToText_Json(uint8_t *audioBuffer, size_t audioSize)
{
  String audioBase64 = base64::encode(audioBuffer, audioSize);
  free(audioBuffer);

  String jsonPayload = "{";
    jsonPayload += "\"config\":{";
    jsonPayload += "\"encoding\":\"LINEAR16\",";
    jsonPayload += "\"sampleRateHertz\":16000,";
    jsonPayload += "\"audioChannelCount\":2,";
    jsonPayload += "\"enableSeparateRecognitionPerChannel\":false,";
    jsonPayload += "\"languageCode\":\"zh-TW\""; // en-us or zh-TW
    jsonPayload += "},";
    jsonPayload += "\"audio\":{";
    jsonPayload += "\"content\":\"" + audioBase64 + "\"";
    jsonPayload += "}";
    jsonPayload += "}";
  
  client.setInsecure(); // 簡化憑證驗證（實務可接受）
  https.begin(client, url);
  https.addHeader("Content-Type", "application/json");

  int httpCode = https.POST(jsonPayload);
  if (httpCode > 0) {
    String response = https.getString();
    //Serial.println(response);

    DeserializationError error = deserializeJson(doc, response);
    if (error) {
      Serial.print(F("JSON 解析失敗: "));
      Serial.println(error.f_str());
    } else {
      const char* transcript = doc["results"][0]["alternatives"][0]["transcript"];
      String text = String(transcript);
      Serial.println("辨識結果: " + text);
    }
  } else {
    Serial.println("HTTP request failed");
  }

  https.end();
}
