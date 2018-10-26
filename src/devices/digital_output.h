#ifndef __DIGITAL_OUTPUT_H__
#define __DIGITAL_OUTPUT_H__

#include "device.h"

class LT_DigitalOutput : public LT_Device {
  protected:
    const uint8_t _pin;
    uint8_t _value;

  public:
    LT_DigitalOutput(const uint8_t id, const uint8_t pin) : LT_Device(id), _pin(pin) {}
    
    LT::DeviceType type() { return LT::DigitalOutput; }
    
    void begin() {
      pinMode(_pin, OUTPUT);
    }
    
    uint8_t getValue() {
      return _value;
    }
    
    void setValue(uint8_t value) {
      _value = value;
      digitalWrite(_pin,_value);
    }
    
    LT_DigitalOutput* instance() {return this;}
};

#endif // End __DIGITAL_OUTPUT_H__ include guard
