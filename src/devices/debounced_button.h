#ifndef __DEBOUNCED_BUTTON_H__
#define __DEBOUNCED_BUTTON_H__

#include "device.h"

typedef void(*voidCallback) ();

class LT_DebouncedButton : public LT_Device {
    const uint8_t _pin;
    const uint8_t _pullup_enable;
    volatile uint8_t _button_state;
    volatile uint8_t _last_button_state;
    volatile uint32_t _t_last_state_change_us;
    volatile bool _button_went_low, _button_went_high;
    uint32_t _debounce_interval_us = 50000;//50ms
    voidCallback _callback_released = NULL;
    voidCallback _callback_pressed = NULL;
    
    inline void onButtonStateChanged() {
      if (_button_state == HIGH) {
        _button_went_high = true;
      }
      else {
        _button_went_low = true;
      }
    }

  public:
    LT_DebouncedButton(const uint8_t id, const uint8_t pin, const uint8_t pullup_enable = false)
    : LT_Device(id), _pin(pin), _pullup_enable(pullup_enable) {}
    
    LT::DeviceType type() const { return LT::DebouncedButton; }
    
    void setButtonReleasedCallback(voidCallback c) {
      _callback_released = c;
    }
    void setButtonPressedCallback(voidCallback c) {
      _callback_pressed = c;
    }
    void setDebounceInterval(uint32_t interval) {
      _debounce_interval_us = interval;
    }
    
    int8_t isPressed() {
      return !_button_state;
    }

    void begin() {
        if(_pullup_enable){
            pinMode(_pin, INPUT_PULLUP);
        }
        else {
            pinMode(_pin, INPUT);
        }
      _last_button_state = _button_state = digitalRead(_pin);
      _t_last_state_change_us = LT_current_time_us;
      _button_went_low = _button_went_high = false;
    }
    
    /**************************************************************************/
    /*!
    @brief  ISR routine updates internal flags. Callback functions are executed in
    the next event loop.
    */
    /**************************************************************************/
    void handleInterrupt() {
      LT_current_time_us = micros();
      debounceLockout();
    }
    
    /**************************************************************************/
    /*!
    @brief  check the button state and update internal flags. If the state has
    changed, execute the callback funtion.
    */
    /**************************************************************************/
    void update() {
      debounceLockout();
      if (_button_went_high) {
        if (_callback_released != NULL) {
          (*_callback_released)();
        }
        _button_went_high = false;
      }
      else if (_button_went_low) {
        if (_callback_pressed != NULL) {
           (*_callback_pressed)();
         }
         _button_went_low = false;
       }
    }
    
    // Method 1: Lockout mode
    // only report position changes if t > debounce has elapsed
    // this method responds to changes instantly
    // the current button reading is stored in _button_state
    inline void debounceLockout() {
      uint8_t reading = digitalRead(_pin);

      // if the debounce interval has elapsed, save the button state
      if ( ( LT_current_time_us - _t_last_state_change_us ) >= _debounce_interval_us ) {
        if ( reading != _button_state ) {
          _button_state = reading;
          onButtonStateChanged();
          //_last_button_state = reading;
          _t_last_state_change_us = LT_current_time_us;
        }
      }
    }
    
    // Method 2: Steady mode
    // store the first change and wait until things stop changing to report
    // only report if it it's been stable for t > debounce
    // this method waits until debounce has elapsed to respond
    inline void debounceSteady() {
      uint8_t reading = digitalRead(_pin);

      // if the state has changed, note the time
      if (reading != _last_button_state) {
        _t_last_state_change_us = LT_current_time_us;
      }
      // if the reading doesn't match the button state
      if (reading != _button_state) {
        // and enough time has passed
        if ((LT_current_time_us - _t_last_state_change_us) >= _debounce_interval_us ) {
          // save the button state
          _button_state = reading;
          onButtonStateChanged();
        }
      }
      _last_button_state = reading;
    }
    
};

#endif // End __DEBOUNCED_BUTTON_H__ include guard
