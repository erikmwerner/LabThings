#ifndef __SENSOR_H__
#define __SENSOR_H__

#include "device.h"

typedef void(*Callback) ();

/**************************************************************************/
/*! 
    @brief  Base Class for all Lab Things sensor devices.
    Subclasses must implement: type(), readSensor()
*/
/**************************************************************************/
class LT_Sensor : public LT_Device {
    bool _polling;
    uint32_t _polling_interval_us = 50000;//50ms
    
    Callback _newDataCallback = nullptr;
    
    private:
    volatile uint32_t _t_last_sample_us = 0;

  public:
    LT_Sensor(const uint8_t id) : LT_Device(id) {}
    
    virtual LT::DeviceType type() const = 0;

    bool isPolling() const {
      return _polling;
    }
    
    void setPolling(bool isPolling) {
      _polling = isPolling;
    }
    
    void setPollingInterval(uint32_t interval) {
      _polling_interval_us = interval;
    }
    
    uint32_t getPollingInterval() const {
      return _polling_interval_us;
    }
    
    void setNewDataCallback(Callback c) {
      _newDataCallback = c;
    }
    
    uint32_t lastSampleTime() const { return _t_last_sample_us; }
    
    virtual uint8_t readSensor() = 0;

    virtual void update() {
      if(_polling) {
        if( (LT_current_time_us - _t_last_sample_us) >= _polling_interval_us ) {
          if( readSensor() == 0) {
            (*_newDataCallback)();
          }
          _t_last_sample_us = LT_current_time_us;
        }
      }
    }
};

#endif // End __SENSOR_H__ include guard
