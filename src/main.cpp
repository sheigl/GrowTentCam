#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
//#include "FS.h"     // SD Card ESP32
//#include "SD_MMC.h" // SD Card ESP32
#include "fb_gfx.h"
#include "soc/soc.h"          //disable brownout problems
#include "soc/rtc_cntl_reg.h" //disable brownout problems
#include <WiFi.h>
#include <settings.h>
#include <PubSubClient.h>
#include <driver/rtc_io.h>

unsigned long lastPhotoTaken = millis();
PubSubClient pubSubClient;
WiFiClient espClient;

typedef struct
{
  uint8_t *buffer = nullptr;
  size_t length = 0;
} Photo, *pPhoto;

void connectWifi();
void disconnectWifi();
bool connectPubSub();
void publishPhoto(Photo *photo);
void log(const String message);
Photo *takePhoto();
void setupCamera();
void setupPubSub();
//void setupSD();

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector
  setupCamera();
  setupPubSub();
  //setupSD();

  Serial.begin(9600);
  Serial.setDebugOutput(false);
}

void loop()
{
  unsigned long now = millis();

  if (now - lastPhotoTaken > DELAY_TIME)
  {
    Photo *photo = takePhoto();
    connectWifi();

    if (connectPubSub())
    {
      if (photo->length > 0)
      {
        log("Sending photo...");
        Serial.println("Sending photo...");
        publishPhoto(photo);
      }
      else
      {
        log("Unable to take photo...");
        Serial.println("Unable to take photo...");
      }

      pubSubClient.disconnect();
    }

    disconnectWifi();

    lastPhotoTaken = now;
  }

  pubSubClient.loop();
  delay(1);
}

/* void setupSD()
{
  Serial.println("Starting SD Card");
  if (!SD_MMC.begin())
  {
    Serial.println("SD Card Mount Failed");
    return;
  }

  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE)
  {
    Serial.println("No SD Card attached");
    return;
  }
} */

void setupPubSub()
{
  pubSubClient.setClient(espClient);
  pubSubClient.setServer(HOST, 1883);
}

void connectWifi()
{
  // Wi-Fi connection
  WiFi.begin(WIFI_SSID, WIFI_PWD);
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

bool connectPubSub()
{
  while (!pubSubClient.connected())
  {
    Serial.println("Connecting to message broker...");
    pubSubClient.connect(HOST_NAME, MQ_USR, MQ_PWD);

    delay(250);
  }

  return pubSubClient.connected();
}

char *createTopic(String topic)
{
  String strTopicBase(MQ_TOPIC);
  String fullTopic = strTopicBase + "/" + topic;
  char *topicBuffer = (char *)malloc(fullTopic.length() + sizeof(char *));

  fullTopic.toCharArray(topicBuffer, fullTopic.length() + sizeof(char *));

  return topicBuffer;
}

void publishPhoto(Photo *photo)
{
  char *topicBuffer = createTopic("snapshot");

  log("Sending payload to: " + String(topicBuffer));
  pubSubClient.beginPublish(topicBuffer, photo->length, false);

  size_t res;
  uint32_t offset = 0;
  uint32_t to_write = photo->length;
  uint32_t buf_len;
  do
  {
    buf_len = to_write;
    if (buf_len > 64000)
      buf_len = 64000;

    log("Sending bytes " + String(offset + buf_len) + "/" + String(to_write));
    res = pubSubClient.write(photo->buffer + offset, buf_len);

    offset += buf_len;
    to_write -= buf_len;

  } while (res == buf_len && to_write > 0);

  bool published = pubSubClient.endPublish();

  if (!published)
    log("Unable to send photo through mq...");

  free(topicBuffer);
}

void log(const String message)
{
  Serial.println(message);
  char *topicBuffer = createTopic("log");
  char *payloadBuffer = (char *)malloc(message.length() + sizeof(char *));
  message.toCharArray(payloadBuffer, message.length() + sizeof(char *));

  if (connectPubSub())
    pubSubClient.publish(topicBuffer, payloadBuffer, message.length());

  free(topicBuffer);
  free(payloadBuffer);
}

void setupCamera()
{
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
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound())
  {
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  }
  else
  {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

Photo *takePhoto()
{
  Serial.println("Capturing frame...");

  camera_fb_t *fb = esp_camera_fb_get();
  Serial.println("Got frame");

  /* Serial.println("Converting to BMP...");
  bool converted = fmt2bmp(fb->buf, fb->len, 800, 600, (pixformat_t)PIXFORMAT_RGB565, &photo.buffer, &photo.length);
  if (!converted)
  {
      Serial.println("Unable to create BMP");
  } */

  // Path where new picture will be saved in SD Card
  //String path = "/picture.jpg";
  uint8_t *buffer;
  size_t length;

  buffer = fb->buf;
  length = fb->len;

  //fs::FS &fs = SD_MMC;
  //Serial.printf("Picture file name: %s\n", path.c_str());

  /* File file = fs.open(path.c_str(), FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file in writing mode");
  }
  else
  {
    buffer = fb->buf;
    length = fb->len;
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.printf("Saved file to path: %s\n", path.c_str());
  }
  file.close(); */

  Serial.println("Resetting frame buffer...");
  esp_camera_fb_return(fb);
  fb = NULL;

  /* File writtenFile = fs.open(path.c_str(), FILE_READ);

  if(!writtenFile){
    Serial.println("Failed to open file in read mode");
  }
  else {
    length = writtenFile.size();
    buffer = (uint8_t*)malloc(sizeof(uint8_t) + length);

    writtenFile.read(buffer, length);

    Serial.printf("Got file: %s\n", path.c_str());
    Serial.print("Length is: ");
    Serial.println(length);
  }

  writtenFile.close();*/

  /* bool jpeg_converted = frame2jpg(fb, 80, &photo.buffer, &photo.length);
  Serial.println("Resetting frame buffer...");
  esp_camera_fb_return(fb);
  fb = NULL;
  if (!jpeg_converted)
  {
      Serial.println("JPEG compression failed");
      ESP.restart();
  } */

  pPhoto photo = (pPhoto)malloc(sizeof(photo));
  photo->buffer = buffer;
  photo->length = length;

  return photo;
}