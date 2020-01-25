/* 
 Lab Things Thermometer Example
 ESP8266
 128 x 32px OLED (SSD1306)
 Digital button input
 1-wire communication DS18B20 digital temperature sensor
 USB Serial
 WiFi
 
 Copyright (c) 2020 Erik Werner erikmwerner@gmail.com
 All rights reserved.

*/

// Include the objects we need to use
#include <LabThings.h>

#include <OneWire.h>

class LT_DS18B20 : public OneWire, public LT_Sensor {

    uint8_t addr[8]; // store the 64-bit address of the sensor
    uint8_t data[12];
    uint8_t present = 0;
    bool has_sensor = false;
    bool type_s = false;
    float celsius = 0; // the last measured
    uint32_t converstion_start_time = 0;
    
    void requestData() {
      OneWire::reset();
      OneWire::select(addr);
      OneWire::write(0x44, 1); // start conversion, with parasite power on at the end
      converstion_start_time = LT_current_time_us;
    }

  public:
    LT_DS18B20(const int id, uint8_t pint)  :
      OneWire(pint), LT_Sensor(id) {}

    // subclasses of LT_Device must implement type()
    LT::DeviceType type() const {
      return (LT::DeviceType)(LT::UserType + 1);
    }

    // implement library-specific functions
    void begin() {
      // find the first sensor on the bus. Returns true if a sensor was found.
      has_sensor = OneWire::search(addr);
      if(has_sensor) {
         /*Serial.print("ROM =");
      for (uint8_t i = 0; i < 8; i++) {
        Serial.write(' ');
        Serial.print(addr[i], HEX);
      }*/

      if (OneWire::crc8(addr, 7) != addr[7]) {
        Serial.println("CRC is not valid!");
        return;
      }
      // the first ROM byte indicates which chip
      switch (addr[0]) {
        case 0x10:
          Serial.println("  Chip = DS18S20");  // or old DS1820
          type_s = 1;
          break;
        case 0x28:
          Serial.println("  Chip = DS18B20");
          type_s = 0;
          break;
        case 0x22:
          Serial.println("  Chip = DS1822");
          type_s = 0;
          break;
        default:
          Serial.println("Device is not a DS18x20 family device.");
          return;
      }
      requestData();
      }
    }
    // this function gets called every time the sensor is polled
    uint8_t readSensor() {
      if(!has_sensor) return 2;
      // the sensor defaults to 12-bit. Conversions take Max 750 ms
      if (LT_current_time_us - converstion_start_time > 750000) { // 750 ms
        // New data is ready. Read and convert it
        present = OneWire::reset();
        OneWire::select(addr);
        OneWire::write(0xBE);         // Read Scratchpad
        for (uint8_t i = 0; i < 9; i++) {           // we need 9 bytes
          data[i] = OneWire::read();
          //Serial.print(data[i], HEX);
          //Serial.print(" ");
        }
        //Serial.print(" CRC=");
        //Serial.print(OneWire::crc8(data, 8), HEX);
        //Serial.println();
        // Convert the data to actual temperature
        // because the result is a 16 bit signed integer, it should
        // be stored to an "int16_t" type, which is always 16 bits
        // even when compiled on a 32 bit processor.
        int16_t raw = (data[1] << 8) | data[0];
        if (type_s) {
          raw = raw << 3; // 9 bit resolution default
          if (data[7] == 0x10) {
            // "count remain" gives full 12 bit resolution
            raw = (raw & 0xFFF0) + 12 - data[6];
          }
        } else {
          uint8_t cfg = (data[4] & 0x60);
          // at lower res, the low bits are undefined, so let's zero them
          if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
          else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
          else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
          //// default is 12 bit resolution, 750 ms conversion time
        }
        celsius = (float)raw / 16.0;
        //Serial.println("got new temp!");
        requestData(); // request more data right away
        return 0;
      }
      else {
        return 1; // still waiting on data;
      }
    }
    float temperatureCelcius() {
      return celsius;
    }
};

char controllerName[] = "Lab Things DS18B20";

// define the message delimiters to avoid storing
// multiple copies and save a little RAM
const char SOM = '<';
const char SEP = ',';
const char EOM ='>';

