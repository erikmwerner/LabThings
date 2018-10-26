
/*!
* Lab Things
*/

#ifndef __LABTHINGS__H__
#define __LABTHINGS__H__

// define fast digital to include (experimental) direct port writing
//#define FAST_DIGITAL
#ifdef FAST_DIGITAL
#include "utilities/fast_digital.h"
#endif

//#define DEBUG_PRINT

uint32_t LT_current_time_us;
const static float LT_VERSION = 0.14;
#define LT_VERSION_STRING "0.14"

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