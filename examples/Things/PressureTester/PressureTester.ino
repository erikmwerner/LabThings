/* 

 PressureTester provides USB communication for a 
 microcontroller system with:

1x Polulu AStar32u4 with 2x8 LCD and buzzer
1x EasyDriver connected to a stepper
2x Omega px139-015d4v sensors
2x SMC 5v solenoid valves

When no USB is connected, the 
build-in LCD will display pressure values.

Copyright (c) 2018 Erik Werner erikmwerner@gmail.com
All rights reserved.

*/

#include <AStar32U4.h>

AStar32U4LCD lcd;
AStar32U4Buzzer buzzer;
const char beepConnect[] PROGMEM = ">g32>>c32";
const char beepConfirm[] PROGMEM = "MS >g32 ML >g32";
const char beepError[] PROGMEM = "MS >g32>>c32>g32>>c32 ML";
const char beepDisconnect[] PROGMEM = ">>c32>g32";
const char beepDone[] PROGMEM = "MS >c16r16>c16>c16 ML >g4";
char controllerName[] = "AStar Pressure Tester";
bool usbConnected = false;

#include <LabThings.h>
ASCII_Serial messenger(Serial);
Message_Handler handler;

#define N_DEVICES 6
Device_Manager<N_DEVICES> device_manager;
LT_AnalogSensor ps1(device_manager.registerDevice(), A0);
LT_AnalogSensor ps2(device_manager.registerDevice(), A1);
LT_DigitalOutput v1(device_manager.registerDevice(), 9);
LT_DigitalOutput v2(device_manager.registerDevice(), 10);
LT_Stepper mot(device_manager.registerDevice(), 5, 4);
LT_Timer lcdUpdateTimer(device_manager.registerDevice(), 250000); //250ms = 4Hz

void setup() {
  Serial.begin(115200);

  messenger.setMessageReceivedCallback(onMessageReceived);

  handler.attachFunction(Fn_Get_Controller_Name, onGetControllerName); // 1
  handler.attachFunction(Fn_Get_Controller_FW, onGetControllerFirmwareVersion); // 2
  handler.attachFunction(Fn_Set_Polling_Interval, onSetPollingInterval); // 7
  handler.attachFunction(Fn_Get_Polling_Interval, onGetPollingInterval); // 8
  handler.attachFunction(Fn_Get_Sensor_Value, onGetSensorValue); // 11
  handler.attachFunction(Fn_Set_Digital_Output, onSetDigitalOutput); // 12
  handler.attachFunction(Fn_Get_Digital_Output, onGetDigitalOutput); // 13
  handler.attachFunction(Fn_Set_Stepper_RPM, onSetMotorSpeed); // 18
  handler.attachFunction(Fn_Set_Stepper_Position, onSetMotorPosition); // 20
  handler.attachFunction(Fn_Get_Stepper_Position, onGetMotorPosition); // 21
  handler.attachFunction(35, onTestSuccess);
  handler.attachDefaultFunction(onUnknownCommand);

  device_manager.attachDevice(&ps1);
  device_manager.attachDevice(&ps2);
  device_manager.attachDevice(&v1);
  device_manager.attachDevice(&v2);
  device_manager.attachDevice(&mot);

  // 24 steps/rev with a 20.83:1 gearing = 500 steps/revolution
  // easy driver defaults to 1/8 step microstepping = 4000 steps/rev
  mot.setResolution(4000);

  device_manager.attachDevice(&lcdUpdateTimer);

  lcdUpdateTimer.setCallback(onLcdUpdateTimerTimeout);

  checkConnection();

  if (!usbConnected) {
    lcd.clear();
    lcd.gotoXY(0, 0);
    lcd << "P1:pinA0";
    lcd.gotoXY(0, 1);
    lcd << "P2:pinA1";
    delay(2000);
    lcd.clear();
    lcd.gotoXY(0, 0);
    lcd << "V1:pin09";
    lcd.gotoXY(0, 1);
    lcd << "V2:pin10";
    delay(2000);
    lcd.clear();
    lcd.gotoXY(0, 0);
    lcd << "P1:";
    lcd.gotoXY(0, 1);
    lcd << "P2:";
  }
}

void loop() {
  device_manager.loop();
  messenger.loop();
  checkConnection();
}

void onMessageReceived(int msg_id) {
  handler.handleMessage(msg_id);
}

void onGetControllerName() {
  Serial << "<" << Fn_Get_Controller_Name
         << "," << controllerName << ">";
}

void onGetControllerFirmwareVersion() {
  Serial << "<" << Fn_Get_Controller_FW
         << "," << LT_VERSION << ">";
}

void onGetDeviceCount() {
  Serial << "<" << Fn_Get_Device_Count
         << "," << device_manager.deviceCount() << ">";
}

