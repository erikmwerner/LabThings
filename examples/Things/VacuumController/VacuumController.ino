/* 

 VacuumController runs a microcontroller system with:

1x Arduino Micro (32u4)
1x SSD 1306 OLED 128x64 px
1x Rotary encoder with pushbutton
1x Honeywell TruStability HSCDRRN015PDSA5 pressure sensor
1x 30N06L MOSFET driving a vacuum pump

Copyright (c) 2018 Erik Werner erikmwerner@gmail.com
All rights reserved.

*/

#include <HoneywellTruStabilitySPI.h>
#include "spi_wrapper.h"
#include <LabThings.h>
#include <EEPROM.h>

DeviceManager<7> device_manager;

// Use 1 page display buffer (256 bytes of SRAM)
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
UiContext context(&u8g2);
Ui ui(device_manager.registerDevice(), context);

LT_Encoder encoder(device_manager.registerDevice(), 11, 12, true);
LT_DebouncedButton button(device_manager.registerDevice(), 10, true);
LT_DigitalOutput motor(device_manager.registerDevice(), 5);
LT_AnalogSensor batteryMeter(device_manager.registerDevice(), A0);

MainMenu<7> screen_main(NULL, &context, "Menu");
NumberScreen<float> screen_pressure(&screen_main, &context, "Pressure (psi)", -15.0, 15.0);
ProgressBarScreen<int> screen_pump(&screen_main, &context, "Vacuum", 0, 1);
NumberScreen<float> screen_temperature(&screen_main, &context, "Temperature (C)", -100.0, 100.0);
NumberScreen<float> screen_battery(&screen_main, &context, "Battery (V)", 0, 55.0);
MenuScreen screen_graph(&screen_main, &context, "Graph");
GraphItem<float, 32> graph(&screen_graph, NULL, "t", "psi", 8, 8, 128);
TextScreen screen_usb(&screen_main, &context, "USB Ctrl", "USB");
AboutScreen screen_about(&screen_main, &context, "About");

LT_SPISensor sensor(device_manager.registerDevice(), SS, -15.0, 15.0);

LT_Timer save_timer(device_manager.registerDevice(), 5000000); // 5 seconds
#define EEPROM_ADDR_VAC 0
#define EEPROM_ADDR_PSI_T EEPROM_ADDR_VAC+=sizeof(int)

void setup() {
  Serial.begin(115200);
  SPI.begin();
  encoder.setValueChangedCallback(onEncoderValueChanged);
  button.setButtonReleasedCallback(onButtonReleased);
  screen_pump.setValueChangedCallback(onPumpValueChanged);

  sensor.setPolling(true);
  sensor.setPollingInterval(100000);// 10 Hz
  sensor.setNewDataCallback(onNewSensorData);

  batteryMeter.setPolling(true);
  batteryMeter.setPollingInterval(100000); // 10Hz
  batteryMeter.setNewDataCallback(onNewBatteryData);
  
  ui.setCurrentScreen(&screen_main);
  ui.setScreenSaverTimeout(60000000); // 60 s
  ui.setScreenSaverEnabled(true);

  graph.setGraphType(2);
  screen_graph.addChild(&graph);

  screen_main.addMenu(&screen_pressure);
  screen_main.addMenu(&screen_pump);
  screen_main.addMenu(&screen_temperature);
  screen_main.addMenu(&screen_battery);
  screen_main.addMenu(&screen_graph);
  screen_main.addMenu(&screen_usb);
  screen_main.addMenu(&screen_about);

  device_manager.attachDevice(&encoder);
  device_manager.attachDevice(&button);
  device_manager.attachDevice(&motor);
  device_manager.attachDevice(&ui);
  device_manager.attachDevice(&sensor);
  device_manager.attachDevice(&batteryMeter);
  device_manager.attachDevice(&save_timer);

  save_timer.setSingleShot(true);
  save_timer.setCallback(onEEPROMUpdateTimeout);

  loadSettings();
}

void loop() {
  device_manager.update();
}

void loadSettings() {
  int mot_value = 0;
  EEPROM.get(EEPROM_ADDR_VAC, mot_value);
  screen_pump.setValue(mot_value);

  /*uint32_t sense_period = 0;
  EEPROM.get(EEPROM_ADDR_PSI_T, mot_sense_period);
  sensor.setPollingInterval(sense_period);*/
}

void onButtonReleased() {
  ui.enter();
}

void onEncoderValueChanged() {
  static int last_value;
  int value = encoder.position();
  //update menu
  if (value > last_value) {
    ui.increment();
  }
  else {
    ui.decrement();
  }
  last_value = value;
}

// callback function executes when new data is read
void onNewSensorData() {
  screen_pressure.setValue(sensor.pressure());
  screen_temperature.setValue(sensor.temperature());
  graph.addDataPoint(LT_current_time_us/1000, sensor.pressure());
  
  Serial << "<" << Fn_Get_Sensor_Value
             << "," << sensor.UDID()
             << "," << sensor.pressure()
             << "," << sensor.temperature() << ">";
}

void onNewBatteryData() {
  int value = batteryMeter.value();
  float voltage = (float)(55*value)/1023.0;
  screen_battery.setValue(voltage);
}

void onPumpValueChanged() {
  int value = screen_pump.value();
  if(value == 0) {
    motor.setValue(0);
  }
  else {
    motor.setValue(1);
  }
  save_timer.start();
}

void onEEPROMUpdateTimeout() {
  int value = screen_pump.value();
  EEPROM.put(EEPROM_ADDR_VAC, value);
}
