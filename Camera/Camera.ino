#include "esp_camera.h"
#include <WiFi.h>
#include <Arduino_GFX_Library.h>
#include "TCA9554.h"

//#define __STREAM_ENABLE__

#define PWDN_GPIO_NUM   -1
#define RESET_GPIO_NUM  -1
#define XCLK_GPIO_NUM   38
#define SIOD_GPIO_NUM   8
#define SIOC_GPIO_NUM   7

#define Y9_GPIO_NUM     21
#define Y8_GPIO_NUM     39
#define Y7_GPIO_NUM     40
#define Y6_GPIO_NUM     42
#define Y5_GPIO_NUM     46
#define Y4_GPIO_NUM     48
#define Y3_GPIO_NUM     47
#define Y2_GPIO_NUM     45
#define VSYNC_GPIO_NUM  17
#define HREF_GPIO_NUM   18
#define PCLK_GPIO_NUM   41

#define GFX_BL 6 // default backlight pin, you may replace DF_GFX_BL to actual backlight pin

// ===========================
// Enter your WiFi credentials
// ===========================
const char *ssid = "TP-Link";
const char *password = "0976511713";
// ===========================
// for button I/O
const byte buttonPin = 0; 
volatile byte ioState = LOW;
unsigned long btnDbTime;

#define LCD_QSPI_CS   12
#define LCD_QSPI_CLK  5
#define LCD_QSPI_D0   1
#define LCD_QSPI_D1   2
#define LCD_QSPI_D2   3
#define LCD_QSPI_D3   4

#define ROTATION      0

Arduino_DataBus *bus = new Arduino_ESP32QSPI(LCD_QSPI_CS, LCD_QSPI_CLK, LCD_QSPI_D0, LCD_QSPI_D1, LCD_QSPI_D2, LCD_QSPI_D3);
Arduino_GFX *g = new Arduino_AXS15231B(bus, -1 /* RST */, 0 /* rotation */, false, 320, 480);
Arduino_Canvas *gfx = new Arduino_Canvas(320, 480, g, 0, 0, ROTATION);

void startCameraServer();
void BtnChg();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // for button I/O
  pinMode(buttonPin, INPUT);
  ioState = digitalRead(buttonPin);
  btnDbTime = millis();
  attachInterrupt(digitalPinToInterrupt(buttonPin), BtnChg, CHANGE);

  // 背光
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;
  config.frame_size = FRAMESIZE_UXGA;
#ifdef __STREAM_ENABLE__
  config.pixel_format = PIXFORMAT_JPEG; // for streaming (JPEG 格式)
#else
  config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
#endif
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) { // Yes, PSRAM 存在
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_QVGA; //FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);        // flip it back
    s->set_brightness(s, 1);   // up the brightness just a bit
    s->set_saturation(s, -2);  // lower the saturation
  }
  else
  {
    // 水平翻轉 (左右反向時使用)
    //s->set_hmirror(s, 1); // 1 為開啟翻轉, 0 為關閉
    // 垂直翻轉 (上下顛倒時使用)
    s->set_vflip(s, 1);   // 1 為開啟翻轉, 0 為關閉
  }

  // drop down frame size for higher initial frame rate
  if (config.pixel_format == PIXFORMAT_JPEG) {
    s->set_framesize(s, FRAMESIZE_QVGA);  // 320 x 240
  }

#ifdef __STREAM_ENABLE__
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
#endif

  // Init Display
  if ( !gfx->begin() )
  {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(RGB565_BLACK);

  gfx->setCursor(10, 10);
  gfx->setTextColor(RGB565_YELLOW);
  gfx->println("Han-Ya");
  gfx->flush();
  delay(500); // 0.5 seconds
}

void loop() {

#ifndef __STREAM_ENABLE__
  camera_fb_t *fb = esp_camera_fb_get();

  if (!fb)
  {
    Serial.println("Camera capture failed");
  } 
  else 
  {
    // 取得像素指針 (uint16_t 是因為 )
    for(int i = 0; i < fb->len; i+=2) {
      // 重要！OV2640 的 RGB565 通常需要高低位元組交換，RGB565 佔 2 bytes
      // 交換兩個 byte
      uint8_t temp = fb->buf[i];
      fb->buf[i] = fb->buf[i+1];
      fb->buf[i+1] = temp;
    }

    //Serial.printf("f:%d w:%d h:%d l:%d\n", fb->format, fb->width, fb->height, fb->len);
    gfx->draw16bitRGBBitmap(0, 40, (uint16_t*)fb->buf, fb->width, fb->height);
    gfx->flush();

    esp_camera_fb_return(fb);
    fb = NULL;
  }
#endif

  // Do nothing. Everything is done in another task by the web server
  delay(200);
}

void BtnChg() {
  unsigned long currTime = millis();
  volatile byte currState = digitalRead(buttonPin);

  if( (currTime > btnDbTime + 50) && (ioState != currState) )
  {
    btnDbTime = currTime;
    ioState = currState;
    Serial.println(currState);
  }
}
