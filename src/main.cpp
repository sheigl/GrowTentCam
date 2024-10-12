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

#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"

#include "fb_gfx.h"
#include "soc/soc.h"          //disable brownout problems
#include "soc/rtc_cntl_reg.h" //disable brownout problems
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "mqttwrapper.h"
#include <EspCam.h>
#include "elegantOTAWrapper.h"
#include <settings.h>

AsyncWebServer server(80);
MqttWrapper pubSubClient;
EspCam cam;
ElegantOTAWrapper elegantOTA(&server);
Settings settings;

unsigned long lastPhotoTaken = millis();

void initWifi()
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

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector

  Serial.begin(9600);
  Serial.setDebugOutput(false);

  initWifi();
  elegantOTA.setup();
  cam.Setup();

  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  unsigned long now = millis();
  elegantOTA.loop();

  if (now - lastPhotoTaken > settings.delayTime)
  {
    Photo photo = cam.CaptureFrame();
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
      }

      pubSubClient.Disconnect();
    }

    lastPhotoTaken = now;
  }

  pubSubClient.Loop();

  delay(1);
}