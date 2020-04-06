#ifndef __INPUT_SCREEN_H__
#define __INPUT_SCREEN_H__

#include "menu_screen.h"
#include "graphics_item.h"
#include "ui_context.h"

/*
NumberScreen provides a menu that 
displays a single value
*/

template <typename T>
class NumberScreen : public MenuScreen {

  private:
    NumberItem<T> _number;
    const char* _suffix;

  public:
  NumberScreen(MenuScreen* parent, UiContext* context, const char* title,  
  T min_value = 0, T max_value = 100, const char* suffix = nullptr)
  : MenuScreen(parent, context, title),_number(this, context->getFontLarge()) {
    uint8_t w = context->display->getDisplayWidth();
    uint8_t h = context->display->getMaxCharHeight();
    uint8_t y = ( context->display->getDisplayHeight() ) >> 1; // half the display height
    uint8_t str_w = 0;
    char buffer[16];
    if(snprintf(buffer, 16, "%5d", max_value) > 0) {
      str_w += context->display->getStrWidth(buffer);
    }
    else {
      str_w += context->display->getMaxCharWidth() * 3;
    }
    if(suffix) {
      str_w += context->display->getStrWidth(suffix);
    }
    uint8_t x = (w - str_w - 32) >> 1 ;
    _number =  NumberItem<T>(this, context->getFontLarge(), x, y, w, h, min_value, max_value);
    _suffix = suffix;
    //_suffix = TextItem<8>(this, context->getFontLarge(), suffix);
    
    
    //if(suffix != nullptr) {
       //_suffix = TextItem<8>(this, context->getFontLarge(), suffix);
   //   addChild(&_suffix);
    //}
    addChild(&_number); 
    }

    void setPrecision(const uint8_t decimals) {
      _number.setPrecision(decimals);
    }

    void setValueChangedCallback(Callback c) {
      _number.setValueChangedCallback(c);
    }

    const T value() {
      return _number.value();
    }
    
    void setValue(const T value, const bool respond = true) {
     _number.setValue(value, respond);
    }
    
    void draw(UiContext* context) {
      //context->display->firstPage();
      //do {
        // print centered title with top of character at the top pixel of the screen
        // context->display->setCursor( (context->display->getDisplayWidth() - titleWidth(context) ) >> 1, context->display->getAscent());
        uint8_t x = (context->display->getDisplayWidth() - titleWidth(context) ) >> 1;
        uint8_t y = context->display->getAscent() + context->getMargin();
        MenuScreen::printTitle(context, x, y);
        //print value
        _number.draw(context);
        if(_suffix) {
          context->display->print(_suffix);
        }
        //_number.template printValue<T>(context);
       // } while ( context->display->nextPage() );
    }
}; // InputScreen

/*
InputScreen provides a menu that 
accepts a single value as an input
*/

template <typename T>
class InputScreen : public NumberScreen<T> {
  T _value_step = 1;
    
  public:
    InputScreen(MenuScreen* parent, UiContext* context, const char* title, 
    T min_value = 0, T max_value = 100, T value_step = 1, char* suffix = nullptr)
      : NumberScreen<T>(parent, context, title, min_value, max_value, suffix), _value_step(value_step) {
    }
    /*InputScreen(MenuScreen* parent, UiContext* context, const char* title, T min_value = 0, T max_value = 100, 
    const char* prefix = NULL, const char* suffix = NULL)
      : NumberScreen<T>(parent, context, title, min_value, max_value, prefix, suffix) {
    }*/
    
    void increment() {
      this->setValue(this->value() + _value_step);
      //_number.increment();
    }

    void decrement() {
      this->setValue(this->value() - _value_step);
      //_number.decrement();
    }

    void adjust(const T value) {
      this->setValue(this->value() + value);
      //_number.increment();
    }

}; // InputScreen

#endif // End __INPUT_SCREEN_H__ include guard