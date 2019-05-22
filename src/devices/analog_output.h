#ifndef __ANALOG_OUTPUT_H__
#define __ANALOG_OUTPUT_H__

#include "digital_output.h"


/**************************************************************************/
/*! 
    @brief  Base Class for analog output devices

    Untested: Use LEDC for PWM on ESP32
*/
/**************************************************************************/
class LT_AnalogOutput : public LT_DigitalOutput {
  #if defined(ARDUINO_ARCH_ESP32)
    const uint8_t _channel;
  #endif
  public:
    
    #if defined(ARDUINO_ARCH_ESP32)
    LT_AnalogOutput(const uint8_t id, const uint8_t pin, const uint8_t channel) 
    : LT_DigitalOutput(id, pin), _channel(channel) {}
    #else
    LT_AnalogOutput(const uint8_t id, const uint8_t pin) 
    : LT_DigitalOutput(id, pin) {}
    #endif
    virtual LT::DeviceType type() const { return LT::AnalogOutput; }
    
    #if defined(ARDUINO_ARCH_ESP32)
    void begin() {
      ledcAttachPin(_pin, _channel);
      ledcSetup(_channel, 12000, 8); // 12 kHz PWM, 8-bit resolution
    }
    #endif
    
    void setValue(const int value) {
      _value = value;
      #if defined(ARDUINO_ARCH_ESP32)
      ledcWrite(_channel, _value);
      #else
      analogWrite(_pin, _value);
      #endif
    }
    
    LT_AnalogOutput* instance(){ return this; }
};

#endif // End __ANALOG_OUTPUT_H__ include guard
