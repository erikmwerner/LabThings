
/*!
* Lab Things: an Arduino library for creating IoT instruments for the lab.
*/

#ifndef __LABTHINGS__H__
#define __LABTHINGS__H__

// define fast digital i/o to include (experimental) direct port writing
//#define FAST_DIGITAL
#ifdef FAST_DIGITAL
#include "utilities/fast_digital.h"
#endif

 #if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

//macro to allow debugging via Serial1 to be switched on or off
//comment out DEBUG_PRINT to turn off
//#define DEBUG_PRINT
#ifdef DEBUG_PRINT
  #define DPRINTLN(a) (Serial1.println(a))
  #define DPRINT(a) (Serial1.print(a))
  #define DPRINTF(a) (Serial1.print(F(a)))
  #define DPRINTLNF(a) (Serial1.println(F(a)))
  #define DEBUG_BAUD 38400 //9600 // use 9600 for soft serial echo, 115k for bluetooth
#else
  #define DPRINTLN(a) 
  #define DPRINT(a) 
  #define DPRINTF(a)
  #define DPRINTLNF(a)
#endif

uint32_t LT_current_time_us;
const static float LT_VERSION = 0.16;
#define LT_VERSION_STRING "0.16"

#include "devices/device.h"
#include "devices/sensor.h"
#include "devices/analog_output.h"
#include "devices/analog_sensor.h"
#include "devices/debounced_button.h"
#include "devices/device_manager.h"
#include "devices/digital_output.h"
#include "devices/digital_sensor.h"
#include "devices/encoder.h"
#include "devices/stepper.h"

//requires U8G2
#include "ui/ui.h"
#include "ui/menu_screen.h"
#include "ui/main_menu.h"
#include "ui/input_screen.h"
#include "ui/text_screen.h"
#include "ui/graphics_item.h"
#include "ui/graph_item.h"
#include "ui/menu_screen_extra.h"

#include "messengers/commands.h"
#include "messengers/ascii_serial.h"
#include "messengers/binary_serial.h"
#include "messengers/message_handler.h"
#include "messengers/process_manager.h"

#include "utilities/noise_maker.h"
#include "utilities/streaming.h"
#include "utilities/timer.h"

template <typename T>
T safeMap(T x, T in_min, T in_max, T out_min, T out_max)
{
  // prevent division by zero
  if(in_max == in_min) {
    return out_min;
  }
  else {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }
}

#endif //End __LABTHINGS__H__ include guard