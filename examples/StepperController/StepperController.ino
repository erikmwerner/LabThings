#include <LabThings.h>
ASCII_Serial messenger = ASCII_Serial(Serial, '<', ',', '>');

Message_Handler handler;

Device_Manager<5> device_manager;

// Our MCU is connected to:
LT_Encoder encoder(device_manager.registerDevice(), 2, 3, true);

LT_DebouncedButton button(device_manager.registerDevice(), 4, true);

U8G2_SSD1306_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
Menu menu(device_manager.registerDevice(), &u8g2);

ListScreen<3> screen_main(NULL, "Main Menu");
ProgressBarScreen screen_m1(&screen_main, "Motor 1", 0, 60);
ProgressBarScreen screen_m2(&screen_main, "Motor 2", 0, 60 );
ProgressBarScreen screen_settings(&screen_main, "Start/Stop", 0, 1);

// 2 stepper motors
LT_Stepper stepper1(device_manager.registerDevice(), 8, 9);
LT_Stepper stepper2(device_manager.registerDevice(), 10, 11);

void setup() {
  menu.setCurrentScreen(&screen_main);
  menu.setScreenSaverTimeout(0); // prevent sleeping screen
  screen_main.addMenu(&screen_m1);
  screen_main.addMenu(&screen_m2);
  screen_main.addMenu(&screen_settings);
  
  messenger.setMessageReceivedCallback(onMessageReceived);
  device_manager.attachDevice(&stepper1);
  device_manager.attachDevice(&stepper2);
  
  device_manager.attachDevice(&encoder);
  device_manager.attachDevice(&button);
  device_manager.attachDevice(&menu);
  
  screen_m1.setValueChangedCallback(onMotor1SpeedChanged);
  screen_m2.setValueChangedCallback(onMotor2SpeedChanged);
  
  encoder.setValueChangedCallback(onEncoderValueChanged);
  button.setButtonReleasedCallback(onButtonReleased);

  handler.attachFunction(Fn_Set_Controller_Name, onSetControllerName);
  handler.attachFunction(Fn_Get_Controller_Name, onGetControllerName);
  handler.attachFunction(Fn_Get_Controller_FW, onGetControllerFirmwareVersion);
  handler.attachFunction(Fn_Get_Device_Count, onGetAttachedDeviceCount);
  handler.attachFunction(Fn_Set_Device_Name, onSetDeviceName);
  handler.attachFunction(Fn_Get_Device_Name, onGetDeviceName);
  handler.attachFunction(Fn_Get_System_Status, onGetSystemStatus);

  handler.attachFunction(Fn_Start, onStart);
  handler.attachFunction(Fn_Stop, onStop);

  handler.attachFunction(Fn_Set_Stepper_RPM, onSetRpm);
}

void loop() {
  // update the messenger and all devices as often as possible
  messenger.loop();
  device_manager.loop();
}

// Define callback functions to use devices
//----------------------------------------------------------------------------//
void onSetControllerName() {}
void onGetControllerName() {}
void onGetControllerFirmwareVersion() {}
void onGetAttachedDeviceCount() {}
void onSetDeviceName() {}
void onGetDeviceName() {}
void onGetSystemStatus() {}

// all devices that can be set running will be set running
void onStart() {}

// all devices that can be set stopped will be set stopped
void onStop() {}

void onMessageReceived(int arg1) {

}

void onMotor1SpeedChanged(int speed) {
  stepper1.setSpeed(speed);
}
void onMotor2SpeedChanged(int speed) {
  stepper2.setSpeed(speed);
}

// set the speed of a device
// arg1 device id
// arg2 rpm
void onSetRpm() {
  int id = messenger.getNextArgInt();
  int rpm = messenger.getNextArgInt();
  //device_manager.device(id)->setRpm(rpm);
}

void onButtonReleased() {
  menu.enter();
}

void onEncoderValueChanged(int value) {
  static int last_value;
  //update menu
  if (value > last_value) {
    menu.increment();
  }
  else {
    menu.decrement();
  }
  last_value = value;
}
