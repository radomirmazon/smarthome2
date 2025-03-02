#ifndef Sensor_Module2_h
#define Sensor_Module2_h

#include <stdlib.h>
#include "lib/pcf8574.h"
#include "shared/mqtt_module.h"
#include "sensor.h"
#include "config.h"
#include "topic-parser.h"
#include "logger.h"


class SensorModule {
  private:
    Logger* pLog;
    uint8_t* addressMap; 
    Config* pConfig;
    PCF8574 * pExpander;
    MqttModule* pMqtt;
    TopicParser* pTopicParser;
    Sensor* sensors[8];
    long time = -1;
    

  public:
    bool isrFlag = false;
    SensorModule(uint8_t address, uint8_t* addressMap, boolean isRevert, MqttModule* pMqtt, Config* pConfig, Logger* pLog) {
      this->pLog = pLog;
      this->addressMap = addressMap;
      this->pConfig = pConfig;
      this->pMqtt = pMqtt;
      pExpander = new PCF8574(address);
    }

    void begin() {
      time = millis();  
      for (int i=0; i<8; i++) {
        pinMode(*pExpander, i, INPUT_PULLUP);
      }
      for (int i=0; i<8; i++) {
        char buffer[5];
        sensors[i] = new Sensor(
          digitalRead(*pExpander, i), 
          itoa(addressMap[i], buffer, 10),
          pConfig,
          pMqtt);
        sensors[i]->begin();  
      } 
    }

    void loop() {
      //calculate timeout or interrupt flag...
      long lastLoop = millis() - time;
      bool isTimeout = false;
      if (lastLoop > 10000 || isrFlag) {
        time = millis();
        isTimeout = true;
        isrFlag = false;
      }

      //loop
      for (int i=0; i<8; i++) {
        int state = -1;
        if (isTimeout) {
          state = digitalRead(*pExpander, i);
        }
        sensors[i]->loop(state);
      }
    }
};

#endif