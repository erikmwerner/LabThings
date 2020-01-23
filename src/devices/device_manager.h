#ifndef __DEVICE_MANAGER_H__
#define __DEVICE_MANAGER_H__

#include "device.h"

template < uint8_t MAX_DEVICES >
class DeviceManager {
    int8_t n_devices = 0; ///< count of attached devices
    LT_Device* _dev[MAX_DEVICES] = {nullptr}; ///< pointers to all attached devices

    /*!
     * @brief SystemStats keeps track of loop time metrics to help estimate what kind of real-time deadlines
     * the system is capable of meeting.
     */
    struct SystemStats {
      uint8_t loop_count; ///< track the number of loops since the average loop time was computed
      uint32_t accumulator; ///< tracks the sum of the each loop length until the loop_count rollsover
      uint32_t max_loop_time; ///< track the longest loop time recorded in microseconds
      uint32_t avg_loop_time; ///< track the average loop time in microseconds
      uint32_t uptime_low; ///< the least significant 4 bytes of a 64-bit microsecond counter
      uint32_t uptime_high; ///< the most significant 4 bytes of a 64-bit microsecond counter
      SystemStats() : loop_count(0), accumulator(0), max_loop_time(0),
      avg_loop_time(0), uptime_low(0), uptime_high(0) {} 
      void update(const uint32_t now) 
      {
        ++loop_count;
        const uint32_t dt = now - LT_current_time_us;
        // update slowest loop time
        if(dt > max_loop_time) {
          max_loop_time = dt;
        }
        // update uptime clock
        if(uptime_low > LT_current_time_us) {
          // micros has rolled over
          uptime_high++;
        }
        uptime_low = LT_current_time_us;
        // update average loop time accumulator
        accumulator += dt;
        if(loop_count == 0) { // rolls over at 256 updates
          avg_loop_time = accumulator >> 8; // divide by 256
          accumulator = 0;
          loop_count = 0;
        }
      }
    };
    SystemStats _system_stats;

  public:
  
    /**************************************************************************/
    /*!
    @brief updates the global time with micros (4us resolution) and sequentially
    calls update() on all attached devices. This function expects at least one 
    device to be attached.
    */
    /**************************************************************************/
    void update() {
      
      // update global external time variable
      LT_current_time_us = micros();
      
      // iterate through devices and update
      // do while loop format from AVR optimization documents
      uint8_t i = n_devices;
      // do not loop if nothing is attached
      if(i == 0) {
        return;
      }
      else {
        do {
        if( _dev[i - 1] != nullptr ) {
          _dev[i - 1]->update();
         }
        } while( --i ); // stops update() when i==0
      }
     
     // update the statistics with the micros() value after devices are updated
      _system_stats.update(micros());
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
        return nullptr;
      }
    }

    /*!
     * @brief Get the Status object. This object contains information on the average
     * and maximum loop times as well as the total uptime of the system
     * 
     * @return SystemStats 
     */
    SystemStats getStatus() const {return _system_stats;}
};

#endif //End __DEVICE_MANAGER_H__ include guard
