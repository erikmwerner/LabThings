/*!
* UiContxt holds
*/

#ifndef __UI_CONTEXT_H__
#define __UI_CONTEXT_H__

#include <U8g2lib.h>
#include <U8x8lib.h>

/*!
* The UiContext structure contains a pointer to the 
* display output plus some additional parameters
* related to the ui
*/
struct UiContext {
  //< a pointer to the U8G2 display object. 
  // this object also contains lots of useful data, like width and height
  U8G2* display = nullptr;
  const uint8_t _margin;
  const uint8_t* _font_large;
  const uint8_t* _font_medium;
  const uint8_t* _font_small;
  const uint8_t* _font_symbol;
  const uint8_t* _current_font;
  
  // Note: if using transparencies (fontMode =1), use transparent fonts (u8g2_xxx_tx)
  // Note: font sets can consume vastly different amounts of
  // memory. Use reduced size fonts (u8g2_xxx_xr) to save memory.
  UiContext(U8G2* d, const uint8_t margin = 2, 
    const uint8_t* font_large = u8g2_font_helvB12_tr, 
    const uint8_t* font_medium = u8g2_font_helvR08_tr, 
    const uint8_t* font_small = u8g2_font_tom_thumb_4x6_tr, 
    const uint8_t* font_symbol = u8g2_font_open_iconic_gui_1x_t):
    display(d), _margin(margin), _font_large(font_large), _font_medium(font_medium), 
    _font_small(font_small), _font_symbol(font_symbol), _current_font(_font_large)
    {}
    
    void begin() {
      display->begin(); // initialize u8g2
      display->setFontMode(1); // font mode 1 enabled transparent fonts
      display->enableUTF8Print();
      setCurrentFont(_font_large);
    }
    void levelWear() {
       display->setDrawColor(2);
       display->drawBox( 0, 0, display->getDisplayWidth(), display->getDisplayHeight() );
       display->setDrawColor(1);
    }
    void setCurrentFont(const uint8_t* font) {
       display->setFont(font);
       _current_font = font;
    }
    const uint8_t* getCurrentFont() const {
      return _current_font;
    }
    const uint8_t* getFontLarge() const {
      return _font_large;
    }
    const uint8_t* getFontMedium() const {
      return _font_medium;
    }
    const uint8_t* getFontSmall() const {
      return _font_small;
    }
    const uint8_t* getFontSymbol() const {
      return _font_symbol;
    }

    const uint8_t getMargin() const { return _margin; }
    
};

#endif // End __UI_CONTEXT_H__ include guard