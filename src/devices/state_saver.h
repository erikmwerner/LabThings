#ifndef __STATE_SAVER_H__
#define __STATE_SAVER_H__

#include <EEPROMex.h>
#include <EEPROMVar.h>

// make sure debug mode is turned off
#ifdef _EEPROMEX_DEBUG
  #undef _EEPROMEX_DEBUG
#endif

#include "device.h"

typedef void(*voidCallback) ();

class LT_State_Saver : public LT_Device {
    voidCallback _callback = NULL;
    bool _dirty = false;
    uint32_t _last_eeprom_write_us;
    uint32_t _delayed_write_interval_us = 10000000; //10 s

  public:
    LT_State_Saver(const uint8_t id) : LT_Device(id) {}
    
    void setTimeoutCallback(voidCallback c) {
      _callback = c;
    }

    void begin() {
      _last_eeprom_write_us = LT_current_time_us;
      EEPROM.setMemPool(0, EEPROMSizeUno);
      // Set maximum allowed writes to maxAllowedWrites.
      // More writes will only give errors when _EEPROMEX_DEBUG is set
      EEPROM.setMaxAllowedWrites(10);
    }

    void setDirty(bool isDirty) {
      _dirty = isDirty;
    }

    void loop() {
      if ( (LT_current_time_us - _last_eeprom_write_us) >= _delayed_write_interval_us ) {
        if (_dirty) {
          if (_callback != NULL) {
            (*_callback)();
          }
          _last_eeprom_write_us = LT_current_time_us;
          _dirty = false;
        }
      }
    }

};
#endif // End __STATE_SAVER_H__ include guard
