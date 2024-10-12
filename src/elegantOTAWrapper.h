#ifndef __OTAWRAPPER_H__
#define __OTAWRAPPER_H__
#include <Arduino.h>
#include <ESPAsyncWebServer.h>

class ElegantOTAWrapper
{
    private:
        unsigned long ota_progress_millis = 0;
        AsyncWebServer *server = nullptr;
        void onOTAStart();
        void onOTAProgress(size_t current, size_t final);
        void onOTAEnd(bool success);
    public:
        ElegantOTAWrapper(AsyncWebServer *server);
        ~ElegantOTAWrapper();

        void setup();
        void loop();
};

#endif