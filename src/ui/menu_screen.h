/*!
* Ui Screen contains a set of content for the display.
* It is also the base class for pre-defined screens
* SA InputScreen
*/

#ifndef __UI_SCREEN_H__
#define __UI_SCREEN_H__

#include "graphics_item.h"
#include "ui_context.h"

size_t getFlashStringLength(const __FlashStringHelper *ifsh)
{
  PGM_P p = reinterpret_cast<PGM_P>(ifsh);
  size_t n = 0;
  while (1) {
    unsigned char c = pgm_read_byte(p++);
    if (c == 0) break;
    n++;
  }
  return n;
}

//idea: MenuScreen<__FlashStringHelper*>
//template <class T>

/*!
 * @brief the MenuScreen provides a base class for a screen.
 * A screen is a representation of graphics items to be displayed
 * 
 */
class MenuScreen : public GraphicsItem{
    MenuScreen* _parent = nullptr;
    const char* _title = NULL;
    const int _icon = NULL;
    const __FlashStringHelper* _flash_title = NULL;
    //Callback _screen_entered_callback = nullptr;
    func_ptr _screen_entered_callback = nullptr;
    
  public:
  MenuScreen() : GraphicsItem(nullptr, 0, 0, 0, 0) {}
    MenuScreen(MenuScreen* parent, UiContext* context, const char* title, const int icon = 0)
      : GraphicsItem(nullptr, 0, 0, context->display->getDisplayWidth(), context->display->getDisplayHeight() ),
       _parent(parent), _title(title), _icon(icon) {
      _flash_title = NULL;
    }
    MenuScreen(MenuScreen* parent, UiContext* context, const __FlashStringHelper* title, const int icon = 0)
      : GraphicsItem(nullptr, 0, 0, context->display->getDisplayWidth(), context->display->getDisplayHeight() ),
       _parent(parent), _flash_title(title), _icon(icon) {
      _title = NULL;
    }
    const char* title() { return _title; }
    const __FlashStringHelper* flashTitle() const { return _flash_title; }
    MenuScreen* parent() { return _parent; }

    /**
     * @brief called by the ui when the user inputs "up"
     * The base class sets a flag and gets redrawn.
     * Reimplement this function in subclasses.
     
    virtual void increment() {
      setDirty(true);
    }
    
     * @brief called by the ui when the user inputs "down"
     * The base class sets a flag and gets redrawn.
     * Reimplement this function in subclasses.
     
    virtual void decrement() {
      setDirty(true);
    }
    */

    //
    virtual void adjust(const int8_t value) {
      setDirty(true);
    }

    virtual void setDirty(bool dirty) {
      GraphicsItem::setDirty(dirty);
    }

    void setScreenEnteredCallback(func_ptr c) {
      _screen_entered_callback = c;
    }
    
    virtual MenuScreen* enter() const { 
      if(_screen_entered_callback != nullptr) {
        (*_screen_entered_callback)(nullptr);
      }
      return _parent; 
    }
    
    virtual void draw(UiContext* context) {
        GraphicsItem::draw(context);
    }
    
    uint8_t titleWidth(UiContext* context) {
       if (_title != NULL) {
         return context->display->getStrWidth(_title);
       }
       else if (_flash_title != NULL) {
         char buffer[20] = {0};
         strcpy_P(buffer,(char*)_flash_title);
         return context->display->getStrWidth(buffer);
       }
    }
    
    void printTitle(UiContext* context, const uint8_t x, const uint8_t y, const bool with_icon = false) {
      
      if(with_icon && _icon != 0) {
        const uint8_t* last_font = context->getCurrentFont();
        context->setCurrentFont(context->getFontSymbol());
        context->display->drawGlyph(x - 1, y + 1, _icon);
        context->display->setCursor(x + context->display->getMaxCharWidth(), y);
        context->setCurrentFont(last_font);
      }
      else {
        context->display->setCursor(x, y);
      }
      // print text
      if (_title != NULL) {
        context->display->print(_title);
      }
      else if (_flash_title != NULL) {
        context->display->print(_flash_title);
      }
    }
}; //MenuScreen


#endif // End __UI_SCREEN_H__ include guard