ASCIISerial messenger(Serial, SOM, SEP, EOM);

MessageHandler handler;
DeviceManager<6> device_manager;

// The OneWire data wire is plugged into pin 14 on the Arduino
LT_DS18B20 temp_sensor( device_manager.registerDevice(), 14 );

// Adafruit Feather ESP8266/32u4 Boards + FeatherWing OLED
// Adafruit FeatherWing display i2c address: 0x3C
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);

// Buttons on Adafruit FeatherWing
LT_DebouncedButton buttonA(device_manager.registerDevice(), 0, true);
LT_DebouncedButton buttonB(device_manager.registerDevice(), 16, true);
LT_DebouncedButton buttonC(device_manager.registerDevice(), 2, true);

// Declare ui elements
UiContext context(&u8g2, 2, u8g2_font_helvB08_tr, u8g2_font_tom_thumb_4x6_tr, u8g2_font_tom_thumb_4x6_tr);
Ui ui(device_manager.registerDevice(), context);
MainMenu<3> screen_main(NULL, &context, "Menu");
NumberScreen<float> screen_temperature(&screen_main, &context, "Temperature (C)", -100.0, 100.0);
MenuScreen screen_graph(&screen_main, &context, "Graph");
GraphItem<float, 32> graph(&screen_graph, NULL, "time [ms]", "T (C)", 8, 0, 128, 24);
AboutScreen screen_about(&screen_main, &context, "About");


void setup() {

  Serial.begin(115200);

  temp_sensor.setPolling(true);
  temp_sensor.setPollingInterval(100000); // 100 ms
  // when the sensor returns 0 (has new data), execute this function
  temp_sensor.setNewDataCallback(onNewSensorData);

  ui.setCurrentScreen(&screen_main);
  screen_main.addScreen(&screen_temperature);

  graph.setGraphType(2);
  screen_graph.addChild(&graph);
  screen_main.addScreen(&screen_graph);
  screen_main.addScreen(&screen_about);

  messenger.setMessageReceivedCallback(onMessageReceived);

  buttonA.setButtonReleasedCallback(onButtonAReleased);
  buttonB.setButtonReleasedCallback(onButtonBReleased);
  buttonC.setButtonReleasedCallback(onButtonCReleased);
  device_manager.attachDevice(&temp_sensor);
  device_manager.attachDevice(&buttonA);
  device_manager.attachDevice(&buttonB);
  device_manager.attachDevice(&buttonC);
  device_manager.attachDevice(&ui);

  handler.attachFunction(LT::Read_Name, onGetControllerName);
  handler.attachFunction(LT::Read_Version, onGetControllerFirmwareVersion);
  handler.attachFunction(LT::Read_Sensor_Value, onReadSensorValue);
}

void loop() {
  // update the messenger and all devices as often as possible
  messenger.update();
  device_manager.update();
}

void onMessageReceived(int msg_id) {
  handler.handleMessage(msg_id);
}

void onNewSensorData() {

  Serial << SOM << LT::Read_Sensor_Value
             << SEP << temp_sensor.UDID()
             << SEP << temp_sensor.temperatureCelcius()
             << EOM;
    
  screen_temperature.setValue(temp_sensor.temperatureCelcius());
  graph.addDataPoint(LT_current_time_us / 1000, temp_sensor.temperatureCelcius());
}

void onButtonAReleased() {
  ui.decrement();
}

void onButtonBReleased() {
  ui.enter();
}

void onButtonCReleased() {
  ui.increment();
}

// Define callback functions to use devices
//----------------------------------------------------------------------------//
void onGetControllerName(void*) {
  Serial << SOM << LT::Read_Name
         << SEP << controllerName << EOM;
}

void onGetControllerFirmwareVersion(void*) {
  Serial << SOM << LT::Read_Name
         << SEP << LT_VERSION << EOM;
}

void onReadSensorValue(void*) {
  // there is only 1 sensor, so skip ID checking
  if (temp_sensor.readSensor() == 0) {
    //onNewSensorData();
  }
  else {
    printError((const char *)LT::Read_Sensor_Value);
  }
}

void printError(const char *msg) {
  Serial << "Error: "<<msg;
}