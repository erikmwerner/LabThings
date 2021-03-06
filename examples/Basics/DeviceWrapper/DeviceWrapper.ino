/* 
 The DeviceWrapper example demonstrates how to define
 a class that inherits from LT_Device and another library.

 This allows code from other libraries to be used with 
 the Lab Things  messenger and multitasking system.

 To inherit additional functions, like the option to poll a sensor 
 or use pre-definied callbacks, see the SensorWrapper example.

  Copyright (c) 2018 Erik Werner erikmwerner@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
*/

#include <HoneywellTruStabilitySPI.h>
#include <LabThings.h>

DeviceManager<1> device_manager;

/**************** Wrapper Class ****************/
/* Define a class that inherits from the sensor's library and from LT_Device */
class LT_SPISensor : public TruStabilityPressureSensor, public LT_Device {

  public:
    // the constructor
    LT_SPISensor(const int id, uint8_t ss_pin, float p_min, float p_max)  :
    TruStabilityPressureSensor(ss_pin, p_min, p_max), LT_Device(id) {}

    // subclasses of LT_Device must implement type()
    LT::DeviceType type() const { return (LT::DeviceType)(LT::UserType + 1); }

    // implement library-specific functions in begin, update, or other added functions
    void begin() {
      TruStabilityPressureSensor::begin();
    }

    void update() {
      // TruStabilityPressureSensor::readSensor() returns 0 when new data is available
      if( TruStabilityPressureSensor::readSensor() == 0 ) {
        Serial.print("temp: ");
        Serial.print(TruStabilityPressureSensor::temperature());
        Serial.print(" pressure: ");
        Serial.println(TruStabilityPressureSensor::pressure());
      }
    }
    
};
/**************** /Wrapper Class ****************/

// Once the pressure sensor logic has been defined, the remaining code is short
LT_SPISensor pressure_sensor(device_manager.registerDevice(), SS, -15.0, 15.0);

void setup() {
  Serial.begin(115200);
  SPI.begin();
  device_manager.attachDevice(&pressure_sensor);
}

void loop() {
  device_manager.update();
}