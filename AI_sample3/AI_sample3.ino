#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <base64.h>
#include "ESP_I2S.h"
#include "esp_check.h"
#include "Wire.h"
#include "es8311.h"
#include "TCA9554.h"
#include "base64.hpp"

#define TAG "AI"

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
#define EXAMPLE_VOICE_VOLUME    (75)

I2SClass  i2s;
TCA9554   TCA(0x20);  // for I/O extensions
WiFiClientSecure    client;
HTTPClient          https;
DynamicJsonDocument doc(1024 * 64);

#define HISTORY_LEN   3
String	 queUsrHistory[HISTORY_LEN];
String	 queAstHistory[HISTORY_LEN];
int que_Put = 0, que_Get = 0;

// ===========================
// Enter your WiFi credentials
// ===========================
const char *ssid = "TP-Link";
const char *password = "0976511xxx";
const String  google_url = "https://speech.googleapis.com/v1/speech:recognize?key=AIzaSyAqz9vyAyGEQA1IQmq8rC27zMxqN9Wxxxx";
const String  openai_url = "https://api.openai.com/v1/responses";
const String  openai_api_key = "sk-proj-Fv1lJegzddhY0ldGJTshW_yKm-";
const String  google_tts_url = "https://texttospeech.googleapis.com/v1/text:synthesize?key=AIzaSyBX76S";

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
    delay(50);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  delay(500);
  //TextToSpeech_Json("你好！我是你的聊天機器人"); //你好！我是你的聊天機器人，隨時準備幫助你解答問題、聊天或提供資訊。有什麼我可以幫忙的嗎？
}

void loop() {
  int i, copyLen;
  uint8_t *tmpBuf;
  String *pStr;

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

      String text = SpeechToText_Json(auBuf, auBufLen);
      Serial.println("辨識結果: " + text);
      if (text != "")
      {
        String usrMsg = text;
        String convMsg = "";

        int tmp_Put = que_Put + 1;

        // pop from Queue
        while(que_Put != que_Get)
        {
          convMsg += " {\"role\":\"user\", \"content\":\"" + queUsrHistory[que_Get] + "\"},";
          convMsg += " {\"role\":\"assistant\", \"content\":\"" + queAstHistory[que_Get] + "\"},";
          if (++que_Get >= HISTORY_LEN) que_Get = 0;
        }

        convMsg += " {\"role\":\"user\", \"content\":\"" + text + "\"}";
        text = ChatBot(convMsg);

        // push to Queue
        if (tmp_Put >= HISTORY_LEN) tmp_Put = 0;
        if (tmp_Put == que_Get) {
          if (++que_Get >= HISTORY_LEN) que_Get = 0;
        }
        queUsrHistory[que_Put] = usrMsg;
        queAstHistory[que_Put] = text;
        que_Put = tmp_Put;

        //TextToSpeech_Json(text);
#if 1        
        String playText = text;
        int playLen = text.length();
        int idxMin = 1000, iOffset = 0;

        for(int n = 0; n < 10; n++)
        {
          int idx[3] = { text.indexOf("，", iOffset), text.indexOf("。", iOffset), text.indexOf("？", iOffset) };
          for(int i = 0; i < 3; i++)
          {
            // found && min
            if ( (idx[i] >= 0) && (idxMin >= idx[i]) ) idxMin = idx[i];
          }

          TextToSpeech_Json(playText.substring(iOffset, idxMin));
          if( idxMin + 10 >= playLen )
          {
            TextToSpeech_Json(playText.substring(idxMin+2, playLen));
            break;
          }

          iOffset = idxMin + 2; // next to
          idxMin = 1000;
        }
#endif
      }

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

String SpeechToText_Json(uint8_t *audioBuffer, size_t audioSize)
{
  String rc = "";
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
  Serial.println("To Speech to text.....");
  if (!https.begin(client, google_url)) {
    Serial.println("HTTP begin google_url failed");
    return rc;
  }
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
      rc = String(doc["results"][0]["alternatives"][0]["transcript"]);
      //Serial.println("辨識結果: " + rc);
    }
  } else {
    Serial.printf("HTTP POST failed: %s\n", https.errorToString(httpCode).c_str());
  }

  https.end();
  delay(1);
  return rc;
}

