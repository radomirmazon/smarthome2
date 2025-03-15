#ifndef Sensor_Module2_h
#define Sensor_Module2_h

#include <stdlib.h>
#include "shared/mqtt_module.h"
#include "sensor.h"
#include "config.h"
#include "topic-parser.h"
#include "logger.h"
#include <Adafruit_MCP23X17.h>


class SensorModule {
  private:
    Logger* pLog;
    uint8_t* addressMap; 
    uint8_t address;
    Config* pConfig;
    Adafruit_MCP23X17 * pExpander;
    MqttModule* pMqtt;
    TopicParser* pTopicParser;
    Sensor* sensors[16];
    long time = -1;
    

  public:
    bool isrFlag = false;
    SensorModule(uint8_t address, uint8_t* addressMap, boolean isRevert, MqttModule* pMqtt, Config* pConfig, Logger* pLog) {
      this->pLog = pLog;
      this->addressMap = addressMap;
      this->pConfig = pConfig;
      this->pMqtt = pMqtt;
      pExpander = new Adafruit_MCP23X17();
      Serial.print("Created sensor ");
      Serial.println(address);
      this->address = address;
    }

    void begin() {
      time = millis();
      Serial.print("Init Expander: ");
      bool b = pExpander->begin_SPI(15, &SPI, address);
      Serial.println(b ? "true" : "false");
      pExpander->enableAddrPins();
      Serial.print("Interupt enable: ");
      pExpander->setupInterrupts(true, false, LOW);

      for (uint8_t i=0; i<16; i++) {
        Serial.println("Input mode.");
        pExpander->pinMode(i, INPUT_PULLUP);
        pExpander->setupInterruptPin(i);
        char buffer[5];

        sensors[i] = new Sensor(
          pExpander->digitalRead(i), 
          itoa(addressMap[i], buffer, 10),
          pConfig,
          pMqtt);
        sensors[i]->begin();  
      }
      pExpander->clearInterrupts();
    }

    void loop() {
      //calculate timeout or interrupt flag...
      long lastLoop = millis() - time;
      bool isTimeout = false;
      if (lastLoop > 1000000 || isrFlag) {
        time = millis();
        isTimeout = true;
        isrFlag = false;
      }

      //loop
      for (int i=0; i<16; i++) {
        int state = -1;
        if (isTimeout) {
          state = pExpander->digitalRead(i);
        }
        sensors[i]->loop(state);
      }
      if (isTimeout) {
        pExpander->clearInterrupts();
      }
    }
};

#endif