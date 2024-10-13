#include "espcam.h"

EspCam::EspCam()
{
}

EspCam::~EspCam()
{
}

void EspCam::Setup()
{

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
    config.pixel_format = PIXFORMAT_RGB565;
    config.frame_size = FRAMESIZE_SVGA;
    config.fb_count = 1;
    config.jpeg_quality = 16;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_LATEST;
    // config.jpeg_quality = 10;
    // config.fb_location = CAMERA_FB_IN_DRAM;
    // config.fb_count = 1;
    // config.frame_size = FRAMESIZE_SVGA;

    /* if (psramFound())
    {
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    }
    else
    {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    } */

    // Camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        ESP.restart();
    }
}

Photo EspCam::CaptureFrame()
{
    Serial.println("Capturing frame...");

    Photo photo;
    camera_fb_t *fb = esp_camera_fb_get();
    Serial.println("Got frame");

    /* Serial.println("Converting to BMP...");
    bool converted = fmt2bmp(fb->buf, fb->len, 800, 600, (pixformat_t)PIXFORMAT_RGB565, &photo.buffer, &photo.length);
    if (!converted)
    {
        Serial.println("Unable to create BMP");
    } */

    photo.buffer = fb->buf;
    photo.length = fb->len;

    Serial.println("Resetting frame buffer...");
    esp_camera_fb_return(fb);
    fb = NULL;
    
    /* bool jpeg_converted = frame2jpg(fb, 80, &photo.buffer, &photo.length);
    Serial.println("Resetting frame buffer...");
    esp_camera_fb_return(fb);
    fb = NULL;
    if (!jpeg_converted)
    {
        Serial.println("JPEG compression failed");
        ESP.restart();
    } */

    return photo;
}