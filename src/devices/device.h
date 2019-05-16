#ifndef __DEVICE__H__
#define __DEVICE__H__

typedef void(*cb) (int);

namespace LT
{
    enum DeviceType : uint8_t
    {
        NoType = 0,
        DigitalOutput = 1,
        AnalogOutput = 2,
        DigitalSensor = 3,
        AnalogSensor = 4,
        DebouncedButton = 5,
        Encoder= 6,
        Stepper = 7,
        Ui = 8,
        Timer = 9,
        UserType = 10
    };
}

class LT_Device {
    const uint8_t _udid;
    
  public:
    LT_Device(const int id) : _udid(id) {}
    virtual LT::DeviceType type() const {return LT::NoType;}
    const int8_t UDID(){return _udid;}
    virtual void update() {}
    virtual void begin() {}
    virtual void* instance() {return nullptr;}
};

// bring in a global external variable to keep track of time
extern uint32_t LT_current_time_us;

#endif //End __DEVICE__H__ include guard
