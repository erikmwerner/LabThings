#include <LabThings.h>

#define N_DEVICES 5
DeviceManager<N_DEVICES> device_manager;

LT_Encoder encoder(device_manager.registerDevice(), 2, 3, true);
LT_DebouncedButton button(device_manager.registerDevice(), 4, true);
LT_AnalogSensor sensor1(device_manager.registerDevice(), 14);
LT_AnalogSensor sensor2(device_manager.registerDevice(), 15);

U8G2_SSD1306_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
UiContext context(&u8g2);
Ui menu(device_manager.registerDevice(), context);

MainMenu<3> screen_main(NULL, &context, "Menu");
InputScreen<float> inputScreen1(&screen_main, "Sensor 1", -30.0, 30.0);
InputScreen<float> inputScreen2(&screen_main, "Sensor 2", -30.0, 30.0);
GraphScreen<int, 64> screen_graph(&screen_main, "Graph", "Time", "Pressure (psig)");

void setup() {
  Serial.begin(115200);
  menu.setCurrentScreen(&screen_main);
  menu.setSleepTimeout(0); // prevent sleeping screen

  screen_main.addMenu(&inputScreen1);
  screen_main.addMenu(&inputScreen2);
  screen_main.addMenu(&screen_graph);

  device_manager.attachDevice(sensor1.UDID(), &sensor1);
  device_manager.attachDevice(sensor2.UDID(), &sensor2);
  device_manager.attachDevice(encoder.UDID(), &encoder);
  device_manager.attachDevice(button.UDID(), &button);
  device_manager.attachDevice(menu.UDID(), &menu);

  encoder.setValueChangedCallback(onEncoderValueChanged);
  button.setButtonReleasedCallback(onButtonReleased);

  sensor1.setPolling(1000000);
  sensor2.setPolling(1000000);

  sensor1.setNewSampleCallback(onSensor1Data);
  sensor2.setNewSampleCallback(onSensor2Data);
}

void loop() {
  // put your main code here, to run repeatedly:
  device_manager.update();
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

void onSensor1Data(int value) {
  //Serial.print("sensor 1: ");
  //Serial.println(value);
  float psi = convertToPSI(value);
  inputScreen1.setValue(psi);
  screen_graph.addDataPoint( (LT_current_time_us/1000) , psi);
}


void onSensor2Data(int value) {
  float psi = convertToPSI(value);
  inputScreen2.setValue(psi);
}

float convertToPSI(int value) {
  static float SLOPE = -0.036;
  static float INTERCEPT = 16.56;

  return ( ( (float)value * SLOPE ) + INTERCEPT );
}

