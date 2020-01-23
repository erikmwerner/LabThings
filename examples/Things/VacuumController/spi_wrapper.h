#include <LabThings.h>

class LT_SPISensor : public TruStabilityPressureSensor, public LT_Sensor {

  public:
    LT_SPISensor(const int id, uint8_t ss_pin, float p_min, float p_max)  :
    TruStabilityPressureSensor(ss_pin, p_min, p_max), LT_Sensor(id) {}

    // subclasses of LT_Device must implement type()
    LT::DeviceType type() { return (LT::DeviceType)(LT::UserType + 1); }

    // it is not required to implement instance(), but messenger applications may need it
    virtual void* instance() {return this;}

    // implement library-specific functions 
    void begin() {
      TruStabilityPressureSensor::begin();
    }
    uint8_t readSensor() {
      return TruStabilityPressureSensor::readSensor();
    }
};
