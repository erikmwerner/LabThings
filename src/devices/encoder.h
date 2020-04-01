#ifndef __ENCODER_H__
#define __ENCODER_H__

#include "device.h"

class LT_Encoder : public LT_Device {
    const uint8_t _pin_a; //< local copy of pin a of the encoder
    const uint8_t _pin_b; //< local copy of pin b of the encoder
    const uint8_t _pullup_enable; //< option to enable pullups
    volatile int8_t _last_position = 0;
    volatile int8_t _position = 0;
    volatile uint32_t _t_last_state_change_us = 0;
    //volatile uint32_t _t_last_check_us = 0;
    uint32_t _debounce_interval_us = 100000; //< encoder ignores events within this interval (default = 100ms)
    Callback _value_changed_callback = nullptr;
    
    volatile uint8_t _old_AB = 0; // use for 16-state

    volatile int8_t _last_dir = 0; //< tk replace this by lookint at first 4 bits lookup value

    bool _accelerate = true; //< if enabled, encoder will count more pulses if rotated continuously
    uint8_t _count = 0; //< counts the sequential changes required to start acceleration
    
    
    
    

  public:
    LT_Encoder(const uint8_t id, const uint8_t pin_a, const uint8_t pin_b, const uint8_t pullup_enable = false)
      : LT_Device(id), _pin_a(pin_a), _pin_b(pin_b), _pullup_enable(pullup_enable) {}
      
    virtual LT::DeviceType type() const { return LT::Encoder; }
      
    void setValueChangedCallback(Callback c) {
      _value_changed_callback = c;
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
      //_t_last_check_us = LT_current_time_us;
      poll_encoder(); // initialize position
    }
    
    // 
    void handleInterrupt() {
      LT_current_time_us = micros();
      poll_encoder();
    }
     // Method 1: Lockout mode w/no interrupts
    // only report position changes if t > t_debounce has elapsed
    // this method responds to changes instantly and is the preferred method
    // Warning: this method adds a filter to remove unwanted events
    // if it isn't checked fast enough. Events will be dropped if it is not refreshed
    // faster than the debounce interval
    void update() {
      poll_encoder();

/*
      if( (LT_current_time_us - _t_last_check_us) >= _debounce_interval_us) {
        // discard changes if the encoder isn't updated fast enough
        #ifdef DEBUG_PRINT
          Serial.println("WARNING: encoder reading dropped");
        #endif
      }
      _t_last_check_us = LT_current_time_us;
*/
      if(_position != _last_position) {
        if(_value_changed_callback != nullptr) {
          (*_value_changed_callback)();
        }
        _last_position = _position;
      }
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
    
    // Returns position change (-1,0,1)
    // Method adapted from: Reading rotary encoder on Arduino by Oleg Mazurov
    // https://www.circuitsathome.com/mcu/reading-rotary-encoder-on-arduino/
    // https://makeatronics.blogspot.com/2013/02/efficiently-reading-quadrature-with.html
    void poll_encoder() {
      //static int8_t position_change[] = {0,1,-1, 0,-1,0,0,1,1,0,0,-1,0,-1,1,0};
      static int8_t lookup_table[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
        _old_AB <<= 2; // remember previous state
        _old_AB |=  ( digitalRead(_pin_a) << 1);
        _old_AB |= digitalRead(_pin_b) ;
        // look at the last 4 bits in _old_AB
        int8_t dir = ( lookup_table[ ( _old_AB & 0x0f ) ] );
        if(dir != 0) {
          if((LT_current_time_us - _t_last_state_change_us) >= _debounce_interval_us ) {
            _position += dir;
            _t_last_state_change_us = LT_current_time_us;
            _count = 0;
          _last_dir = dir;
          }
          else {
            // either a bounce or a fast turn
            if(dir == _last_dir && _accelerate) {
              if(_count > 4) {
                _position += dir;
              }
              else {
                _count++;
              }
            }
          }
        }
      }
};

#endif // __ENCODER_H__
