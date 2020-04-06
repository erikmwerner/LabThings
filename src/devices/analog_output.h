#ifndef __ANALOG_OUTPUT_H__
#define __ANALOG_OUTPUT_H__

#include "digital_output.h"


/**************************************************************************/
/*! 
    @brief  Base Class for analog output devices

    Untested: Use LEDC for PWM on ESP32
*/
/**************************************************************************/
#if defined(ARDUINO_ARCH_ESP32)
class LT_AnalogOutput : public LT_DigitalOutput {
  protected:
    const uint8_t m_channel;
  public:
    LT_AnalogOutput(const uint8_t id, const uint8_t pin, const uint8_t channel) 
    : LT_DigitalOutput(id, pin), m_channel(channel) {}
    virtual LT::DeviceType type() const { return LT::AnalogOutput; }
    
    void begin() {
      ledcAttachPin(_pin, m_channel);
      ledcSetup(m_channel, 12000, 8); // 12 kHz PWM, 8-bit resolution
    }
    
    void setValue(const int value) {
      _value = value;
      ledcWrite(m_channel, _value);
    }
    LT_AnalogOutput* instance(){ return this; }
};
#else
class LT_AnalogOutput : public LT_DigitalOutput {
  public:
    LT_AnalogOutput(const uint8_t id, const uint8_t pin) 
    : LT_DigitalOutput(id, pin) {}
    virtual LT::DeviceType type() const { return LT::AnalogOutput; }
    
    void setValue(const int value) {
      _value = value;
      analogWrite(_pin, _value);
    }
    LT_AnalogOutput* instance(){ return this; }
};
#endif

#endif // End __ANALOG_OUTPUT_H__ include guard
