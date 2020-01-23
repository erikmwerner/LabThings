/* SmartOven
 * Designed for ESP32, RTD, OLED, and rotary encoder with pushbutton
 * 
 *  Copyright (c) 2019 Erik Werner erikmwerner@gmail.com
 *  All rights reserved.
*/
 
#include <LabThings.h>
#include "rtd.h"
#include "pid.h"
#include "menus.h"
#include "io.h"

double heaterStatus;
uint8_t days = 0;
uint8_t hours = 0;
uint8_t mins = 0;
uint8_t secs = 0;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

//DeviceManager<7> device_manager; // device manager to run on core1 (arduino default)

LT_MQTT io(0);
AdafruitIO_Feed *set_temperature_feed = io.feed("set-temperature");
AdafruitIO_Feed *oven_temperature_feed = io.feed("oven-temperature");
AdafruitIO_Feed *probe_temperature_feed = io.feed("probe-temperature");

LT_Encoder encoder(0, 39, 36, true);
LT_DebouncedButton button(0, 34, true);
LT_RTD rtd(0, 17, 13, 12, 14);

U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 14, /* data=*/13 , /* cs=*/ 16, /* dc=*/ 27, /* reset=*/ 4);

UiContext context(&u8g2);
Ui ui(0, context);

LT_Timer uptimeTimer(0, 1000000); // 1000 ms
LT_DigitalOutput heater(0, 21); //21 is heater, 22 is heat EN

MainMenu<9> screen_main(NULL, &context, "Main Menu");

MenuScreen screen_dashboard(&screen_main, &context, "Dashboard");
MenuBar bar(&screen_dashboard, 0, 0, 128, 8);
TextItem<32> textStatus(&bar, context.fontSmall(), "Heat OFF", AlignLeft | AlignVCenter);
TextItem<32> textConnection(&bar, context.fontSmall(), "No IP", AlignRight | AlignVCenter);
TextItem<32> textUptime(&screen_dashboard, context.fontSmall(), "Uptime: 00d:00h:00m:00s", AlignHCenter | AlignBottom);
NumberItem<float> temp_meas(&screen_dashboard, context.fontLarge(), 22, 22, 16, 16, 0 , 300);
NumberItem<float> temp_set(&screen_dashboard, context.fontLarge(), 84, 22, 16, 16, 0, 300);
TextItem<16> textMeas(&screen_dashboard, context.fontMedium(), "Meas (ºC)");
TextItem<16> textSet(&screen_dashboard, context.fontMedium(), "Set (ºC)");

MenuScreen screen_graph(&screen_main, &context, "Graph");

GraphItem<float, 512> graph(&screen_graph, "Graph 1", "Time", "Temp (ºC)", 8, 8, 120, 48);
//graph.addDataSet

InputScreen<int> screen_set(&screen_main, &context, "Set Temp (ºC)", 0, 300);

Menu<8> screen_profile(&screen_main, &context, "Set Profile");
//MainMenu<8> screen_profile(&screen_main, &context, "Set Profile");
TextItem<32>text_profile(&screen_dashboard, context.fontSmall(), "Running program...", AlignHCenter);
//MenuScreen screen_step1(&screen_profile, &context, "Step 1");
//MenuScreen screen_step2(&screen_profile, &context, "Step 2");
//MenuScreen screen_step3(&screen_profile, &context, "Step 3");
//MenuScreen screen_step4(&screen_profile, &context, "Step 4");

MenuScreen screen_network(&screen_main, &context, "Network");
TextItem<64>textIP(&screen_network, context.fontMedium(), "IP: Connecting...");
TextItem<64>textMAC(&screen_network, context.fontMedium(), "MAC: ");

MenuScreen screen_about(&screen_main, &context, "About");
TextItem<64>textAbout(&screen_about, context.fontMedium(), "About");

InputScreen<double> screen_setP(&screen_main, &context, "Set PID P", 0, 10000.0);
InputScreen<double> screen_setI(&screen_main, &context, "Set PID I", 0, 10000.0);
InputScreen<double> screen_setD(&screen_main, &context, "Set PID D", 0, 10000.0);

void IRAM_ATTR isrRotate() {
  portENTER_CRITICAL_ISR(&mux);
  encoder.handleInterrupt();
  portEXIT_CRITICAL_ISR(&mux);
}

