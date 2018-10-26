#ifndef __ANALOG_SENSOR_H__
#define __ANALOG_SENSOR_H__

#include "digital_sensor.h"


class LT_AnalogSensor : public LT_Sensor {
  int _value;
  const uint8_t _pin;
  
  public:
    LT_AnalogSensor(const uint8_t id, const uint8_t pin) 
    : LT_Sensor(id), _pin(pin) {}
    
    LT::DeviceType type() { return LT::AnalogSensor; }
    
    void begin() {
      pinMode(_pin, INPUT);
    }
    
    // readSensor always returns 0 to indicate there
    // is a fresh sample
    uint8_t readSensor() {
      _value = analogRead(_pin);
      return 0;
    }
    
    // return the most recent sample
    int value() {
      return _value;
    }
    
    LT_AnalogSensor* instance(){return this;}
};

#endif // End __ANALOG_SENSOR_H__ include guard
