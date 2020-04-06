#ifndef BUZZER_H
#define BUZZER_H

/*!
 * warning: the tone() function interferes with PWM on pins 3 and 11
*/ 
class LT_Buzzer : public LT_AnalogOutput{

  uint32_t m_last_update = 0; //< the time in ms of last change
  uint32_t m_t_on; //< time in ms to make noise
  uint32_t m_t_off; //< time in ms to shut up
  int m_frequency; //< the frequency of the tone to generate
  bool m_is_active = false; //< flag to track if alarm is sounding
  bool m_is_noisy = false; //< flag to track if currently making noise

public:
#if defined(ARDUINO_ARCH_ESP32)
  LT_Buzzer(const uint8_t id, const uint8_t pin, const uint8_t channel, 
  const int freq = 1175, const uint32_t t_on = 200000, const uint32_t t_off = 50000) : 
  LT_AnalogOutput(id, pin, channel), m_frequency(freq), m_t_on(t_on), m_t_off(t_off) {}
#else
  LT_Buzzer(const uint8_t id, const uint8_t pin, const int freq = 1175, const uint32_t t_on = 200000, const uint32_t t_off = 50000) : 
  LT_AnalogOutput(id, pin), m_frequency(freq), m_t_on(t_on), m_t_off(t_off) {}
#endif

  void update() {
    uint32_t now = LT_current_time_us;
    if(m_is_active) {
      // alarm is on
      if(m_is_noisy) {
        // buzzer making noise
        if( (now - m_last_update) >= m_t_on) {
          // time to stop noise
          #if defined(ARDUINO_ARCH_ESP32)
          ledcWriteTone(m_channel, 0);
          #else
          noTone(_pin);
          #endif
          m_is_noisy = false;
          m_last_update = now;
          Serial.println("buzz off");
        }
        else {
          // keep making noise
        }
      }
      else {
        // busser is not making noise
        if( (now - m_last_update) >= m_t_off) {
          // time to start noise
          #if defined(ARDUINO_ARCH_ESP32)
          ledcWriteTone(m_channel, m_frequency);
          #else
          tone(_pin, m_frequency);
          #endif
          m_is_noisy = true;
          m_last_update = now;
          Serial.println("buzz on");
        }
        else {
          // stay silent for now
        }
      }
    }
    else {
      // alarm is not active
      if(m_is_noisy) {
        // buzzer making noise, stop noise
        #if defined(ARDUINO_ARCH_ESP32)
          ledcWriteTone(m_channel, 0);
          #else
          noTone(_pin);
          #endif
          m_is_noisy = false;
      }
      else {
        // no changes
      }
    }
  }
  

  // set the note to make noise at
  void setFrequency(int frequency) {
    m_frequency = frequency;
  }

  // turn the alarm on or off
  void setActive(bool is_active) {
    m_is_active = is_active;
  };
};

#endif // BUZZER_H
