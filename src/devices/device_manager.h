#ifndef __DEVICE_MANAGER_H__
#define __DEVICE_MANAGER_H__

#include "device.h"

template < uint8_t MAX_DEVICES >
class DeviceManager {
    int8_t n_devices = 0; ///< count of attached devices
    LT_Device* _dev[MAX_DEVICES] = {NULL}; ///< pointers to all attached devices

    struct LoopStats {
      uint8_t loop_count;
      uint32_t accumulator;
      uint32_t max_loop_time;
      uint32_t avg_loop_time;
      LoopStats() : loop_count(0), accumulator(0), max_loop_time(0),avg_loop_time(0) {} 
      void update(const uint32_t now) 
      {
        ++loop_count;
        uint32_t dt = now - LT_current_time_us;
        if(dt > max_loop_time) {
          max_loop_time = dt;
        }
        accumulator += dt;
        if(loop_count == 0) { // rolls over at 256 updates
          avg_loop_time = accumulator >> 8; // divide by 256
          accumulator = 0;
          loop_count = 0;
          //Serial.print("max: ");
          //Serial.print(max_loop_time);
          //Serial.print(" avg: ");
          //Serial.println(avg_loop_time);
        }
      }
    };
    LoopStats _loop_stats;

  public:
  
    /**************************************************************************/
    /*!
    @brief updates the global time with micros (4us resolution) and sequentially
    calls update() on all attached devices. This function expects at least one 
    device to be attached.
    */
    /**************************************************************************/
    void update() {
    
      //static int updateCounter = 0;
      //static uint32_t update_time_start = 0;
      
      // update global external time variable
      LT_current_time_us = micros();
      
      uint8_t i = n_devices;
      do {
        if( _dev[i - 1] != NULL ) {
          _dev[i - 1]->update();
         }
      } while( --i ); // stops update() when i==0

      _loop_stats.update(micros());
    }
    
    
    /**************************************************************************/
    /*!
    @brief Generates a new device ID, a unique identifier (UDID)
    NOTE: UDID is zero-indexed
    @return the index of the next available spot in the device list
    */
    /**************************************************************************/
    uint8_t registerDevice() {
      return n_devices < MAX_DEVICES ? n_devices++ : -1;
    }
    
    
    /**************************************************************************/
    /*!
    @brief  insert a device into the device list
    @param d a pointer to the device to atttach
    @return  the id if the device was inserted successfuly
    */
    /**************************************************************************/
    int8_t attachDevice(LT_Device* d) {
      uint8_t id = d->UDID();
      if (id >= 0 && id < MAX_DEVICES)  {
        _dev[id] = d;
        d->begin();
      }
      else {
#ifdef DEBUG_PRINT
          Serial.print("WARNING: Device_Manager failed to attache device ID:");
          Serial.println(id);
#endif
      }
      return id;
    }
    
    /**************************************************************************/
    /*!
    @brief get the number of devices attached to this device manager
    NOTE: count is one-indexed, device IDs are zero-indexed
    @return  the number of devices
    */
    /**************************************************************************/
    const int8_t deviceCount() {
      return n_devices;
    }

   
   
    /**************************************************************************/
    /*!
    @brief 
    @param udid the index of the device to access
    @return a pointer to the device at index udid. If there is no device, returns NULL
    */
    /**************************************************************************/
    LT_Device* device(const int udid) {
      if (udid < n_devices) {
        return _dev[udid];
      }
      else {
        return NULL;
      }
    }

    LoopStats getStatus() const {return _loop_stats;}
};

#endif //End __DEVICE_MANAGER_H__ include guard
