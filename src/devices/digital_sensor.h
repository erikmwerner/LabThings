#ifndef __DIGITAL_SENSOR_H__
#define __DIGITAL_SENSOR_H__

#include "device.h"


class LT_DigitalSensor : public LT_Sensor {
    uint8_t _value;
    const uint8_t _pin;
    

  public:
    LT_DigitalSensor(const uint8_t id, const uint8_t pin) 
    : LT_Sensor(id), _pin(pin) {}
    
    LT::DeviceType type() { return LT::DigitalSensor; }
    
    void begin() {
      pinMode(_pin, INPUT);
    }
    
    // readSensor always returns 0 to indicate there
    // is a fresh sample
    uint8_t readSensor() {
      _value = digitalRead(_pin);
      return 0;
    }
    
    // return the most recent sample
    uint8_t value() {
      return _value;
    }
    
    LT_DigitalSensor* instance(){return this;}
};

#endif // End __DIGITAL_SENSOR_H__ include guard
