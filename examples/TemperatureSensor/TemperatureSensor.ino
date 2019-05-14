// Include the objects we need to use
#include <LabThings.h>
ASCIISerial messenger = ASCII_Serial(Serial, '<', ',', '>');

MessageHandler handler;

DeviceManager device_manager;

// Our MCU is connected to and LED on pin 13
LT_DigitalOutput led(device_manager.registerDevice(), 13);


void setup() {
  messenger.attachHandler(&handler);
  device_manager.attachDevice(led.UDID(), &led);

  handler.attachFunction(Fn_Get_Controller_Name, onGetControllerName);
  handler.attachFunction(Fn_Get_Controller_FW, onGetControllerFirmwareVersion);

  handler.attachFunction(Fn_Set_Digital_Output, onSetDigitalOutput);
  handler.attachFunction(Fn_Get_Digital_Output, onGetDigitalOutput);
}

void loop() {
  // update the messenger and all devices as often as possible
  messenger.update();
  device_manager.update();
}

// Define callback functions to use devices
//----------------------------------------------------------------------------//
void onGetControllerName() {}
void onGetControllerFirmwareVersion() {}

// all devices that can be set running will be set running
void onSetDigitalOutput() {
  int id = messenger.getNextArgInt();
  int value = messenger.getNextArgInt();
  device_manager.device(id)->setValue(value);
}

// all devices that can be set stopped will be set stopped
void onGetDigitalOutput() {
  int id = messenger.getNextArgInt();
  device_manager.device(id)->getValue();
  //send the value
}
