#ifndef __ENCODER_H__
#define __ENCODER_H__

#include "device.h"

class LT_Encoder : public LT_Device {
    const uint8_t _pin_a;
    const uint8_t _pin_b;
    const uint8_t _pullup_enable;
    volatile int8_t _position = 0;
    volatile uint32_t _t_last_state_change_us = 0;
    volatile uint32_t _t_last_check_us = 0;
    uint32_t _debounce_interval_us = 10000;//10ms
    Callback _valueChangedCallback = NULL;
    
    #ifdef LT_Encoder_4_State
    uint8_t _idx = 0;
     uint8_t state = 0;
    #else
    uint8_t _old_AB = 0; // use for 16-state
    #endif
    
    #ifdef LT_Encoder_Debounce_Steady
    	int8_t _last_reading = 0;
        bool _last_reading_accepted = true;
    #else // default to lockout mode
    	//debounceLockOut();
      //debounceLockOutNoIterrupts();
    #endif
    
    
    
    

  public:
    LT_Encoder(const uint8_t id, const uint8_t pin_a, const uint8_t pin_b, const uint8_t pullup_enable = false)
      : LT_Device(id), _pin_a(pin_a), _pin_b(pin_b), _pullup_enable(pullup_enable) {}
      
      LT::DeviceType type() { return LT::Encoder; }
      
    void setValueChangedCallback(Callback c) {
      _valueChangedCallback = c;
    }
    
    void setDebounceInterval(uint32_t interval) {
      _debounce_interval_us = interval;
    }
    
    void resetPosition() {
      _position = 0;
    }
    
    void setPosition(int8_t position) {
        _position = position;
    }
    
    int8_t position() {
        return _position;
    }
    
    void begin() {
        if(_pullup_enable) {
            pinMode(_pin_a, INPUT_PULLUP);
            pinMode(_pin_b, INPUT_PULLUP);
        }
        else {
            pinMode(_pin_a, INPUT);
            pinMode(_pin_b, INPUT);
        }
      _t_last_state_change_us = LT_current_time_us;
      _t_last_check_us = LT_current_time_us;
    }
    
    // 
    void handleInterrupt() {
      _position += poll_encoder();
    }
    
    #ifdef LT_Encoder_Debounce_Steady
    // Method 2: Steady mode
    // store the first change and wait until things stop changing to report
    // only report change if it's been stable for t > t_debounce
    // this method has a lag time of t_debounce
    void loop() {
      int8_t reading = poll_encoder();
      // if the position has changed, note the time
      if(reading != 0) {
        _t_last_state_change_us = LT_current_time_us;
        //if this change has not been reported yet, store it
        if(_last_reading_accepted) {
          _last_reading = reading;
          _last_reading_accepted = false;
        }
        return;
      }
      // otherwise, the position has not changed
      // if there was a new, unstable reading
      if(!_last_reading_accepted) {
        // and if the debounce interval has elapsed
        if( (LT_current_time_us - _t_last_state_change_us) >= _debounce_interval_us ) {
          _position += _last_reading;
          (*_valueChangedCallback)();
          _last_reading_accepted = true;
        }
      }
    }
    #else
     // Method 1: Lockout mode w/no interrupts
    // only report position changes if t > t_debounce has elapsed
    // this method responds to changes instantly and is the preferred method
    // Warning: this method adds a filter to remove unwanted events
    // if it isn't checked fast enough. Events will be dropped if it is not refreshed
    // faster than the debounce interval
    void loop() {
      uint8_t temp = poll_encoder();
      if( (LT_current_time_us - _t_last_check_us) >= _debounce_interval_us) {
        // discard changes if the encoder isn't updated fast enough
#ifdef DEBUG_PRINT
          Serial.println("WARNING: encoder reading dropped");
#endif
      }
      else if (temp != 0 ) {
          // accept the new position or
        if ( (LT_current_time_us - _t_last_state_change_us) >= _debounce_interval_us ) {
          _position += temp;
          /*Serial.print("Encoder changed: ID: ");
          Serial.print(UDID());
          Serial.print(" value: ");
          Serial.println(_position);*/
          (*_valueChangedCallback)();
          _t_last_state_change_us = LT_current_time_us;
        }
        else {
            //
        }
      }
      else {
          //
      }
        // save the last check time
      _t_last_check_us = LT_current_time_us;
    }
    /*
    // Method 1: Lockout mode w/interrutps
    // only report position changes if t > t_debounce has elapsed
    // this method responds to changes instantly
    void debounceLockOut() {
      uint8_t temp = poll_encoder();
      if (temp != 0 ) {
        if ( (LT_current_time_us - _t_last_state_change_us) >= _debounce_interval_us ) {
          _position += temp;
          (*_valueChangedCallback)(_position);
          _t_last_state_change_us = LT_current_time_us;
        }
      }
    }*/
    #endif
    
    #ifdef LT_Encoder_4_State
    // Returns position change (-1,0,1)
    // Alternative method of polling encoder
    // Encoder must be checked often to work well
    // 16-state array is usually more reliable
    int8_t poll_encoder() {
      static uint8_t states[4] = {0, 1, 3, 2};
      // store the new state in bits 0 and 1
      state = ( digitalRead(_pin_b) << 1) | digitalRead(_pin_a);
      // took a step forward
      if (states[(idx + 1) % 4] == state) {
        idx = (idx + 1) % 4;
        return  1;
      }
      // took a step backward
      else if (states[(idx + 3) % 4] == state) { // backwards wrap: 5 = -1 + 4
        idx = (idx - 1) % 4;
        return -1;
      }
      return 0;
    }
    #else
    // Returns position change (-1,0,1)
    // Method adapted from: Reading rotary encoder on Arduino by Oleg Mazurov
    // https://www.circuitsathome.com/mcu/reading-rotary-encoder-on-arduino/
    int8_t poll_encoder() {
      static int8_t position_change[] = {0,1,-1, 0,-1,0,0,1,1,0,0,-1,0,-1,1,0};
     
       _old_AB <<= 2; // remember previous state
       _old_AB |= ( ( digitalRead(_pin_b) << 1) | digitalRead(_pin_a) );
       return ( position_change[ ( _old_AB & 0x0f ) ] );
    }
    #endif
};

#endif // __ENCODER_H__
