#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"

#include "fb_gfx.h"
#include "soc/soc.h"          //disable brownout problems
#include "soc/rtc_cntl_reg.h" //disable brownout problems
#include <WiFi.h>
#include "mqttwrapper.h"
#include <EspCam.h>
#include <settings.h>
//#include <ArduinoOTA.h>

MqttWrapper pubSubClient;
EspCam cam;
Settings settings;

unsigned long lastPhotoTaken = millis();

void connectWifi()
{
  // Wi-Fi connection
  WiFi.begin(settings.wifiSsid, settings.wifiPassword);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void disconnectWifi()
{
  WiFi.disconnect();
}

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector

  Serial.begin(9600);
  Serial.setDebugOutput(false);

  cam.Setup();

 /*  ArduinoOTA
      .onStart([]()
               {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_SPIFFS
        type = "filesystem";
      }

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type); })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      } });

  ArduinoOTA.begin(); */
}

void loop()
{
  unsigned long now = millis();

  if (now - lastPhotoTaken > settings.delayTime)
  {
    Photo photo = cam.CaptureFrame();

    connectWifi();

    if (pubSubClient.Connect())
    {
      if (photo.length > 0)
      {
        pubSubClient.Publish("log", "Sending photo...");
        pubSubClient.Publish("snapshot", photo.length, photo.buffer);
      }
      else
      {
        pubSubClient.Publish("log", "Unable to take photo...");
        // ESP.restart();
      }

      pubSubClient.Disconnect();
    }

    disconnectWifi();

    lastPhotoTaken = now;
  }

  pubSubClient.Loop();
  //ArduinoOTA.handle();
  delay(1);
}