/*!
  Speaker Remote Version 0.9

  Digital volume controller for Klipsch Promedia 2.1 speakers

  1306 128x64 OLED
  Rotary encoder with pushbutton
  IR receiver
  MCP 4261 digital potentiometer

  Copyright (c) 2018, Erik Werner
  erikmwerner@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

  Redistributions of source code must retain the above copyright notice, this list
  of conditions and the following disclaimer.

  Redistributions in binary form must reproduce the above copyright notice, this
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

// Include the objects we need to use
// To manage inputs and outputs of 4 devices
#include <LabThings.h>
Device_Manager<4> device_manager;

// Our MCU is connected to:
// an encoder on pins 3 and 2 with pushbutton on pin 5
LT_Encoder encoder(device_manager.registerDevice(), 3, 2, true);
LT_DebouncedButton button(device_manager.registerDevice(), 5, true);

// an IR sensor on pin 4
#include <IRremote.h>

IRrecv irrecv(4);
decode_results results;

// two digital potentiometers with CS pins 6 and 7
#include <SPI.h>
#include <mcp4261.h>

MCP4261 volumePot = MCP4261(7);
MCP4261 bassPot = MCP4261(6);

// we want to save values to EEPROM
#include <EEPROMex.h>
#include <EEPROMVar.h>
// make sure debug mode is turned off
#ifdef _EEPROMEX_DEBUG
  #undef _EEPROMEX_DEBUG
#endif

LT_Timer save_timer(device_manager.registerDevice(), 10000000); // 10 seconds

// EEPROM address scheme
#define ADDR_Volume 0 //0, 1
#define ADDR_Bass 2 // 2, 3
#define ADDR_Balance 4 // 4, 5
#define ADDR_ir_f 6 //6, 7, 8, 9
#define ADDR_ir_r 10 //10, 11, 12, 13
#define ADDR_ir_s 14 //14, 15, 16, 17

// an OLED display
//1306 on hardware SPI
U8G2_SSD1306_128X64_NONAME_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);

// running a menu
UiContext context(&u8g2);
Ui ui(device_manager.registerDevice(), context);

// select screen has 6 items
#define PGMSTR(x) (__FlashStringHelper*)(x)
/*
const char menuTitle[] PROGMEM = "Menu";
const char volumeTitle[] PROGMEM = "Volume";
const char bassTitle[] PROGMEM = "Bass";
const char balanceTitle[] PROGMEM = "Balance";
const char irTitle[] PROGMEM = "Program IR";
const char irText1[] PROGMEM = "Press forward (up)...";
const char irText2[] PROGMEM = "Press back (down)....";
const char irText3[] PROGMEM = "Press menu (select)....";
const char sleepTitle[] PROGMEM = "Sleep";
const char aboutTitle[] PROGMEM = "About";
const char aboutText[] PROGMEM = "KPM Volume Pod V0.9";

ListScreen<6> screen_main(NULL, PGMSTR(menuTitle));
ProgressBarScreen screen_volume(&screen_main, PGMSTR(volumeTitle), 0, 100);
ProgressBarScreen screen_bass(&screen_main, PGMSTR(bassTitle), 0, 100 );
ProgressBarScreen screen_balance(&screen_main, PGMSTR(balanceTitle), 0, 100);

IrProgScreen screen_ir1(&screen_main, PGMSTR(irTitle), PGMSTR(irText1), ADDR_ir_f);
IrProgScreen screen_ir2(&screen_main, PGMSTR(irTitle), PGMSTR(irText2), ADDR_ir_r);
IrProgScreen screen_ir3(&screen_main, PGMSTR(irTitle), PGMSTR(irText3), ADDR_ir_s);

SleepScreen screen_sleep(&screen_main, PGMSTR(sleepTitle));
TextScreen screen_about(&screen_main, PGMSTR(aboutTitle), PGMSTR(aboutText));
*/
 //Text in RAM is slightly faster
MainMenu<6> screen_main(NULL, &context, "Menu");
ProgressBarScreen<int> screen_volume(&screen_main, &context, "Volume", 0, 100);
ProgressBarScreen<int> screen_bass(&screen_main, &context, "Bass", 0, 100 );
ProgressBarScreen<int> screen_balance(&screen_main, &context, "Balance", 0, 100);

IrProgScreen screen_ir1(&screen_main, &context, "Program IR", "Press forward (up)...", ADDR_ir_f);
IrProgScreen screen_ir2(&screen_main, &context, "Program IR", "Press back (down)....", ADDR_ir_r);
IrProgScreen screen_ir3(&screen_main, &context, "Program IR", "Press menu (select)....", ADDR_ir_s);

SleepScreen screen_sleep(&screen_main, &context, "Sleep");
TextScreen screen_about(&screen_main, &context, "About", "KPM Volume Pod V0.9");


// these will be saved and stored in non-vilatile EEPROM
int8_t volume = EEPROM.readInt(ADDR_Volume); // 2
int8_t bass = EEPROM.readInt(ADDR_Bass); //10
int8_t balance = EEPROM.readInt(ADDR_Balance); //50
uint32_t ir_forward = EEPROM.readLong(ADDR_ir_f);
uint32_t ir_reverse = EEPROM.readLong(ADDR_ir_r);
uint32_t ir_select = EEPROM.readLong(ADDR_ir_s);