void IRAM_ATTR isrButton() {
  portENTER_CRITICAL_ISR(&mux);
  button.handleInterrupt();
  portEXIT_CRITICAL_ISR(&mux);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Starting Smart Oven...");
  screen_main.addMenu(&screen_dashboard);
  screen_main.addMenu(&screen_graph);
  screen_main.addMenu(&screen_set);
  //screen_main.appendMenu(&screen_profile);
  screen_main.addMenu(&screen_network);
  screen_main.addMenu(&screen_about);
  
  screen_main.addMenu(&screen_setP);
  screen_main.addMenu(&screen_setI);
  screen_main.addMenu(&screen_setD);

  bar.addChild(&textStatus);
  bar.addChild(&textConnection);

  screen_dashboard.addChild(&temp_meas);
  screen_dashboard.addChild(&temp_set);
  screen_dashboard.addChild(&textMeas);
  screen_dashboard.addChild(&textSet);
  screen_dashboard.addChild(&textUptime);

  //screen_profile.appendMenu(&screen_step1);
  //screen_profile.appendMenu(&screen_step2);
  //screen_profile.appendMenu(&screen_step3);
  //screen_profile.appendMenu(&screen_step4);
  //screen_profile.appendMenu(&screen_main);

  // add the menu bar after the rest of the children
  screen_dashboard.addChild(&bar);

  text_profile.setPos(0, 56);
  text_profile.setVisible(false);
  screen_dashboard.addChild(&text_profile);
   screen_about.addChild(&textAbout);
  textAbout.setPos(12, 24);
  char aboutBuf[64] = {0};
  snprintf( aboutBuf, 64, "Lab Things v %02F", LT_VERSION );
  textAbout.setText(aboutBuf);

  screen_network.addChild(&textIP);
  screen_network.addChild(&textMAC);
  textIP.setPos(2, 24);
  textMAC.setPos(2, 48);

  graph.setGraphType(2);

  screen_graph.addChild(&graph);

  // add the menu bar after the rest of the children
  screen_graph.addChild(&bar);

  temp_meas.setValue(measTemp);
  temp_set.setValue(setTemp);
  screen_set.setValue(setTemp);
  textMeas.setPos(14, 48);
  textSet.setPos(78, 48);

  screen_set.setValueChangedCallback(onSetTempChanged);
  screen_setP.setValueChangedCallback(onPChanged);
  screen_setI.setValueChangedCallback(onIChanged);
  screen_setD.setValueChangedCallback(onDChanged);

  screen_set.setScreenEnteredCallback(onSetTempEntered);

  screen_setP.setValue(Kp);
  screen_setI.setValue(Ki);
  screen_setD.setValue(Kd);

  ui.setScreenSaverTimeout(120000000); // start screensaver after 120 s
  ui.setScreenSaverEnabled(true);
  ui.setHomeScreenTimeout(0); // prevent returning to home
  ui.setCurrentScreen(&screen_dashboard);
  ui.setHomeScreen(&screen_dashboard);

  /* / Ui Setup */
  encoder.setDebounceInterval(5000); // 5 ms
  encoder.setValueChangedCallback(onEncoderValueChanged);
  button.setButtonReleasedCallback(onButtonReleased);

  uptimeTimer.setCallback(onUptimeTimerTimeout);

  rtd.setPolling(true);
  rtd.setPollingInterval(pidSampleTime * 1000L); // 1000 ms
  rtd.setNewDataCallback(onNewTemperatureEvent);

  heater.setValue(LOW);
  //tell the PID to range between 0 and the full window size
  myPID.SetOutputLimits(0, pidWindowSize);
  //turn the PID on
  myPID.SetMode(AUTOMATIC);

  myPID.SetSampleTime(pidSampleTime); //500ms

  set_temperature_feed->onMessage(handleMessage);
  io.setConnectionStatusChangedCallback(onConnectionStatusChanged);

  screen_profile.begin();
  
  encoder.begin();
  button.begin();
  rtd.begin();
  uptimeTimer.begin();
  heater.begin();
  ui.begin();
  io.begin();
  
  attachInterrupt(digitalPinToInterrupt(39), isrRotate, FALLING);
  attachInterrupt(digitalPinToInterrupt(36), isrRotate, FALLING);
  attachInterrupt(digitalPinToInterrupt(34), isrButton, FALLING);
  
  //attachInterrupt(39, isrRotate, FALLING); // encoder A
  //attachInterrupt(36, isrRotate, FALLING); // encoder B
  //attachInterrupt(34, isrButton, CHANGE); // encoder button
}

void loop() {
  // put your main code here, to run repeatedly:
  //device_manager.update();
  LT_current_time_us = micros();
  static uint8_t limit = 0;
  portENTER_CRITICAL(&mux);
  encoder.update();
  button.update();
  portEXIT_CRITICAL(&mux);
  
  rtd.update();
  uptimeTimer.update();
  heater.update();
  if(limit % 64 == 0) { // every 64 loops
    io.update();
  }
  limit++;
  ui.update();
  checkHeater();
}

