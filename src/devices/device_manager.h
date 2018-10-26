#ifndef __DEVICE_MANAGER_H__
#define __DEVICE_MANAGER_H__

#include "device.h"

template < uint8_t MAX_DEVICES >
class Device_Manager {
    // count of attached devices
    int8_t n_devices = 0;
    // list of all attached devices
    LT_Device* _dev[MAX_DEVICES] = {NULL};

  public:
  
    // updates the global time with micros (4us resolution) and sequentially
    // calls loop to update all attached devices
    void loop() {
    
      //static int loopCounter = 0;
      //static uint32_t loop_time_start = 0;
      
      LT_current_time_us = micros();
      
      uint8_t i = n_devices;
      do {
        if( _dev[i - 1] != NULL ) {
          _dev[i - 1]->loop();
         }
      } while( --i ); // stops loop when i==0
      
      /*
      for( uint8_t i = 0; i < n_devices; ++i ) {
        if( _dev[i] != NULL ) {
          _dev[i]->loop();
        }
      }*/
      
      /*++loopCounter;
      if( (LT_current_time_us - loop_time_start) > 1000000 ) {
      	Serial.print("loops: ");
      	Serial.println(loopCounter);
      	loopCounter = 0;
      	loop_time_start = LT_current_time_us;
      }*/
    }
    
    // return the index of the next available spot in the device list
    // this becomes the device ID, a unique identifier
    // NOTE: UDID is zero-indexed
    uint8_t registerDevice() {
      return n_devices < MAX_DEVICES ? n_devices++ : -1;
    }
    
    
    // insert a device into the device list
    // returns the id if the device was inserted successfuly
    int8_t attachDevice(LT_Device* d) {
      uint8_t id = d->UDID();
      if (id >= 0 && id < MAX_DEVICES)  {
        _dev[id] = d;
        d->begin();
      }
      else {
#ifdef DEBUG_PRINT
          Serial.print("WARNING: Device_Manager failed to register device. ID:");
          Serial.println(id);
#endif
      }
      return id;
    }

    // return the number of devices
    // NOTE: count is one-indexed, device IDs are zero-indexed
    const int8_t deviceCount() {
      return n_devices;
    }

    // if there is a device with matching id,
    // reuturn a pointer to the device
    LT_Device* device(const int udid) {
      if (udid < n_devices) {
        return _dev[udid];
      }
      else {
        return NULL;
      }
    }
};


#endif //End __DEVICE_MANAGER_H__ include guard
