#ifndef __MENU_SCREEN_EXTRA_H__
#define __MENU_SCREEN_EXTRA_H__

#include "menu_screen.h"
#include "graphics_item.h"


template <typename T>
class ProgressBarScreen : public InputScreen<T> {

  // static helper
  static void drawBar(UiContext* context, uint8_t value) {
  
     uint8_t BAR_FRAME_PAD = 2;
       uint8_t PROG_BAR_X = 14;
     uint8_t PROG_BAR_Y = 36;
       uint8_t PROG_BAR_H = context->display->getDisplayHeight() - (PROG_BAR_Y + (3 * BAR_FRAME_PAD) ); //20
       uint8_t PROG_BAR_W = context->display->getDisplayWidth() - (2 * PROG_BAR_X); //100

      uint8_t PROG_FRAME_X = PROG_BAR_X - BAR_FRAME_PAD;
      uint8_t PROG_FRAME_Y = PROG_BAR_Y - BAR_FRAME_PAD;
      uint8_t PROG_FRAME_W = PROG_BAR_W + (2 * BAR_FRAME_PAD);
      uint8_t PROG_FRAME_H = PROG_BAR_H + (2 * BAR_FRAME_PAD);

      uint8_t max_value = 1;
      uint8_t x_scale = PROG_BAR_W/max_value;
      //black out
        context->display->setDrawColor(0);
        // x, y, w, h
        context->display->drawBox( (PROG_BAR_X + value), (PROG_BAR_Y), (100 - value * x_scale), (PROG_BAR_H) ); //maxValue() - value

        //draw bar
        context->display->setDrawColor(1);
        context->display->drawBox(PROG_BAR_X, PROG_BAR_Y, value * x_scale, PROG_BAR_H);
        
        context->display->drawFrame(PROG_FRAME_X, PROG_FRAME_Y, PROG_FRAME_W, PROG_FRAME_H);
  }
  
  
  public:
    ProgressBarScreen(MenuScreen* parent, UiContext* context, const char* title, T min_value = 0, T max_value = 100)
      : InputScreen<T>(parent, context, title, min_value, max_value) {
    }
    ProgressBarScreen(MenuScreen* parent, UiContext* context, const __FlashStringHelper* title, T min_value = 0, T max_value = 100)
      : InputScreen<T>(parent, context, title, min_value, max_value) {
    }
    void draw(UiContext* context) {
      //context->display->firstPage();
      //confirm font
      context->setFont(context->fontLarge());
      //do {
        //context->display->setCursor( (context->display->getDisplayWidth() - MenuScreen::titleWidth(context) ) >> 1, 24);
        //MenuScreen::printTitle(context);
        drawBar(context, InputScreen<T>::value());
        //InputScreen<T>::template printValue<T>(context);
        InputScreen<T>::draw(context);
      //} while ( context->display->nextPage() );
    }
}; // ProgressBarScreen

/*class ToggleBarScreen : public InputScreen {
public:
    ToggleBarScreen(MenuScreen* parent, const char* title)
    : InputScreen(parent, title, 0, 1) {
    }
    ToggleBarScreen(MenuScreen* parent, const __FlashStringHelper* title)
    : InputScreen(parent, title, 0, 1) {
    }
    void draw(U8G2* display) {
        static uint8_t FRAME_PAD = 2;
        static uint8_t TOGGLE_X = display->getDisplayWidth()/4;
        static uint8_t TOGGLE_Y = display->getDisplayHeight()/2 - FRAME_PAD;
        static uint8_t TOGGLE_H = display->getDisplayHeight()/2;//32
        static uint8_t TOGGLE_W = display->getDisplayWidth()/2;//64
        
        uint8_t SWITCH_X = TOGGLE_X + 1 + (value() > 0 ?  32 : 0);
        uint8_t SWITCH_Y = TOGGLE_Y + 1;
        uint8_t SWITCH_W = TOGGLE_W/2 - 4;
        uint8_t SWITCH_H = TOGGLE_H-2;
        
        display->firstPage();
        //confirm font
        display->setFont(u8g2_font_helvB12_te);
        do {
            //black out
            //display->setDrawColor(0);
            
            display->setDrawColor(1);
            if(value() > 0) {
                display->drawBox(TOGGLE_X, TOGGLE_Y, TOGGLE_W, TOGGLE_H);
            }
            else {
                display->drawFrame(TOGGLE_X, TOGGLE_Y, TOGGLE_W, TOGGLE_H);
            }
            
            display->drawBox(SWITCH_X,SWITCH_Y,SWITCH_W,SWITCH_H);
            
            display->setCursor(SWITCH_X,SWITCH_Y);
            if(value() > 0) {
                display->print("ON");
            }
            else {
                display->print("OFF");
            }
            // if value == 1, highlight right and black left
            // x, y, w, h
            display->drawBox(TOGGLE_X + value(), TOGGLE_Y, maxValue() - value(), TOGGLE_H);
            //draw selection box
            display->setDrawColor(1);
            display->drawBox(TOGGLE_X, TOGGLE_Y, value(), TOGGLE_H);
            
            //print title
            MenuScreen::printTitle(display, 24);
        } while ( display->nextPage() );
    }
};*/


class SleepScreen : public MenuScreen {
  public:
    SleepScreen(MenuScreen* parent, UiContext* context, const char* title)
      : MenuScreen(parent, context, title) {
    }
    SleepScreen(MenuScreen* parent, UiContext* context, const __FlashStringHelper* title)
      : MenuScreen(parent, context, title) {
    }
    void draw(UiContext* context) {
      context->display->setPowerSave(true);
    }
};

class AboutScreen : public TextScreen {
  public:
    AboutScreen(MenuScreen* parent, UiContext* context, const char* title)
      : TextScreen(parent, context, title, "Lab Things v"LT_VERSION_STRING){
        //dtostrf(LT_VERSION, 2, 2, &_text[14]);
        //sprintf(&_text[12], "%.2f", LT_VERSION); //saves a few bytes of prog mem
        //snprintf(&_text[12], 5, "%.2f", LT_VERSION);
    }
};

typedef void (*IrCodeCallback) (const int arg1, const uint32_t arg2);
class IrProgScreen : public TextScreen {
    int _ir_addr = 0;
    IrCodeCallback _irCodeCallback = nullptr;
  public:
    IrProgScreen(MenuScreen* parent, UiContext* context, const char* title, const char* text, int ir_addr)
      : TextScreen(parent, context, title, text), _ir_addr(ir_addr) {
    }
    IrProgScreen(MenuScreen* parent, UiContext* context, const __FlashStringHelper* title, const __FlashStringHelper* text, int ir_addr)
      : TextScreen(parent, context, title, text), _ir_addr(ir_addr) {
    }
    void setIrCodeChangedCallback(IrCodeCallback c) {
      _irCodeCallback = c;
    }
    void setValue(uint32_t value) {
      if (_irCodeCallback != nullptr) {
        (*_irCodeCallback)(_ir_addr, value);
      }
      TextScreen::enter();
    }
};

#endif // End __MENU_SCREEN_EXTRA_H__ include guard