void handleMessage(AdafruitIO_Data *data) {
  Serial.print("received data (string):<- ");
  Serial.print(data->value());
  Serial.print(" to int:");

  int reading = data->toInt();
  Serial.println(reading);
  Serial.print("set temp was:");
  Serial.println(setTemp);
  if (setTemp != reading) {
    setTemp = reading;
    temp_set.setValue(setTemp);
    // update input screen, but do not trigger the callback
    screen_set.setValue(reading, false);
  }
}

uint8_t graph_limiter = 0;

void onNewTemperatureEvent() {
  measTemp = round(rtd.getTemperature() * 100.0) / 100.0; // round to nearest .01
  //measTemp = 80;
  
  if (graph_limiter %4 == 0) { //only log every 4th point (4 s)
    graph.addDataPoint( (LT_current_time_us / 1000) , measTemp);
  }
  if(graph_limiter %8 == 0) { // send every 8th point to io (8 s)
    oven_temperature_feed->save(measTemp);
    set_temperature_feed->save(setTemp);
    //probe_temperature_feed->save(measTemp);
  }

  graph_limiter++;
  temp_meas.setValue(measTemp);
  // https://electronics.stackexchange.com/questions/311507/please-explain-in-laymans-terms-how-a-pid-accounts-for-inertia-in-temperature-c
  // https://electronics.stackexchange.com/questions/163907/how-to-interpret-pid-output-in-a-slow-response-system?rq=1
  // https://electronics.stackexchange.com/questions/50049/how-to-implement-a-self-tuning-pid-like-controller?rq=1

  myPID.Compute();
  /*Serial.print("PID meas temp:");
    Serial.print(measTemp);
    Serial.print(" PID out: ");
    Serial.print(pidOutput);
    Serial.print(" PID set temp: ");
    Serial.println(setTemp);*/
}

void checkHeater() {
  double prev_state = heaterStatus;
  if (millis() - pidWindowStartTime > pidWindowSize) {
    // shift the window
    pidWindowStartTime += pidWindowSize;
  }
  if (pidOutput < millis() - pidWindowStartTime) {
    heaterStatus = pidOutput; //0
    heater.setValue(LOW);
  }
  else {
    heaterStatus = pidOutput;
    heater.setValue(HIGH); //1
  }

  if (heaterStatus != prev_state) {
    onHeaterStatusChanged(heaterStatus);
  }
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

void onGraphTypeChanged(int value) {
  graph.setGraphType(value);
}

void onSwitchChanged(int value) {
  heater.setValue(value);
}

void onSetTempChanged() {
  int value = screen_set.value();
  setTemp = value;
  temp_set.setValue(setTemp);
}

void onSetTempEntered(void* ptr) {
  //yield();
  //set_temperature_feed->save(setTemp);
}

void onPChanged() {
  onPIDTuningsChanged(screen_setP.value(), Ki, Kd);
}

void onIChanged() {
  onPIDTuningsChanged(Kp, screen_setI.value(), Kd);
}

void onDChanged() {
  onPIDTuningsChanged(Kp, Ki, screen_setD.value());
}

void onPIDTuningsChanged(float kp, float ki, float kd) {
  Kp = kp,
  Ki = ki;
  Kd = kd;

  myPID.SetTunings(Kp, Ki, Kd);
}

void onUptimeTimerTimeout() {
  // update the system time
  secs++;
  if (secs > 59) {
    secs = 0;
    mins++;
  }
  if (mins > 59) {
    mins = 0;
    hours++;
  }
  if (hours > 23) {
    days ++;
    hours = 0;
  }
  // update the time label
  char buf[32];
  snprintf(buf, 32, "Uptime: %02ud:%02uh:%02um:%02us", days, hours, mins, secs);
  textUptime.setText(buf);
}

void onConnectionStatusChanged(String ip, String mac) {
  char ip_buf[24];
  ip.toCharArray(ip_buf, 24);
  textConnection.setText(ip_buf);
  textIP.setText(ip_buf);
  char mac_buf[24];
  mac.toCharArray(mac_buf, 24);
  textMAC.setText(mac_buf);
}

void onHeaterStatusChanged(double value) {
  heaterStatus = value;

  char buf[32];
  snprintf(buf, 32, "Heat: %.1f%% ", ( pidOutput / (double)pidWindowSize * 100.0) );
  textStatus.setText(buf);
}

void updateCloudData() {
  // only update data if it has changed or if 1 minutes (60000 ms) have elapsed
}
