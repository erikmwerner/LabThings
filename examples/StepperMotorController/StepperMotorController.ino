/* StepperMotorController
 * Designed for Arduino CNC Shield
 * With 4 stepper motors
 * 
 *  Copyright (c) 2019 Erik Werner erikmwerner@gmail.com
 *  All rights reserved.
*/

#include <LabThings.h>

// give the controller a name
char controllerName[] = "4-Motor Stepper";

// define the message delimiters to avoid storing
// multiple copies and save a little RAM
const char SOM = '<';
const char SEP = ',';
const char EOM ='>';

// define messager and message handler
ASCIISerial messenger(Serial, SOM, SEP, EOM);
MessageHandler handler;

// define 4 stepper motors
#define N_DEVICES 4
DeviceManager<N_DEVICES> device_manager;

// (unique-id, step pin, dir pin)
// the order determines the unique-id of each motor
LT_Stepper m1(device_manager.registerDevice(), 2, 5); // uid = 0
LT_Stepper m2(device_manager.registerDevice(), 3, 6); // uid = 1
LT_Stepper m3(device_manager.registerDevice(), 4, 7); // uid = 2
LT_Stepper m4(device_manager.registerDevice(), 7, 13); // uid = 3

void setup() {
  // open the serial port 
  Serial.begin(115200);

  // set the callback function for when a message arrives
  messenger.setMessageReceivedCallback(onMessageReceived);

  // set the callback functions that implement commands
  handler.attachFunction(LT::Read_Name, onGetControllerName);
  handler.attachFunction(LT::Read_Name, onGetControllerFirmwareVersion);
  handler.attachFunction(LT::Read_Device_Count, onGetDeviceCount);
  handler.attachFunction(LT::Write_Speed, onSetMotorSpeed);
  handler.attachFunction(LT::Read_Speed, onGetMotorSpeed);
  handler.attachFunction(LT::Write_Position, onSetMotorPosition);
  handler.attachFunction(LT::Read_Position, onGetMotorPosition);
  handler.attachDefaultFunction(onUnknownCommand);

  // add the devides to the device manager
  device_manager.attachDevice(&m1);
  device_manager.attachDevice(&m2);
  device_manager.attachDevice(&m3);
  device_manager.attachDevice(&m4);

  // stepper motors default to 200 steps/revoulution
  // different resolutions can be set
  // example: 200 step motor with 1/8 step microstepping = 1600 steps/revolution
  // m1.setResolution(1600); 

  // enable motors
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);
}

void loop() {
  device_manager.update();
  messenger.update();
}

void onMessageReceived(int msg_id) {
  handler.handleMessage(msg_id);
}

void onGetControllerName() {
  Serial << SOM << LT::Read_Name
         << SEP << controllerName << EOM;
}

void onGetControllerFirmwareVersion() {
  Serial << SOM << LT::Read_Name
         << SEP << LT_VERSION << EOM;
}

void onGetDeviceCount() {
  Serial << SOM << LT::Read_Device_Count
         << SEP << device_manager.deviceCount() << EOM;
}

void onSetMotorSpeed() {
  int id = messenger.getNextArgInt();
  double rpm = messenger.getNextArgDouble();
  if (LT_Device* d = device_manager.device(id)) {
    if (d->type() == LT::Stepper) {
      LT_Stepper *stepper = d->instance();
      stepper->setSpeed(rpm);
      return;
    }
  }
  printError((uint8_t)LT::Write_Speed);
}
void onGetMotorSpeed() {
  int id = messenger.getNextArgInt();
  double rpm = messenger.getNextArgDouble();
  if (LT_Device* d = device_manager.device(id)) {
    if (d->type() == LT::Stepper) {
      LT_Stepper *stepper = d->instance();
      Serial << SOM << LT::Read_Speed
             << SEP << stepper->UDID()
             << SEP <<  stepper->getSpeed() << EOM;
      return;
    }
  }
  printError((uint8_t)LT::Read_Speed);
}

void onSetMotorPosition() {
  int id = messenger.getNextArgInt();
  int steps = messenger.getNextArgInt();
  double rpm = messenger.getNextArgDouble();
  if (LT_Device* d = device_manager.device(id)) {
    if (d->type() == LT::Stepper) {
      LT_Stepper *stepper = d->instance();
      stepper->rotate(steps, rpm);
      return;
    }
  }
  printError((uint8_t)LT::Write_Position);
}

void onGetMotorPosition() {
  int id = messenger.getNextArgInt();
  if (LT_Device* d = device_manager.device(id)) {
    if (d->type() == LT::Stepper) {
      LT_Stepper *stepper = d->instance();
      Serial << SOM << LT::Read_Position
             << SEP << stepper->UDID()
             << SEP << stepper->getPosition()
             << SEP << stepper->distanceToGo() << EOM;
      return;
    }
  }
  printError((uint8_t)LT::Read_Position);
}


void onUnknownCommand() {
  Serial << endl << "Lab Things v " << LT_VERSION
         << endl << controllerName
         << endl << F("Available command IDs: ") << endl;
  uint8_t count = handler.getAttachedFunctionCount();
  uint8_t function_list[count] = {0};
  handler.getAttachedFunctions(function_list);
  for (uint8_t i = 0; i < sizeof(function_list); ++i) {
    Serial << (function_list[i]);
    Serial << SEP;
  }
  Serial << endl;
}

void printError(const char *msg) {
  Serial << "Error: "<<msg;
}
