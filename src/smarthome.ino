#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "switch-module.h"
#include "sensor-module.h"
#include "shared/mqtt_module.h"
#include "config.h"
#include "logger.h"
#include <SPI.h>


Config config;
Logger logger;
WiFiClient espClient;
MqttModule* pMqtt = new MqttModule(&espClient, 
    config.mqtt_broker,
    config.mqtt_port,
    config.mqtt_username,
    config.mqtt_password,
    config.mgtt_topic,
    &logger);

SensorModule sensor0(2, (uint8_t[]){ 20,21,22,23,27,26,25,24,28,29, 1, 2, 3, 4, 5, 6},
                      false, pMqtt, &config, &logger);                                
SensorModule sensor1(3, (uint8_t[]){ 13,14,15,16,17,18,19,30,35,36,37,38,34,33,32,31}, 
                      false, pMqtt, &config, &logger);
SensorModule sensor2(4, (uint8_t[]){ 43,42,41,40, 0, 7, 8, 9,10,11,12,39,47,46,45,44}, 
                      false, pMqtt, &config, &logger);

SwitchModule switch0(1, 0, true, pMqtt, &config, &logger, 
                        (uint8_t[]){ 8, 9,10,11,12,13,14,15, 0, 1, 2, 3, 7, 6, 5, 4});

void ICACHE_RAM_ATTR isr0() {
  sensor0.isrFlag = true;
}

void ICACHE_RAM_ATTR isr1() {
  sensor1.isrFlag = true;
}

void ICACHE_RAM_ATTR isr2() {
   sensor2.isrFlag = true;
}

void setup()
{
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(10, INPUT);
  pinMode(9, INPUT);

  SPI.begin();
  //Serial.begin(74880);
  Serial.println("\nHello v2\n");
  logger.begin();
  delay(200);
  
  logger.on();
  setup_wifi();
  logger.off();

  pMqtt->begin();
  switch0.begin();
  sensor0.begin();
  sensor1.begin();
  sensor2.begin();

  //setup interrupts for sensors:
  /**
  attachInterrupt(digitalPinToInterrupt(GPIO), ISR, mode);
  GPIO: wszystkie za wyjątkiem 16
  MODE:
    LOW:	Triggers the interrupt whenever the pin is LOW
    HIGH:	Triggers the interrupt whenever the pin is HIGH
    CHANGE:	Triggers the interrupt whenever the pin changes value, from HIGH to LOW or LOW to HIGH
    FALLING:	Triggers the interrupt when the pin goes from HIGH to LOW
    RISING:	Triggers the interrupt when the pin goes from LOW to HIGH

  **/
  attachInterrupt(digitalPinToInterrupt(5), isr0, FALLING);
  attachInterrupt(digitalPinToInterrupt(4), isr1, FALLING);
  attachInterrupt(digitalPinToInterrupt(10), isr2, FALLING);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(config.ssid);

  WiFi.begin(config.ssid, config.password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  pMqtt->loop();
  switch0.loop();
  sensor0.loop();
  sensor1.loop();
  sensor2.loop();
  logger.loop();
}