#ifndef RETAILALERT_H
#define RETAILALERT_H

#include "Particle.h"
#include <vector>

constexpr uint16_t DEBOUNCE_TIME = 2500;  //milliseconds

template <class T> inline Print& operator <<(Print& obj, T arg)
{
  obj.print(arg);
  return obj;
}

namespace RetailAlert{

  using functionPtr = void(*)(void);
  class ButtonPressAction{
    public:
      ButtonPressAction(functionPtr action);
      void bindToPin(uint8_t buttonPin);
      static void update(void);
    private:
      uint8_t pin;
      uint32_t lastPressMillis;
      bool lastState;
      functionPtr callback;
      static std::vector<ButtonPressAction*>instances;
  };

  class IndicatorLED{
    public:
      IndicatorLED(uint8_t indicatorLed);
      void begin(void);
      void on(void);
      void off(void);
      void toggle(void);
    private:
      uint8_t pin;
      bool state;
  };

  class ToggleSwitchAction{
    public:
      ToggleSwitchAction(functionPtr openAction, functionPtr closeAction) : onOpen(openAction), onClosed(closeAction){};
      void bindToPin(uint8_t togglePin);
      static void update(void);

    private:
      uint8_t pin;
      uint32_t lastPressMillis;
      functionPtr onOpen;
      functionPtr onClosed;
      enum SwitchState{
        OPEN,
        CLOSED,
        UNKNOWN,
      };
      SwitchState lastState = UNKNOWN;
      static std::vector<ToggleSwitchAction*>instances;
  };

  class Buzzer{
    public:
      Buzzer(uint8_t relayPin) : pin(relayPin){};
      void begin();
      void alarmOn(const char* deviceID);
      void alarmOff(const char* deviceID);
      void buzz(void);

    private:
      uint8_t pin;
      std::vector<String> devices;
      enum BuzzState{
        BUZZER_OFF,
        BUZZER_ON,
      } state = BUZZER_OFF;
  };

  class MillisTimer{
    public:
      MillisTimer(uint32_t time, functionPtr action, bool repeat);
      static void processTimers(void);
      void start(void);
      void start (bool repeat);
      void stop(void);
      void begin(void);
    private:
      enum TimerState{
        STOPPED,
        RUNNING,
      }state = STOPPED;
      functionPtr callback;
      uint32_t delay;
      uint32_t lastMillis = 0;
      bool repeats;
      static std::vector<MillisTimer*>timerArray;
  };

};

#endif