void onSetDigitalOutput() {
  int id = messenger.getNextArgInt();
  int value = messenger.getNextArgInt();
  if (LT_Device* d = device_manager.device(id)) {
    if(d->type() == LT::DigitalOutput) {
    LT_DigitalOutput *valve = d->instance();
      valve->setValue(value);
      playTune(beepConfirm);
      return;
    }
  }
  playTune(beepError);
}

void onGetDigitalOutput() {
  int id = messenger.getNextArgInt();
  int value = messenger.getNextArgInt();
  if (LT_Device* d = device_manager.device(id)) {
    if(d->type() == LT::DigitalOutput) {
      LT_DigitalOutput *valve = d->instance();
      Serial << "<" << Fn_Get_Digital_Output
             << "," << valve->UDID()
             << "," << valve->getValue() << ">";
      return;
    }
  }
  playTune(beepError);
}

void onSetPollingInterval() {
  int id = messenger.getNextArgInt();
  long value = messenger.getNextArgULong();
  if (LT_Device* d = device_manager.device(id)) {
    if(d->type() == LT::AnalogSensor) {
      LT_AnalogSensor *sensor = d->instance();
      sensor->setPollingInterval(value);
      playTune(beepConfirm);
      return;
    }
  }
  playTune(beepError);
}

void onGetPollingInterval() {
  int id = messenger.getNextArgInt();
  long value = messenger.getNextArgULong();
  if (LT_Device* d = device_manager.device(id)) {
    if(d->type() == LT::AnalogSensor) {
      LT_AnalogSensor *sensor = d->instance();
      Serial << "<" << Fn_Get_Polling_Interval
             << "," << sensor->UDID()
             << "," << sensor->getPollingInterval() << ">";
      return;
    }
  }
  playTune(beepError);
}

void onGetSensorValue() {
  int id = messenger.getNextArgInt();
  if (LT_Device* d = device_manager.device(id)) {
    if(d->type() == LT::AnalogSensor) {
      LT_AnalogSensor *sensor = d->instance();
      sensor->readSensor();
      Serial << "<" << Fn_Get_Sensor_Value
             << "," << sensor->UDID()
             << "," << sensor->value() << ">";
      return;
    }
  }
  playTune(beepError);
}

void onSetMotorSpeed() {
  int id = messenger.getNextArgInt();
  double rpm = messenger.getNextArgDouble();
  if (LT_Device* d = device_manager.device(id)) {
    if(d->type() == LT::Stepper) {
      LT_Stepper *stepper = d->instance();
      stepper->setSpeed(rpm);
      return;
    }
  }
  playTune(beepError);
}

void onSetMotorPosition() {
  int id = messenger.getNextArgInt();
  int steps = messenger.getNextArgInt();
  double rpm = messenger.getNextArgDouble();
  if (LT_Device* d = device_manager.device(id)) {
    if(d->type() == LT::Stepper) {
      LT_Stepper *stepper = d->instance();
      stepper->rotate(steps, rpm);
      return;
    }
  }
  playTune(beepError);
}

void onGetMotorPosition() {
  int id = messenger.getNextArgInt();
  if (LT_Device* d = device_manager.device(id)) {
    if(d->type() == LT::Stepper) {
      LT_Stepper *stepper = d->instance();
      Serial << "<" << Fn_Get_Stepper_Position
             << "," << stepper->UDID()
             << "," << stepper->getPosition()
             << "," << stepper->distanceToGo() << ">";
      return;
    }
  }
  playTune(beepError);
}

void onTestSuccess() {
  playTune(beepDone);
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
    Serial << ",";
  }
  Serial << endl;
}

void checkConnection() {
  if (USBDevice.configured() != usbConnected) {
    usbConnected = !usbConnected;
    if (usbConnected) {
      playTune(beepConnect);
      lcd.clear();
      lcd.gotoXY(0, 0);
      lcd << "USB CTRL";
      lcd.gotoXY(0, 1);
      lcd << "FW: " << LT_VERSION;
    }
    else {
      playTune(beepDisconnect);
      lcd.clear();
      lcd.gotoXY(0, 0);
      lcd << "P1: ";
      lcd.gotoXY(0, 1);
      lcd << "P2: ";
    }
  }
}

// convert pressure values and show on LCD
// if not connected to USB
void onLcdUpdateTimerTimeout() {
  if (usbConnected) {
    return;
  }
  else {
    static float SLOPE = (30.0 / 818.4);
    static float INTERCEPT = -16.875;
    ps1.readSensor();
    // Print a string
    lcd.gotoXY(3, 0);
    lcd << ( ( (float)ps1.value() * SLOPE ) + INTERCEPT );
    ps2.readSensor();
    // Go to the next line
    lcd.gotoXY(3, 1);
    lcd << ( ( (float)ps2.value() * SLOPE ) + INTERCEPT );
  }
}

void playTune(const char* t) {
  buzzer.playFromProgramSpace(t);
}

