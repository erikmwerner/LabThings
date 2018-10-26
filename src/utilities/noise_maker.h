#ifndef __NOISE_MAKER_H__
#define __NOISE_MAKER_H__

#include "../devices/device.h"

typedef void(*intCallback) (int);

enum NoiseType {
   Noise_Sin,
   Noise_Rand,
   Noise_Ramp
};

class Noise_Maker : public LT_Device {

    int _min_value = 0;
    int _max_value = 50;
    uint32_t _interval = 1000000; //1 second
    uint32_t _last_time = 0;
    intCallback _callback = NULL;
    NoiseType _noise_type = Noise_Sin;
    public:
    Noise_Maker(const uint8_t id, int min, int max)
     : LT_Device(id), _min_value(min), _max_value(max) {}

    
    void begin() {
        randomSeed(analogRead(0));
        _last_time = LT_current_time_us;
    }
    void loop() {
        if( (LT_current_time_us - _last_time) >= _interval) {
            if (_callback != NULL) {
          (*_callback)( makeNoise() );
        }
        _last_time = LT_current_time_us;
        }
    }
    void setCallback(intCallback c) {
      _callback = c;
    }
    
    void setInterval(uint32_t interval) {_interval = interval;}
    
    int makeNoise() {
    switch(_noise_type) {
      case Noise_Sin: {
        int scale = (_max_value - _min_value)/2;
        return ( scale * sin(0.00000314 * LT_current_time_us) ) + (scale);
      }
      case Noise_Rand: {
      break;
        //return random(_min, _max);
      }
      case Noise_Ramp: {
        break;
      }
    }
        
    }
    
};

#endif // End __NOISE_MAKER_H__ include guard