void TextToSpeech_Json(String text)
{
  if (text.length() < 2) return;

  String jsonPayload = "{";
    jsonPayload += "\"input\":{";
    jsonPayload += "\"text\":\"" + text + "\"},";
    jsonPayload += "\"voice\":{";
    jsonPayload += "\"languageCode\":\"zh-TW\",";
    jsonPayload += "\"modelName\":\"gemini-2.5-flash-tts\",";
    jsonPayload += "\"ssmlGender\":\"FEMALE\"},";
    jsonPayload += "\"audioConfig\":{";
    jsonPayload += "\"audioEncoding\":\"MP3\",";
    jsonPayload += "\"pitch\":0,";
    jsonPayload += "\"speakingRate\":1.0}";
    jsonPayload += "}";

  client.setInsecure(); // 簡化憑證驗證（實務可接受）
  Serial.println("To TTS....." + text);
  if (!https.begin(client, google_tts_url)) {
    Serial.println("HTTP begin google_tts_url failed");
    return;
  }
  https.addHeader("Content-Type", "application/json");

  int httpCode = https.POST(jsonPayload);
  if (httpCode > 0) {

#if 0
    char *netBuf = (char *)malloc(1024 * 128);
    int iBufLen = 0, iStart = -1, iEnd = -1;

    while (true)
    {
      if (client.available() > 0)
      {
        int len = client.read((uint8_t *)&netBuf[iBufLen], 1024 * 128 - iBufLen - 1);
        if (len > 0)
        {
          iBufLen += len;
          if( iStart < 0 )
          {
            for(int i = 0; i < iBufLen; i++)
            {
              if (memcmp(&netBuf[i], "audioContent\": \"", 16) == 0)
              {
                iStart = i + 16;
                break;
              }
            }
          }
          else if( iEnd < 0 )
          {
            for(int n = iStart; n < iBufLen; n++)
            {
              if (netBuf[n] == '"')
              {
                iEnd = n;
                break;
              }
            }
          }

          if (iStart != -1 && iEnd != -1) break;
        }
        else if (len <= 0)
        {
          break;
        }
      }

      delay(1);
    }

    // from iStart to iEnd-1
    //Serial.print(iStart); Serial.print(","); Serial.println(iEnd);
    //Serial.println(iBufLen);

    // Audio base64 內容有換行字元, 這會影響解碼的長度與結果, 所以要先篩掉\r\n
    unsigned int totalLen = 0;
    for(int i = iStart; i < iEnd; i++) {
      if (isalnum(netBuf[i]) || netBuf[i] == '+' || netBuf[i] == '/' || netBuf[i] == '=') 
        netBuf[totalLen++] = netBuf[i];
    }
    netBuf[totalLen] = 0;

    uint8_t* audioBuffer = (uint8_t*)malloc(totalLen * 0.8);
    unsigned int outLen = decode_base64((unsigned char*)netBuf, totalLen, audioBuffer);

    free(netBuf);
    i2s.playMP3(audioBuffer, outLen);
    free(audioBuffer);
    Serial.println(outLen);

#else

    String response = https.getString();
    //Serial.println(response);

    DeserializationError error = deserializeJson(doc, response);  
    if (error) {
      Serial.print(F("TTS JSON 解析失敗: "));
      Serial.println(error.f_str());
    } else {

      char* b64Data = (char*)doc["audioContent"].as<const char*>();

      char* src = b64Data;
      char* dst = b64Data;
      while (*src) {
          if (isalnum(*src) || *src == '+' || *src == '/' || *src == '=') {
              *dst++ = *src;
          }
          src++;
      }
      *dst = '\0'; // 重新結束字串

      unsigned int outLen = 0, totalLen = strlen(b64Data);
      //Serial.println(totalLen);

      uint8_t* audioBuffer = (uint8_t*)malloc(totalLen * 0.75);
      outLen = decode_base64((unsigned char*)b64Data, totalLen, audioBuffer);

      i2s.playMP3(audioBuffer, outLen);
      free(audioBuffer);
      //Serial.println(outLen);
    }
#endif
  } else {
    Serial.printf("HTTP POST failed: %s\n", https.errorToString(httpCode).c_str());
  }

  https.end();
  delay(1);
  return;
}

String ChatBot(String convsation)
{
  String rc = "";

  client.setInsecure(); // 簡化憑證驗證（實務可接受）
  Serial.println("To LLM.....");

  if (!https.begin(client, openai_url)) {
    Serial.println("HTTP begin openai_url failed");
    return rc;
  }

  https.addHeader("Authorization", "Bearer " + String(openai_api_key));
  https.addHeader("Content-Type", "application/json");

  String payload = "{"
    "\"model\":\"gpt-4.1-mini\","
    "\"input\":["
    " {\"role\":\"system\", \"content\":\"你是一個聊天機器人\"},"
    + convsation +
//    " {\"role\":\"user\", \"content\":[ {\"type\":\"input_text\",\"text\":\"" + question + "\"} ] },"
//    " {\"role\":\"assistant\", \"content\":[ {\"type\":\"input_text\",\"text\":\"" + question + "\"} ] }"
    "],"
    "\"max_output_tokens\":50,"
    "\"temperature\":0.7"
  "}";

  //Serial.println(payload);
  int httpCode = https.POST(payload);
  if (httpCode > 0) {

    String response = https.getString();
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
      Serial.print(F("JSON 解析失敗: "));
      Serial.println(error.f_str());
    } else {
      rc = String(doc["output"][0]["content"][0]["text"]);
      Serial.println(rc);
    }
  } else {
    Serial.printf("HTTP POST failed: %s\n", https.errorToString(httpCode).c_str());
  }

  https.end();
  delay(1);
  return rc;
}
