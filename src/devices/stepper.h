#ifndef __STEPPER__H__
#define __STEPPER__H__

#include "device.h"

/*!
 * @file stepper.h
 *
 * @mainpage 
 *
 * @section intro_sec Introduction
 *
 *
 * @section dependencies Dependencies
 *
 * @section author Author
 *
 * Written by Erik Werner
 *
 * @section license License
 *
 *
 */


typedef void(*intCallback) (int);

class LT_Stepper : public LT_Device {
    const uint8_t _step_pin; ///< digital input pin to control stepping
    const uint8_t _dir_pin; ///< digital input pin to control direction
    const int8_t _enable_pin; ///< digital input pin to control power to the motor
    uint32_t _t_last_step; ///< the system time in microseconds that the last step was taken
    uint32_t _interval; ///< the time in microseconds to wait until setting step high
    uint32_t _pulse_width; ///< the minimum time in microseconds to hold a step pulse high
    uint16_t _resolution; ///< the number of steps per revolution of the motor. Used in position measurements.
    int16_t _position; ///< the current position of the motor from [0 resolution]
    int16_t _steps_remaining; ///< the number of steps remaining until the motor has reached its target position
    bool _direction;
    bool _running;
    bool _stepping;
    bool _enabled;
    //go ahead, take a step
    void step() {
        // finish a step
        if(_stepping) {
            if( (LT_current_time_us - _t_last_step) >= _pulse_width ) {
                digitalWrite( _step_pin, LOW );
                _stepping = false;
                updatePosition( (_direction == true) ? _position++ : _position--);
            }
        }
        // start a step
        else {
            if( (LT_current_time_us - _t_last_step) >= _interval ) {
                digitalWrite( _step_pin, HIGH );
                _t_last_step += _interval;
                _stepping = true;
            }
        }
    }
    // update the position variable
    // this function is called whenever a step is completed
    // example: for a motor with 200 steps/rev, the position can be [0, 199]
    void updatePosition(const int16_t position) {
        _position = position;
        if( _position >= _resolution ){
            _position = 0;
        }
        else if( _position < 0 ) {
            _position = _resolution - 1;
        }
        // check if a target position was set
        if( _steps_remaining > 0 ) {
          _steps_remaining--;
        }
        if( _steps_remaining == 0 ) {
          _steps_remaining = -1;
          _running = false;
        }
    }
    
    void checkDirection(float rpm) {
      bool isCCW = ( rpm > 0 );
      if( isCCW != _direction ) {
        _direction = isCCW;
        digitalWrite( _dir_pin, _direction );
      }
    }
  public:
    LT_Stepper(const uint8_t id, const uint8_t step_pin, const uint8_t dir_pin,
    const int8_t enable_pin = -1) : LT_Device(id), _step_pin(step_pin), 
    _dir_pin(dir_pin), _enable_pin(enable_pin) {}
    
    virtual LT::DeviceType type() const { return LT::Stepper; }
    
    void begin() {
        _t_last_step = LT_current_time_us;
        _interval = 1000;
        _pulse_width = 30;
        _resolution = 200;
        _position = 0;
        _steps_remaining = -1;
        _direction = true;
        _running = false;
        _stepping = false;
        _enabled = true;
        pinMode( _step_pin, OUTPUT );
        pinMode( _dir_pin, OUTPUT );
        if(_enable_pin < 0) {
          // an enable pin was not specified
        }
        else {
          pinMode( _enable_pin, OUTPUT );
          setEnabled(true);
        }
        digitalWrite( _step_pin,LOW );
        digitalWrite( _dir_pin, _direction );

    }
    void update() {
        if( _running || _stepping ) {
            step();
        }
    }
    LT_Stepper* instance() { return this; }

    void setEnabled(const bool enabled) {
      if(_enable_pin < 0) return;
      // enable is active low
      digitalWrite( _enable_pin, !enabled );
      _enabled = enabled;
    }
    
    void setResolution(const uint16_t resolution) {
      _resolution = resolution;
    }
    
    void setSpeed(float rpm) {
        // resolution is the number of steps to make a revolution
        // (1 rev / 1 min) * (1 min / 60,000,000 us) * (_resolution / 1 rev) = _resolution (steps) / us
        if(rpm == 0) {
          _running = false;
        }
        else {
          checkDirection(rpm);
          if(rpm < 0) {
            rpm = -rpm;
          }
          else {}
          
          _interval = 60000000.0 / ( (float)(_resolution) * rpm);
          _t_last_step = LT_current_time_us;
          _running = true;
        }
    }
    
    float getSpeed() {
      float rpm = 60000000.0 / ( (float)(_resolution) * _interval);
      return rpm;
    }
    
    // read the motor speed and avoid float calculation
    uint32_t getInterval() const { return _interval; }
    uint16_t getResolution() const { return _resolution; }
    
    // expects a position argument between 0 and _resolution
    // common _resolution values are 200, 400, 800, 1600
    // useful for setting index or zero position
    // use rotate() to move to a specific angle
    void setPosition(const uint16_t position) {
      updatePosition(position);
    }
    
    // take the specified number of steps. use negative (-) for anti-clockwise rotation
    void rotate(int16_t steps, float rpm) {
      // save the starting point
      if(steps == 0 || rpm == 0) return;
      if(steps < 0) {
        steps = -steps;
        rpm = -rpm;
      }
      setSpeed(rpm);
      _steps_remaining = steps;
    }
    
    // return the number of steps between the current position and the target position
    int16_t distanceToGo() const {
      return _steps_remaining;
    }
    
    /**************************************************************************/
    /*!
    @brief read the current postion of the motor
    @return returns a value between 0 and _resolution
    */
    /**************************************************************************/
    uint16_t getPosition() const {return _position;}
    
    /**************************************************************************/
    /*!
    @brief read if the motor driver is enabled
    @return true if the driver is enabled
    */
    /**************************************************************************/
    bool getEnabled() const {return _enabled;}
    
};

#endif //End __STEPPER__H__ include guard
