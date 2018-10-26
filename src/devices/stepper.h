#ifndef __STEPPER__H__
#define __STEPPER__H__

#include "device.h"

typedef void(*intCallback) (int);

class LT_Stepper : public LT_Device {
    const uint8_t _step_pin, _dir_pin;
    uint32_t _t_last_step;
    uint32_t _interval;
    uint32_t _pulse_width;
    uint16_t _resolution;
    int16_t _position;
    int16_t _steps_remaining;
    bool _direction;
    bool _running;
    bool _stepping;
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
    // called whenever a step is completed
    // example: for a motor with 200 steps/rev, the position can be between 0 and 200
    void updatePosition(int16_t position) {
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
    
    void checkDirection(double rpm) {
      bool isCCW = ( rpm > 0 );
      if( isCCW != _direction ) {
        _direction = isCCW;
        digitalWrite( _dir_pin, _direction );
      }
    }
  public:
    LT_Stepper(const uint8_t id, const uint8_t step_pin, const uint8_t dir_pin) 
    : LT_Device(id), _step_pin(step_pin), _dir_pin(dir_pin) {}
    
    LT::DeviceType type() { return LT::Stepper; }
    
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
        pinMode( _step_pin, OUTPUT );
        pinMode( _dir_pin, OUTPUT );
        digitalWrite( _step_pin,LOW );
        digitalWrite( _dir_pin, _direction );
    }
    void loop() {
        if( _running || _stepping ) {
            step();
        }
    }
    LT_Stepper* instance() { return this; }
    
    void setResolution(uint16_t resolution) {
      _resolution = resolution;
    }
    
    void setSpeed(double rpm) {
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
          
          _interval = 60000000.0 / ( (double)(_resolution) * rpm);
          _t_last_step = LT_current_time_us;
          _running = true;
        }
    }
    double getSpeed() {
      double rpm = 60000000.0 / ( (double)(_resolution) * _interval);
      return rpm;
    }
    
    // read the motor speed and avoid float calculation
    uint32_t getInterval() { return _interval; }
    uint16_t getResolution() { return _resolution; }
    
    // expects a position argument between 0 and _resolution
    // common _resolution values are 200, 400, 800, 1600
    // useful for setting index or zero position
    void setPosition(uint16_t position) {
      updatePosition(position);
    }
    
    // take the specified number of steps. (-) is anti clockwise
    void rotate(int16_t steps, double rpm) {
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
    int16_t distanceToGo() {
      return _steps_remaining;
    }
    
    // read the current postion. returns a value between 0 and _resolution
    uint16_t getPosition() {return _position;}
    
};

#endif //End __STEPPER__H__ include guard
