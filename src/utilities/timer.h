#ifndef __TIMER_H__
#define __TIMER_H__

#include "../devices/device.h"

typedef void(*voidCallback) ();

class LT_Timer : public LT_Device {
  uint32_t _last_time = 0;
  uint32_t _timeout = 0;
  voidCallback _callback = NULL;
  bool _isSingleShot = false;
  bool _isActive = true;
  
  public:
    LT_Timer(const uint8_t id, uint32_t timeout) : LT_Device(id), _timeout(timeout) {}
    
    LT::DeviceType type() {return LT::Timer;}
     void setCallback(voidCallback c) {
      _callback = c;
    }
    
    void setTimeout(uint32_t interval) {_timeout = interval;}
    
    void begin() {
        _last_time = LT_current_time_us;
    }
    
    void setSingleShot(bool isSingleShot) {
      _isSingleShot = isSingleShot;
    }
    
    void start() {
      _last_time = LT_current_time_us;
      _isActive = true;
    }
    
    void stop() {
      _isActive = false;
    }
    
     void loop() {
       if(_isActive) {
        if( (LT_current_time_us - _last_time) >= _timeout) {
            if (_callback != NULL) {
              (*_callback)();
            }
            if(_isSingleShot) {
              _isActive = false;
            }
            else {
              do {
                _last_time += _timeout;
              } while ( (LT_current_time_us - _last_time) >= _timeout );
            }
        } // is expired
      } // isActive
    }
};



#endif // End __TIMER_H__ include guard