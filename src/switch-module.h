#ifndef Switch_Module_h
#define Switch_Module_h

#include <stdlib.h>
#include "shared/mqtt_module.h"
#include "config.h"
#include "topic-parser.h"
#include "logger.h"
#include "switch.h"
#include "switchSetter.h"
#include <Adafruit_MCP23X17.h>

class SwitchModule :public SwitchSetter {
  private:
    uint8_t logicAddress; 
    uint8_t address;
    Config* pConfig;  
    Adafruit_MCP23X17* pExpander;
    bool isRevert;
    MqttModule* pMqtt;
    Logger* pLog;
    Switch* switches[16];
    uint8_t* addressMap;


  public:
    SwitchModule(uint8_t address, uint8_t logicAddress, bool isRevert, MqttModule* pMqtt, 
    Config* pConfig, Logger* pLog, uint8_t* addressMap) {
      this->logicAddress = logicAddress;
      this->pConfig = pConfig;
      this->pMqtt = pMqtt;
      this->isRevert = isRevert;
      this->pLog = pLog;
      this-> addressMap = addressMap;
      this-> address = address;
      pExpander = new Adafruit_MCP23X17;
    }

    void begin() {
      Serial.print("Init Expander: ");
      bool b = pExpander->begin_SPI(15, &SPI, address);
      Serial.println(b ? "true" : "false");
      pExpander->enableAddrPins();

      for (uint8_t i=0; i<16; i++) {
        pExpander->pinMode(i, OUTPUT);//OUTPUT
        pExpander->digitalWrite(i, isRevert);
      }

      char buffer[5];
      for (uint8_t i=0; i<16; i++) {
        switches[i] = new Switch(itoa(decodeAddress(i), buffer, 10), pConfig, pMqtt, this, i);
        switches[i]->begin();
      }
    }

    uint8_t decodeAddress(uint8_t address) {
      return logicAddress*16 + address;
    }

    void loop() {
    }

    void setPort(int index, bool state) {
      pExpander->digitalWrite(addressMap[index], isRevert ^ state);
      pLog->setBlinkOnce();
    }
  
};


#endif
