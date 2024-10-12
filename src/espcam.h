#ifndef __ESPCAM_H__
#define __ESPCAM_H__

#include "esp_camera.h"
#include "Arduino.h"

#ifndef __PHOTO_H__
#include "photo.h"
#endif ///__PHOTO_H__/

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

class EspCam
{
private:
    camera_config_t config;
public:
    EspCam(/* args */);
    ~EspCam();
    void Setup();
    Photo CaptureFrame();
};

#endif // __ESPCAM_H__


