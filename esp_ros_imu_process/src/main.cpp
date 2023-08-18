#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "JY901.h"
#include "comunicate.h"
#include "ws2812.h"
#include "config.h"
#include "data_proc.h"
 

void setup()
{
  Serial.begin(115200);
  Serial.println("start init");
  udp.begin(18688);
  imu.init();
  tata_proc_init();
  // ws2812_init();
}
void loop()
{

  // udp.write("hello");
  // ws2812_show_blink();

  delay(1000);
}