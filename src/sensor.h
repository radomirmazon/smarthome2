#ifndef Sensor__h
#define Sensor__h

#include <stdlib.h>
#include "lib/pcf8574.h"
#include "shared/base_device.h"
#include "shared/mqtt_module.h"
#include "logger.h"
#include "shared/timer.h"
#include "shared/switch_button.h"

class Sensor :public BaseDevice, ChangeDetector {
  private:
  String internalState;
  bool lastState = false;
  bool bell = false;
  const char * channel;
  Timer configTimer;
  uint8_t configAttemptCounter =10;
  MqttModule* pMqtt;
  SwitchButton* pSwitchButton;
  bool invertState = true;

  public:
  Sensor(int initState, const char * channel, const char * deviceName, const char * topicTemplate, MqttModule* pMqtt) 
    : BaseDevice(pMqtt, channel, deviceName, topicTemplate, false, true) {
      lastState = initState > 0;
      this->channel = channel;
      this->pMqtt = pMqtt;
      this->pSwitchButton = new SwitchButton(this, lastState, invertState, bell);
      configTimer.start(1000 + random(100, 1000));
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
    configTimer.loop();
    if (configTimer.isAlarm() && (configAttemptCounter--) > 0) {
      getNextConfigTimer();
      configRequest();
    }
    
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

  bool onState(String state) {
    //sensor doesn't support setting state from HA
    return 0;
  }

  void stateChanged(bool s) {
    if (s) {
        setState(String("ON"));
    } else {
        setState(String("OFF"));
    }
  }

  void onConfig(String config) {
    configTimer.cancel();
    if (config == "bell") {
      bell = true;
      Serial.println("Config set as bell");
    } else {
      Serial.println("Config set as switch");
    }
  }


  uint8_t mapState(String sState) {
    if (sState == "1") {
      return 1;
    }
    return 0;
  }

  private:
  void getNextConfigTimer() {
    configTimer.start(10000 + random(100, 1000));
  }
  
};


#endif
