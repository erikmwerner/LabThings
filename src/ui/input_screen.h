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
    
  public:
  NumberScreen(MenuScreen* parent, UiContext* context, const char* title,  T min_value = 0, T max_value = 100)
  : MenuScreen(parent, context, title),_number(this, context->getFontLarge()) {
    uint8_t h = context->display->getDisplayHeight() >> 1;
    uint8_t x = (context->display->getDisplayWidth() - h) >> 1;
    _number =  NumberItem<T>(this, context->getFontLarge(), x, h, h, h, min_value, max_value);
    addChild(&_number); 
    }
    /*NumberScreen(MenuScreen* parent, UiContext* context, const char* title,  T min_value = 0, T max_value = 100, 
    const char* prefix = NULL, const char* suffix = NULL)
      : MenuScreen(parent, context, title), 
      _number( NumberItem<T>(this, context->fontLarge(), 56, 54, 0, 0, min_value, max_value) ) {
      addItem(&_number); // TK I think it adds to the child list. Change to addChild()?
      if(prefix != NULL) {
        TextItem<4>text(this, context->fontMedium(), "pre");
        addItem(&text);
      }
      if(suffix != NULL) {
        TextItem<64>text(this, context->fontMedium(), "pos");
        addItem(&text);
      }
    }///WTF is this?
    */
    
    virtual void increment() {
      // do nothing
    }

   virtual void decrement() {
     // do nothing
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
        context->display->setCursor( (context->display->getDisplayWidth() - titleWidth(context) ) >> 1, context->display->getAscent());
        MenuScreen::printTitle(context);
        //print value
        _number.draw(context);
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
    
  public:
    InputScreen(MenuScreen* parent, UiContext* context, const char* title, T min_value = 0, T max_value = 100)
      : NumberScreen<T>(parent, context, title, min_value, max_value) {
    }
    /*InputScreen(MenuScreen* parent, UiContext* context, const char* title, T min_value = 0, T max_value = 100, 
    const char* prefix = NULL, const char* suffix = NULL)
      : NumberScreen<T>(parent, context, title, min_value, max_value, prefix, suffix) {
    }*/
    
    void increment() {
      this->setValue(this->value() + 1);
      //_number.increment();
    }

    void decrement() {
      this->setValue(this->value() + -1);
      //_number.decrement();
    }

}; // InputScreen

#endif // End __INPUT_SCREEN_H__ include guard