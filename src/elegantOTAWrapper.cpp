#include <elegantOTAWrapper.h>
#include <ElegantOTA.h>

ElegantOTAWrapper::ElegantOTAWrapper(AsyncWebServer *server)
{
    this->server = server;
}

ElegantOTAWrapper::~ElegantOTAWrapper()
{

}

void ElegantOTAWrapper::setup()
{
    ElegantOTA.begin(server); // Start ElegantOTA
    // ElegantOTA callbacks
    ElegantOTA.onStart([this]() { onOTAStart(); });
    ElegantOTA.onProgress([this](size_t current, size_t final){ onOTAProgress(current, final); });
    ElegantOTA.onEnd([this](bool success){ onOTAEnd(success); });

}

void ElegantOTAWrapper::loop()
{
    ElegantOTA.loop();
}

void ElegantOTAWrapper::onOTAStart()
{
  // Log when OTA has started
  Serial.println("OTA update started!");
  // <Add your own code here>
}

void ElegantOTAWrapper::onOTAProgress(size_t current, size_t final)
{
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000)
  {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void ElegantOTAWrapper::onOTAEnd(bool success)
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