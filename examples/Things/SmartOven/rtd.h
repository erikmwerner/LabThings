#include <Adafruit_MAX31865.h>

/**************** Wrapper for Adaruit RTD Class ****************/
/* Define a class that inherits from the library and from LT_Sensor */
class LT_RTD : public Adafruit_MAX31865, public LT_Sensor {
    // The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RREF      430.0
    // The 'nominal' 0-degrees-C resistance of the sensor
    // 100.0 for PT100, 1000.0 for PT1000
#define RNOMINAL  101.0 // calibrated 2019 07 09

  public:
    LT_RTD(const int id, uint8_t cs, uint8_t dat_i, uint8_t dat_o, uint8_t clk)  :
      Adafruit_MAX31865(cs, dat_i, dat_o, clk), LT_Sensor(id) {}
      
    LT::DeviceType type() const {
      return (LT::DeviceType)(LT::UserType + 1);
    }
    virtual void* instance() {
      return this;
    }

    // implement library-specific functions
    void begin() {
      Adafruit_MAX31865::begin(MAX31865_2WIRE);
    }
    
    uint8_t readSensor() {
      uint16_t reading = Adafruit_MAX31865::readRTD();
      float ratio = reading;
      ratio /= 32768;
      return 0; // return 0 to report new data
    }

    float getTemperature() {
      return Adafruit_MAX31865::temperature(RNOMINAL, RREF);
      /*
        // Check and print any faults
        uint8_t fault = Adafruit_MAX31865::readFault();
        if (fault) {
         Serial.print("RTD Fault 0x"); Serial.println(fault, HEX);
         if (fault & MAX31865_FAULT_HIGHTHRESH) {
           Serial.println("RTD High Threshold");
         }
         if (fault & MAX31865_FAULT_LOWTHRESH) {
           Serial.println("RTD Low Threshold");
         }
         if (fault & MAX31865_FAULT_REFINLOW) {
           Serial.println("REFIN- > 0.85 x Bias");
         }
         if (fault & MAX31865_FAULT_REFINHIGH) {
           Serial.println("REFIN- < 0.85 x Bias - FORCE- open");
         }
         if (fault & MAX31865_FAULT_RTDINLOW) {
           Serial.println("RTDIN- < 0.85 x Bias - FORCE- open");
         }
         if (fault & MAX31865_FAULT_OVUV) {
           Serial.println("Under/Over voltage");
         }
         Adafruit_MAX31865::clearFault();
        }*/
    }
};
