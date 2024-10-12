#ifndef __SETTINGS_H__
#define __SETTINGS_H__

class Settings
{
private:
    /* data */
public:
    unsigned long delayTime;
    const char *wifiSsid = nullptr;
    const char *wifiPassword = nullptr;
    const char *host = nullptr;
    const char *mqUser = nullptr;
    const char *mqPassword = nullptr;
    const char *mqTopicBase = nullptr;
    const char *hostName = nullptr;

    Settings();
    ~Settings();
};

#endif // __SETTINGS_H__