void setup() {
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  SPI.begin();

  volumePot.initialize();
  writePots();
  
  // IR receiver setup
  irrecv.enableIRIn();

  //Serial.begin(115200);

  device_manager.attachDevice(&encoder);
  device_manager.attachDevice(&button);
  device_manager.attachDevice(&save_timer);

  screen_volume.setValue(volume);
  screen_bass.setValue(bass);
  screen_balance.setValue(balance);
  screen_main.addMenu(&screen_volume);
  screen_main.addMenu(&screen_bass);
  screen_main.addMenu(&screen_balance);
  screen_main.addMenu(&screen_ir1);
  screen_main.addMenu(&screen_sleep);
  screen_main.addMenu(&screen_about);

  screen_ir1.setNextPage(&screen_ir2);
  screen_ir2.setNextPage(&screen_ir3);

  ui.setCurrentScreen(&screen_main);
  ui.setHomeScreen(&screen_volume);

  device_manager.attachDevice(&ui);

  encoder.setValueChangedCallback(onEncoderValueChanged);
  button.setButtonReleasedCallback(onButtonReleased);

  save_timer.setSingleShot(true);
  save_timer.setCallback(onEEPROMUpdateTimeout);

  screen_volume.setValueChangedCallback(onVolumeChanged);
  screen_bass.setValueChangedCallback(onBassChanged);
  screen_balance.setValueChangedCallback(onBalanceChanged);

  screen_ir1.setIrCodeChangedCallback(onIRCodeChanged);
  screen_ir2.setIrCodeChangedCallback(onIRCodeChanged);
  screen_ir3.setIrCodeChangedCallback(onIRCodeChanged);

  EEPROM.setMemPool(0, EEPROMSizeUno);
  // Set maximum allowed writes to maxAllowedWrites.
  // More writes will only give errors when _EEPROMEX_DEBUG is set
  EEPROM.setMaxAllowedWrites(10);
}

void loop() {
  checkIR();
  // update all devices as often as possible
  device_manager.loop();
}

void checkIR() {
  if (irrecv.decode(&results)) {
    //Serial.print("got ir: ");
    //Serial.println(results.value);
    if (ui.currentScreen() == &screen_ir1) {
      screen_ir1.setValue(results.value);
    }
    else if (ui.currentScreen() == &screen_ir2) {
      screen_ir2.setValue(results.value);
    }
    else if (ui.currentScreen() == &screen_ir3) {
      screen_ir3.setValue(results.value);
    }
    else if (results.value == ir_forward) {
      onSleepActivated(false);
      // invert select menu controls for ir remote
      (ui.currentScreen() == &screen_main) ? ui.decrement() : ui.increment();
    }
    else if (results.value == ir_reverse) {
      onSleepActivated(false);
      // invert select menu controls for ir remote
      (ui.currentScreen() == &screen_main) ? ui.increment() : ui.decrement();
    }
    else if (results.value == ir_select) {
      onSleepActivated(false);
      ui.enter();
    }
    irrecv.resume(); // Receive the next value
  }
}

void writePots() {
  //uint8_t adjusted = map(value, 0, 100, 0, 255);

  int8_t wiper_l = volume;
  int8_t wiper_r = volume;

  if (balance > 50) {
    wiper_l = volume - (balance - 50) / 100.0 * volume;
  }
  else if (balance < 50) {
    wiper_r = volume - (50 - balance) / 100.0 * volume;
  }

  //Serial.print("L pot:");
  //Serial.print(wiper_l);
  //Serial.print(" R pot:");
  //Serial.println(wiper_r);

  volumePot.setWiper0(wiper_l);
  volumePot.setWiper1(wiper_r);
  //bassPot.setWiper0(bass);
  //bassPot.setWiper1(bass);
}

// Define callback functions for devices
//----------------------------------------------------------------------------//
void onEncoderValueChanged(int value) {
  static int last_value;
  //update menu
  onSleepActivated(false);
  if (value > last_value) {
    ui.increment();
  }
  else {
    ui.decrement();
  }
  last_value = value;
}

void onButtonReleased() {
  onSleepActivated(false);
  //update menu
  ui.enter();
}

void onVolumeChanged() {
  //volume = value;
  int value = screen_volume.value();
  writePots();
  save_timer.start();
  //saver.setDirty(true);
}

void onBassChanged() {
  //bass = value;
  int value = screen_bass.value();
  writePots();
  save_timer.start();
  //saver.setDirty(true);
}

void onBalanceChanged() {
  //balance = value;
  int value = screen_balance.value();
  writePots();
  save_timer.start();
  //saver.setDirty(true);
}

void onIRCodeChanged(int addr, uint32_t value) {
  //Serial.print("IR changed addr:");
  //Serial.print(addr);
  //Serial.print(" value:");
  //Serial.println(value);
  if (addr == ADDR_ir_f) {
    ir_forward = value;
  }
  else if (addr == ADDR_ir_r) {
    ir_reverse = value;
  }
  else if (addr == ADDR_ir_s) {
    ir_select = value;
    // schedule an EEPROM update
    save_timer.start();
    //saver.setDirty(true);
  }
  // advance to the next screen
  ui.enter();
}

void onSleepActivated(bool isSleeping) {
  ui.setSleeping(true);
}

void onEEPROMUpdateTimeout() {
  EEPROM.updateLong(ADDR_ir_f, ir_forward);
  EEPROM.updateLong(ADDR_ir_r, ir_reverse);
  EEPROM.updateLong(ADDR_ir_s, ir_select);

  EEPROM.updateInt(ADDR_Volume, volume);
  EEPROM.updateInt(ADDR_Bass, bass);
  EEPROM.updateInt(ADDR_Balance, balance);
}