#ifndef Sensor__h
#define Sensor__h

#include <stdlib.h>
#include "lib/pcf8574.h"
#include "shared/base_device.h"
#include "shared/mqtt_module.h"
#include "logger.h"
#include "shared/timer.h"
#include "shared/button.h"
#include "config.h"

class Sensor :public BaseDevice, ChangeDetector {
  private:
  String internalState;
  bool lastState = false;
  const char * channel;
  MqttModule* pMqtt;
  SwitchButton* pSwitchButton;
  bool invertState = true;
  Config* pConfig;

  public:
  Sensor(int initState, const char * channel, Config* pConfig, MqttModule* pMqtt) 
    : BaseDevice(pMqtt, channel, 
          pConfig->sensorSwitchDeviceName,
          pConfig->sensorBellDeviceName,
          pConfig->mqtt_topic_template, false, false) 
  {      
      lastState = initState > 0;
      this->channel = channel;
      this->pMqtt = pMqtt;
      this->pConfig = pConfig;
      this->pSwitchButton = new SwitchButton(this, lastState, invertState);
  }

  void begin() {
    BaseDevice::begin();
    pSwitchButton->begin();
  }


  /**
   * Setting state from hardware is here by loop
   * == -1 => not changes
   * != -1 => real value
   */
  void loop(int newState) {
    pSwitchButton->loop();

    if (newState == -1) {
      return;
    }

    //have new state
    if (lastState != newState) {
      lastState = newState;
      pSwitchButton->setState(newState);
    }
  }

  void stateBoolDetected(bool s) {
    if (s) {
        setState(String(pConfig->sensor_on));
    } else {
        setState(String(pConfig->sensor_off));
    }
  }

  void stateClickDetected(uint8_t value) {
    setState2(String(pConfig->sensor_bell_prefix) + String(value));
    setState2(String(pConfig->sensor_bell_prefix) + String(0));
  }

  void onConfig(String config) {
    //not supported
  }

  bool onState(String state) {
    //sensor doesn't support setting state from HA
    return 0;
  }
};


#endif
