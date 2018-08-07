#include "Particle.h"
#include <vector>
#include "VineAlert.h"

// ButtonPressAction

using namespace RetailAlert;

std::vector<ButtonPressAction*> ButtonPressAction::instances;
//std::vector<String> Buzzer::devices;

ButtonPressAction::ButtonPressAction(void(*action)(void))
{
  callback = action;
}

void ButtonPressAction::bindToPin(uint8_t buttonPin)
{
  pin = buttonPin;
  pinMode(pin, INPUT_PULLUP);
  instances.push_back(this);
}

void ButtonPressAction::update(void)
{
  // for(std::vector<ButtonPressAction*>::iterator i = instances.begin(); i !=instances.end(); i++)
  for (auto& i : instances)  // not needed dereferencing of iterator!
  {
    bool currentState = digitalRead(i->pin) == 0? true: false;
    if (i->lastState != currentState and millis() - i->lastPressMillis > DEBOUNCE_TIME)
    {
      if(currentState == true)
      {
        if (i->callback)
        {
          i->callback();
        }
      }
      i->lastState = currentState;
      i->lastPressMillis = millis();
    }
  }
}

//IndicatorLED

IndicatorLED::IndicatorLED(uint8_t indicatorLed)
{
  pin = indicatorLed;
}

void IndicatorLED::begin(void)
{
  pinMode(pin, OUTPUT);
}

void IndicatorLED::on(void)
{
  digitalWrite(pin, HIGH);
  state = true;
}

void IndicatorLED::off(void)
{
  digitalWrite(pin, LOW);
  state = false;
}

void IndicatorLED::toggle(void)
{
  state = !state;
  digitalWrite(pin, state? HIGH : LOW);
}

// ToggleSwitchAction

std::vector<ToggleSwitchAction*> ToggleSwitchAction::instances;

void ToggleSwitchAction::bindToPin(uint8_t togglePin)
{
  pin = togglePin;
  pinMode(pin, INPUT_PULLUP);
  instances.push_back(this);
}

void ToggleSwitchAction::update(void)
{
  for(auto& i : instances)
  {
    //Serial << ("Toggle");
    SwitchState currentState = digitalRead(i->pin) == HIGH? OPEN : CLOSED;
    if (i->lastState != currentState and millis() - i->lastPressMillis > DEBOUNCE_TIME)
    {
      Serial << ("toggle");
      if (i->lastState == OPEN)
      {
        i->lastState = CLOSED;
        if(i->onOpen)
        {
          i->onOpen();
        }
      }
      else //(i->state == CLOSED)
      {
        i->lastState = OPEN;
        if (i->onClosed)
        {
          i->onClosed();
        }
      }
      i->lastState = currentState;
      i->lastPressMillis = millis();
    }
  }
}

// Buzzer

void Buzzer::begin()
{
  pinMode(pin, OUTPUT);
}

void Buzzer::alarmOn(const char* deviceID)
{
  if (devices.size())
  {
    for (auto i : devices)
    {
      if (i == deviceID)
      {
        return;
      }
    }
  }
  devices.push_back(deviceID);
}

void Buzzer::alarmOff(const char* deviceID)
{
  if (devices.size())
  {
    int loc = 0;
    for (auto i : devices)
    {
      if (i == deviceID)
      {
        devices.erase(devices.begin() + loc);
      }
      loc++;
    }
  }
}

void Buzzer::buzz(void)
{
  if (devices.size())
  {
    if (state == BUZZER_OFF)
    {
      state = BUZZER_ON;
      tone(A0, 1000);
    }
  }
  else
  {
    if (state == BUZZER_ON)
    {
      state = BUZZER_OFF;
	  noTone(A0);
    }
  }
}

// MillisTimer

std::vector<MillisTimer*> MillisTimer::timerArray;

MillisTimer::MillisTimer(uint32_t milliseconds, functionPtr action, bool repeat) {
  //timerArray.push_back(this);
  delay = milliseconds;
  callback = action;
  repeats = repeat;
}

void MillisTimer::begin(void) {
  timerArray.push_back(this);
  Serial << "Timer Added" << "\n";
}

void MillisTimer::processTimers(void) {
  uint32_t currentMillis = millis();
  int count = 0;
  for (auto& i : timerArray)
  {
    if(currentMillis - i->lastMillis >= i->delay)
    {
      if (i->state == RUNNING){

        if(i->callback){
          i->callback();
        }
        if (!i->repeats){
          i->state = STOPPED;
        }
        else {
          i->lastMillis = currentMillis;
        }
      }
    }
  }
}

void MillisTimer::start (void){
  lastMillis = millis();
  state = RUNNING;
  Serial << "Timer Start\n";
}

void MillisTimer::start (bool repeat){
  repeats = repeat;
  lastMillis = millis();
  state = RUNNING;
  Serial << "Timer Start\n";
}

void MillisTimer::stop(void){
  lastMillis = millis();
  state = STOPPED;
}