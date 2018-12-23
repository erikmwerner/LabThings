/* 
 The SensorWrapper example demonstrates how to define
 a class that inherits from LT_Sensor and another library.

 This allows code from other libraries to be used with 
 the Lab Things callback, messenger, and multitasking system.

 See Also: The DeviceWrapper example

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

Device_Manager<1> device_manager;

/**************** Wrapper Class ****************/
/* Define a class that inherits from the library and from LT_Sensor */
class LT_SPI_Sensor : public TruStability_PressureSensor, public LT_Sensor {

  public:
    LT_SPI_Sensor(const int id, uint8_t ss_pin, float p_min, float p_max)  :
    TruStability_PressureSensor(ss_pin, p_min, p_max), LT_Sensor(id) {}

    // subclasses of LT_Device must implement type()
    LT::DeviceType type() { return (LT::DeviceType)(LT::UserType + 1); }

    // it is not required to implement instance(), but messenger applications may need it
    virtual void* instance() {return this;}

    // implement library-specific functions 
    void begin() {
      TruStability_PressureSensor::begin();
    }
    uint8_t readSensor() {
      return TruStability_PressureSensor::readSensor();
    }
    
};

/**************** /Wrapper Class ****************/

// Once the pressure sensor logic has been defined, the remaining code is short
LT_SPI_Sensor pressure_sensor(device_manager.registerDevice(), SS, -15.0, 15.0);

void setup() {
  Serial.begin(115200);
  device_manager.attachDevice(&pressure_sensor);
  
  // inheriting from LT_Sensor allows 
  // polling and callback features to be re-used
  // from the Lab Things library
  pressure_sensor.setPolling(true);
  pressure_sensor.setPollingInterval(500000);
  pressure_sensor.setNewDataCallback(onNewSensorData);
}

void loop() {
  device_manager.loop();
}

// callback function executes when new data is read
void onNewSensorData() {
  Serial.print("temp: ");
  Serial.print(pressure_sensor.temperature());
  Serial.print(" pressure: ");
  Serial.print(pressure_sensor.pressure());
  Serial.print(" sample time: ");
  Serial.println(pressure_sensor.lastSampleTime());
}