#ifndef __MAIN_MENU_SCREEN_H__
#define __MAIN_MENU_SCREEN_H__

#include "menu_screen.h"
#include "ui_context.h"

template < uint8_t MAX_SCREENS >
class MainMenu : public MenuScreen {
    
    //< the index of the current screen
    volatile uint8_t _value = 0;

    //< the index of the previous screen
    uint8_t _last_value = 0;

    //uint8_t _scroll_speed = 4;
    uint8_t _frames_to_go = 0;
    uint8_t _list_elm_height = 0;//11;

    //< an array of pointers to child screens
    MenuScreen* _sub_menus[MAX_SCREENS] = {nullptr};
    uint8_t _screen_count = 0;

    ////////

    //< index of the previous screen to scroll from
    int prev_screen = 0;
    //< index of the previous screen to scroll to
    int next_screen = 1;
    //< index of the next next screen to scroll to
    int next_next_screen = 2;
    //< y pixel for the top of the list (top of screen - text height)
    int list_top = 0;//10;

    //< distance to offset list top at the start of the scroll
    //< can be + or -
    int8_t scroll_offset = 10; 

    //< flag to track if currently scrolling
    bool _scrolling = false;

    uint8_t nextScreenIndex(const uint8_t current) const {
      uint8_t next_index = current + 1;
      return (next_index < _screen_count) ? next_index : 0;
      
    }

    uint8_t previousScreenIndex(const uint8_t current) const {
      return (current == 0) ?  (_screen_count - 1) : (current - 1);
    }

  public:
    MainMenu(MenuScreen* parent, UiContext* context, const char* title)
      : MenuScreen(parent, context, title) {
        context->setCurrentFont(context->getFontLarge());
        _list_elm_height = context->display->getMaxCharHeight();
        list_top = 20;//* _list_elm_height;

        // calculate the number of item to show on the menu
        uint8_t num_show = context->display->getDisplayHeight() / _list_elm_height + 1;

      }
    MainMenu(MenuScreen* parent, const __FlashStringHelper* title, UiContext* context)
      : MenuScreen(parent, context, title) {}

    /*!
     * @brief 
     * 
     * @param screen 
     * @return true 
     * @return false 
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
     * @brief 
     * 
     * @return true 
     * @return false 
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
     * @brief 
     * 
     * @return uint8_t 
     */
    uint8_t count() const {return _screen_count;}

    /*
    void setScrollSpeed(uint8_t scroll_speed) {
        _scroll_speed = scroll_speed;
    }*/

    /*!
     * @brief move to the next screen
     * 
     */
    void increment() {
      if(_scrolling) return;
      _last_value = _value;
      _value = nextScreenIndex(_value);
      setDirty(true);
      updateScrollData();
    }

    /*!
     * @brief move to the previous screen
     * 
     */
    void decrement() {
      if(_scrolling) return;
      _last_value = _value;
      _value = previousScreenIndex(_value);
      setDirty(true);
      updateScrollData();
    }

    MenuScreen* enter() {
      if(_scrolling) {
        return _sub_menus[_last_value];
      } 
      else {
        return _sub_menus[_value];
      }
      //_sub_menus[_value]->setDirty(true);
      //updateScrollData();
    }

    /*!
     * @brief called whenever the menu needs to be redrawn
     * 
     * @param dirty 
     */
    void setDirty(bool dirty) {
      
      if(!_scrolling) {
        // not scrolling, call base class
        MenuScreen::setDirty(dirty);
      }
      else {
          // scrolling. set the list top
          list_top += scroll_offset;
        // finally, check if there are more frames to draw
        --_frames_to_go;
        if( _frames_to_go == 0 ) {
          //_last_value = _value; //store last value to prevent scrolling on menu show
          //Serial.println("done scrolling");
          //Serial.println("........");
          //Serial.println(LT_current_time_us);
          _scrolling = false;
        }
      }
    }

