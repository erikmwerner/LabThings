#ifndef __TEXT_SCREEN_H__
#define __TEXT_SCREEN_H__

#include "menu_screen.h"
#include "ui_context.h"

/*template <uint8_t SIZE>
class TextScreen< : public MenuScreen {
    char *_text[SIZE] = NULL;
  public:
    TextScreen(MenuScreen* parent, UiContext* context, const char* title, uint8_t size, const char* text)
      : MenuScreen(parent, context, title), _text(text) {
    }
    MenuScreen* enter() {
      if (_next_page != NULL) {
        return _next_page;
      }
      else {
        return parent();
      }
    }
    void setNextPage(MenuScreen* next_page) {
      _next_page = next_page;
    }

    void setText(char* text, uint8_t size) {

    }
    const char *text() {
      return _text;
    }

    void printText(UiContext* context) {
      if (_text != NULL) {
        context->display->print(_text);
      }
    }

    virtual void draw(UiContext* context) {
      //context->display->firstPage();
      //do {
        //confirm settings
        context->setFont(context->fontLarge());
        context->display->setDrawColor(1);

        //print title
        context->display->setCursor( (context->display->getDisplayWidth() - titleWidth(context) ) >> 1 , 16);
        MenuScreen::printTitle(context);

        //draw seperator
        context->display->drawLine(0, 20, context->display->getDisplayWidth(), 20);

        //print text
        context->setFont(context->fontSmall());
        uint8_t h = 36;
        context->display->setCursor(0, h);
        printText(context);
     // } while ( context->display->nextPage() );
    }
};*/

class TextScreen : public MenuScreen {
    MenuScreen * _next_page = nullptr;
  protected:
    const char *_text = NULL;
    const __FlashStringHelper* _flash_text = NULL;
  public:
    TextScreen(MenuScreen* parent, UiContext* context, const char* title, const char* text)
      : MenuScreen(parent, context, title), _text(text) {
      _flash_text = NULL;
    }
    TextScreen(MenuScreen* parent, UiContext* context, const __FlashStringHelper* title, const __FlashStringHelper* text)
      : MenuScreen(parent, context, title), _flash_text(text) {
      _text = NULL;
    }
    MenuScreen* enter() {
      if (_next_page != nullptr) {
        return _next_page;
      }
      else {
        return parent();
      }
    }
    void setNextPage(MenuScreen* next_page) {
      _next_page = next_page;
    }
    const char *text() {
      return _text;
    }
    const __FlashStringHelper* _flashText() {
      return _flash_text;
    }

    void printText(UiContext* context) {
      if (_text != NULL) {
        context->display->print(_text);
      }
      else {
        context->display->print(_flash_text);
      }
    }

    virtual void draw(UiContext* context) {
        // confirm settings and print title
        context->setCurrentFont(context->getFontLarge());
        // write pixels to on
        context->display->setDrawColor(1);
        // print centered title with top of characters at top pixels of the screen
        //context->display->setCursor( (context->display->getDisplayWidth() - titleWidth(context) ) >> 1 , context->display->getAscent() );
        uint8_t x = (context->display->getDisplayWidth() - titleWidth(context) ) >> 1;
        uint8_t y = context->display->getAscent();
        MenuScreen::printTitle(context, x, y);

        //draw seperator line at the botton of the title
        context->display->drawLine(0, context->display->getMaxCharHeight(), context->display->getDisplayWidth(), context->display->getMaxCharHeight() );


        //TK getDisplayWidth, getStrWidth, getMaxCharWidth
        //print text
        uint8_t h = context->display->getMaxCharHeight() * 2 + context->getMargin();
        context->setCurrentFont(context->getFontSmall());
        context->display->setCursor(context->getMargin(), h);
        printText(context);
    }
};

#endif // End __TEXT_SCREEN_H__ include guard