#ifndef __MAIN_MENU_SCREEN_H__
#define __MAIN_MENU_SCREEN_H__

#include "menu_screen.h"
#include "ui_context.h"

/**
 * @brief 
 * 
 * @tparam MAX_SCREENS defines the maximum number of list entries in the menu
 */
template < uint8_t MAX_SCREENS >
class MainMenu : public MenuScreen {
    
    volatile uint8_t _value = 0; //< the index of the current screen
    uint8_t _last_value = 0; //< the index of the previous screen

    uint8_t _properties = 5; //< holds appearance settings
    // b0 = underline / no underline
    // b1 = midline / no midline
    // b2 = animate scroll / no animation
    // b3 = wrap / no wrap
    // b4...b7 unused

    uint8_t _h_title_txt = 0; //< max rise of characters in font used for title
    uint8_t _title_stop= 0;
    uint8_t _h_list_txt = 0; //< max rise of characters in font used for list elements
    uint8_t _h_list_elm = 0; //< max rise of characters in font used for list elements
    uint8_t _w_tot = 0; //< total width of the screen
    uint8_t _list_count = 0;
    int8_t _offset = 0;

    //< an array of pointers to child screens
    MenuScreen* _sub_menus[MAX_SCREENS] = {nullptr};
    uint8_t _screen_count = 0; //< the number of screens in the list
    uint8_t _list_show_count = 0;

    //< flag to track if currently scrolling
    int8_t _scroll_step = 1;
    int8_t _scroll_offset = 0;
    uint8_t _scroll_frames = 5;

    /**
     * @brief returns the index of the next screen in the list. rolls over
     * if the current screen is the final screen
     * 
     * @param current 
     * @return uint8_t 
     */
    uint8_t nextScreenIndex(const uint8_t current) const {
      uint8_t next_index = current + 1;
      return (next_index < _screen_count) ? next_index : 0;
    }

    /**
     * @brief returns the index of the previous screen in the list.
     * Rolls over if the current screen is the first screen
     * 
     * @param current 
     * @return uint8_t 
     */
    uint8_t previousScreenIndex(const uint8_t current) const {
      return (current == 0) ?  (_screen_count - 1) : (current - 1);
    }

  public:
    MainMenu(MenuScreen* parent, UiContext* context, const char* title, const int icon = 0)
      : MenuScreen(parent, context, title, icon) {
        if(parent != nullptr) {
          addScreen(parent);
        }
        context->setCurrentFont(context->getFontLarge());
        _h_title_txt = context->display->getAscent();
        // compute the total height of the title, including underline style options
        _title_stop = _h_title_txt + (_properties & 0x1 ? 4 : 2);
        context->setCurrentFont(context->getFontMedium());
        _h_list_txt = context->display->getAscent();
        _w_tot = context->display->getDisplayWidth();
        // compute the remaining display height left over for list elements
        uint8_t list_height = context->display->getDisplayHeight() - _title_stop;
        // compute the height of each list element
        _h_list_elm = context->display->getMaxCharHeight() + 2 * context->getMargin() - 1;
        // calulate the number of items in the list that will be visible at any time
        // 2 is spacing between list rows (hard coded for now)
        _list_show_count = (list_height)/( _h_list_elm+ 1) + 1; // add one and round up
      }

    // alternate constructor for storing text in flash
    //MainMenu(MenuScreen* parent, const __FlashStringHelper* title, UiContext* context)
     // : MenuScreen(parent, context, title) {}

    /*!
     * @brief adds a screen to the menu. The screen is inserted at the end of the list.
     * 
     * @param screen a pointer to the screen to add
     * @return true if the screen was added to the list
     * @return false if the screen was not added (the list is full)
     */
    bool addScreen(MenuScreen * screen) {
      if (_screen_count < MAX_SCREENS) {
        _sub_menus[_screen_count++] = screen;
        return true;
      }
      else {
        return false;
      }
    }

    /*!
     * @brief removes the last screen from the list
     * 
     * @return true if a screen was removed from the end of the list
     * @return false if a screen could not be removed (the list is empty)
     */
    bool removeScreen() {
      if(_screen_count > 0) {
        _sub_menus[_screen_count--] = nullptr;
        return true;
      }
      else {
        return false;
      }
    }

    /*!
     * @brief counts the number of screens in the list
     * 
     * @return uint8_t the number of screens in the list
     */
    uint8_t count() const {return _screen_count;}

    void adjust(const int8_t delta) {
      if(_scroll_offset != 0) return;
      _last_value = _value;
      _value = (delta > 0) ? nextScreenIndex(_value) : previousScreenIndex(_value);
      setDirty(true);
      _scroll_offset = _h_list_elm * delta;
      _scroll_step = -_h_list_elm/4 * delta;
    }