    /*!
     * @brief sets up member variables at the start of a scroll
     * when scrolling "down", the list of menus on-screen is drawn
     * and one additional menu is added to the bottom.
     * 
     * When scrolling "up", the list of menus on-screen is drawn and
     * one additional menu is added to the top.
     * 
     */
    void updateScrollData() {
      // collect some info about the screens that will be scrolled through
      // this depends on screen height and font height
      prev_screen = previousScreenIndex(_value);
      next_screen = nextScreenIndex(_value);
      next_next_screen = nextScreenIndex(next_screen);
      //prev_screen = (_value - 1 + _screen_count) % _screen_count;
      //next_screen = (_value + 1) % _screen_count;
      //next_next_screen = (next_screen + 1) % _screen_count;
    
      //int8_t d2go = con
      // test for scroll direction
       if ( (_last_value == next_screen) && (scroll_offset != 0) ) {
        // scroll up
        //scroll_offset = 5;
        //list_top = 18; // text box h 
        scroll_offset = 2;
        list_top = 10;

        // draw scrolling animation in 4 frames
        // (frames * offset = distance scrolled [px] )
        _frames_to_go = 5; // (20px/5px) + 1
      }
      else if ( (_last_value == prev_screen) && (scroll_offset != 0) ) {
        //scroll down
        //scroll_offset = -5;
        //list_top = 58; //~ screen height
        scroll_offset = -2;
        list_top = 30; //~ screen height

        // draw scrolling animation in 4 frames
        // (frames * offset = distance scrolled [px] )
        _frames_to_go = 5;
      }
      else {
       // Serial.println("s no");
        // don't scroll
        // jump stright to the next item (no scrolling)
        //scroll_offset = 20;
        //list_top = 38; 
        scroll_offset = 12; // make the height of each scroll 12
        list_top = 20;
        _frames_to_go = 1; // only draw 1 frame (the completed scroll)
      }
      _scrolling = true;
    }

    /*!
     * @brief this function is called by the main program loop
     * if this object has been set dirty. 
     * For screens that are not stored in a full buffer, this will be as many
     * times as it takes to draw the full screen (ie 4 times for 1/4 buffer)
     * 
     * @param context a pointer to the ui context with the screen info
     */
    void draw(UiContext* context) {
      // set to large font
      context->setCurrentFont(context->getFontLarge());
      uint8_t w_tot = context->display->getDisplayWidth();
      uint8_t h_txt = context->display->getAscent();
      uint8_t margin = context->getMargin();
      uint8_t h_tot = h_txt + margin;

      //draw selection bar first
      context->display->setDrawColor(1); // draw color 1 = foreground color 1 (on)
      // draw a box around the selected text (x, y, w, h)
      context->display->drawBox(0, _list_elm_height , w_tot, _list_elm_height);
      Serial<<_list_elm_height;

      // draw the list of submenus
      context->display->setDrawColor(2); // draw color 2 = xor
      // set point to bottom left character of first menu entry
      context->display->setCursor(margin, list_top - _list_elm_height);
      if(_sub_menus[prev_screen] != nullptr)
        _sub_menus[prev_screen]->printTitle(context);
          
      context->display->setCursor(margin, list_top);
      if(_sub_menus[_value] != nullptr)
        _sub_menus[_value]->printTitle(context);
      
      context->display->setCursor(margin, list_top + _list_elm_height + margin);
      if(_sub_menus[next_screen] != nullptr)
        _sub_menus[next_screen]->printTitle(context);
         
      context->display->setCursor(margin, list_top + (2* _list_elm_height + margin) );
      if(_sub_menus[next_next_screen] != nullptr)
        _sub_menus[next_next_screen]->printTitle(context);
         
      // now black out behind the title
      context->display->setDrawColor(0); // draw color 0 = foreground color 0 (off)
      context->display->drawBox(0, 0, context->display->getDisplayWidth(), _list_elm_height);
          
      context->display->setDrawColor(1);

      // draw centered title, position cursor to bottom of left-most character
      context->display->setCursor( (context->display->getDisplayWidth() - titleWidth(context) ) >> 1, h_txt);
      MenuScreen::printTitle(context);

      //draw horizontal seperator line
      context->display->drawLine(0, h_tot, context->display->getDisplayWidth(), h_tot);

      //list-top now starts at 22
      //text lower-left is 39
    }
};

#endif // End __MAIN_MENU_SCREEN_H__ include guard
