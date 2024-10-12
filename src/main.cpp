/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-cam-video-streaming-web-server-camera-home-assistant/

  IMPORTANT!!!
   - Select Board "AI Thinker ESP32-CAM"
   - GPIO 0 must be connected to GND to upload a sketch
   - After connecting GPIO 0 to GND, press the ESP32-CAM on-board RESET button to put your board in flashing mode

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h"          //disable brownout problems
#include "soc/rtc_cntl_reg.h" //disable brownout problems
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <SPIFFS.h>
#include <FS.h>
#include <PubSubClient.h>
#include "ArduinoJson.h"

// Replace with your network credentials
const char *ssid = "Internetz";
const char *password = "javajuice";

#define PART_BOUNDARY "123456789000000000000987654321"

#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
#define FILE_PHOTO "/photo.jpg"

WiFiClient espClient;
AsyncWebServer server(80);
PubSubClient client(espClient);

unsigned long ota_progress_millis = 0;
unsigned long lastPhotoTaken = millis();

void onOTAStart()
{
  // Log when OTA has started
  Serial.println("OTA update started!");
  // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final)
{
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000)
  {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success)
{
  // Log when OTA has finished
  if (success)
  {
    Serial.println("OTA update finished successfully!");
  }
  else
  {
    Serial.println("There was an error during OTA update!");
  }
  // <Add your own code here>
}

void mqttInit()
{
  client.setServer("192.168.4.202", 1883);
}

bool mqttConnect()
{
  while (!client.connected())
  {
    Serial.println("Connecting to message broker...");
    client.connect("VOC", "mosquitto", "86LSwHF0JSAf");

    delay(250);
  }

  return client.connected();
}

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector

  Serial.begin(9600);
  Serial.setDebugOutput(false);

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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    ESP.restart();
  }
  else
  {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }

  if (psramFound())
  {
    // config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  }
  else
  {
    // config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  config.frame_size = FRAMESIZE_UXGA;

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  // Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.print("Camera Stream Ready! Go to: http://");
  Serial.print(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
    camera_fb_t * fb = esp_camera_fb_get();
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = NULL;
    char * part_buf[64];
  
    _jpg_buf_len = fb->len;
    _jpg_buf = fb->buf;
      
    AsyncWebServerResponse *response = request->beginResponse(200, "image/jpg", _jpg_buf, _jpg_buf_len);
    request->send(response); });

  server.on("/snapshot", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, FILE_PHOTO, "image/jpg", false); });

  ElegantOTA.begin(&server); // Start ElegantOTA
  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);

  mqttInit();

  server.begin();
  Serial.println("HTTP server started");
}

// Check if photo capture was successful
bool checkPhoto(fs::FS &fs)
{
  File f_pic = fs.open(FILE_PHOTO);
  unsigned int pic_sz = f_pic.size();
  return (pic_sz > 100);
}

void loop()
{
  unsigned long now = millis();

  ElegantOTA.loop();
  if (now - lastPhotoTaken > 1000)
  {

    if (mqttConnect())
    {
      camera_fb_t *fb = NULL;
      fb = esp_camera_fb_get();
      if (fb)
      {
        client.publish("growtent/snapshot", fb->buf, fb->len);
        client.loop();
      }
      else
      {
        Serial.println("Camera capture failed");
        return;
      }
    }

    lastPhotoTaken = now;
  }
  delay(1);
}