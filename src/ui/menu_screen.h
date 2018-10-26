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
class MenuScreen : public GraphicsItem{
    MenuScreen* _parent = NULL;
    const char* _title = NULL;
    const __FlashStringHelper* _flash_title = NULL;
    
  public:
    MenuScreen(MenuScreen* parent, UiContext* context, const char* title)
      : GraphicsItem(NULL, 0, 0, context->display->getDisplayWidth(), context->display->getDisplayHeight() ),
       _parent(parent), _title(title) {
      _flash_title = NULL;
    }
    MenuScreen(MenuScreen* parent, UiContext* context, const __FlashStringHelper* title)
      : GraphicsItem(NULL, 0, 0, context->display->getDisplayWidth(), context->display->getDisplayHeight() ),
       _parent(parent), _flash_title(title) {
      _title = NULL;
    }
    const char* title() { return _title; }
    const __FlashStringHelper* flashTitle() const { return _flash_title; }
    MenuScreen* parent() { return _parent; }

    virtual void increment() {
      setDirty(true);
    }
    virtual void decrement() {
      setDirty(true);
    }
    virtual void setDirty(bool dirty) {
      GraphicsItem::setDirty(dirty);
    }
    virtual MenuScreen* enter() { 
      //if( _parent != NULL) {
      //  _parent->setDirty(true);
     // }
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
    
    void printTitle(UiContext* context) {
      if (_title != NULL) {
        context->display->print(_title);
      }
      else if (_flash_title != NULL) {
        context->display->print(_flash_title);
      }
    }
}; //MenuScreen


#endif // End __UI_SCREEN_H__ include guard
