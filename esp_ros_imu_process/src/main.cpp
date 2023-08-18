#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiClient.h>
 
#include "comunicate.h"
 
#include "config.h"
#include "data_proc.h"
 

void setup()
{
  Serial.begin(115200);
  Serial.println("start init");
  udp.begin(18688);
  tata_proc_init();
}
void loop()
{

  // udp.write("hello");
  

  delay(1000);
}