    /*!
     * @brief Called by the ui when the user inputs "up.
     * Moves the selected screen to the next screen in the list
     * and requests a redraw
     */
    void increment() {
      adjust(1);
      /*if(_scroll_offset != 0) return;
      _last_value = _value;
      _value = nextScreenIndex(_value);
      
      setDirty(true);
      _scroll_offset = _h_list_elm;
      _scroll_step = -_h_list_elm/4;*/
    }

    /*!
     * @brief Called by the ui when the user inputs "down."
     * Moves the selected screen to the previous screen in the list
     * and requests a redraw
     */
    void decrement() {
      adjust(-1);
      /*if(_scroll_offset != 0) return;
      _last_value = _value;
      _value = previousScreenIndex(_value);
      
      setDirty(true);
      _scroll_offset = -_h_list_elm;
      _scroll_step = _h_list_elm/4;*/
    }

    /**
     * @brief 
     * 
     * @return MenuScreen* a pointer at the screen that was selected
     */
    MenuScreen* enter() const {
      if(_scroll_offset == 0) {
        return _sub_menus[_value];
      } 
      else {
        return _sub_menus[_last_value];
      }
    }

    /*!
     * @brief called whenever the menu needs to be redrawn
     * If the menu is scrolling, this function will
     * keep the dirty flag set until the scrolling is complete.
     * 
     * @param dirty 
     */
    void setDirty(bool dirty) {
      
      if(_scroll_offset == 0) {
        // not scrolling, call base class
        MenuScreen::setDirty(dirty);
      }
      else {
        // in the middle of scrolling...
        _scroll_offset += _scroll_step;
        if(abs(_scroll_offset) < abs(_scroll_step)) {
          _scroll_offset = 0;
        }
      }
    }

    void setProperties(const uint8_t properties) { _properties = properties;}

    /*!
     * @brief this function is called by ui from the main program loop if
     * the _dirty flag has been set to true.
     * For screens that are not stored in a full buffer, this will be as many
     * times as it takes to draw the full screen (ie 4 times for 1/4 buffer)
     * 
     * @param context a pointer to the ui context with the screen info
     */
    void draw(UiContext* context) {
      /*Serial.print("Value:");
      Serial.print(_value);
      Serial.print(" Draw. Offset:");
      Serial.print(_scroll_offset);
      Serial.print(" step:");
      Serial.println(_scroll_step);*/

      // draw the list of submenus
      context->setCurrentFont(context->getFontMedium());
      context->display->setDrawColor(2); // draw color 2 = xor
      
      uint8_t draw_index = _value;
      if(_scroll_offset == 0) {
      // done scrolling
        for(uint8_t i = 0; i < _list_show_count; ++i) {
          uint8_t y = context->getMargin() + _title_stop + _h_list_txt + _h_list_elm*i;
          uint8_t x = context->getMargin();
          if(_sub_menus[draw_index] != nullptr) {
            _sub_menus[draw_index]->printTitle(context, x, y, true);
          }
          else {}
          draw_index = nextScreenIndex(draw_index);
        } 
      }
      else if(_scroll_offset > 0) {
        // scrolling up
        // draw two extra items when scrolling
        draw_index = previousScreenIndex(draw_index);
        for(uint8_t i = 0; i < _list_show_count + 2; ++i) {
          uint8_t y = _scroll_offset + _h_list_txt + _h_list_elm*i;
          uint8_t x = context->getMargin();
          if(_sub_menus[draw_index] != nullptr) {
            _sub_menus[draw_index]->printTitle(context, x, y, true);
          }
          else {}
          draw_index = nextScreenIndex(draw_index);
        }
      }
      else {
        // scrolling down
        // draw two extra items when scrolling
        for(uint8_t i = 0; i < _list_show_count + 2; ++i) {
          uint8_t y = _scroll_offset + context->getMargin() + _title_stop + _h_list_txt + _h_list_elm*i;
          uint8_t x = context->getMargin();
          if(_sub_menus[draw_index] != nullptr) {
            _sub_menus[draw_index]->printTitle(context, x, y, true);
          }
          else {}
          draw_index = nextScreenIndex(draw_index);
        }
      }
      // black out behind the title
      context->display->setDrawColor(0); // draw color 0 = foreground color 0 (off)
      context->display->drawBox(0, 0, context->display->getDisplayWidth(), _title_stop);

      // draw centered title, position cursor to bottom of left-most character
      context->setCurrentFont(context->getFontLarge());
      context->display->setDrawColor(1);
      uint8_t x = (context->display->getDisplayWidth() - titleWidth(context) ) >> 1;
      uint8_t y = _h_title_txt;
      MenuScreen::printTitle(context, x, y);
      if(_properties & 0x1) {
        //draw horizontal seperator line
        context->display->drawLine(0, _title_stop - 2, context->display->getDisplayWidth(), _title_stop - 2 );
      }
      //draw the selection bar
      context->display->setDrawColor(2); // // draw color 2 = xor
      // draw a box around the selected text (x, y, w, h)
      context->display->drawBox(0, _title_stop , _w_tot, _h_list_elm);
    }
};

#endif // End __MAIN_MENU_SCREEN_H__ include guard
