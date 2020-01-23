template < uint8_t MAX_SCREENS >
class Menu : public MenuScreen {
  private:
    uint8_t _screen_count = 0;
    volatile uint8_t _value = 0;
    uint8_t _last_value = 0;
    uint8_t _scroll_speed = 5;
    uint8_t _frames_to_go = 0;

    MenuScreen _base;
    MenuScreen _sub_menus_o[MAX_SCREENS];
    MenuScreen* _sub_menus[MAX_SCREENS];

    UiContext* _c;

    ////////
    int prev_screen = 0;
    int next_screen = 1;
    int next_next_screen = 2;
    int list_top = 38;
    int scroll_offset = 20;
    bool _scrolling = false;
    
  public:
    void begin() {
      appendMenu(*parent());
      appendMenu(_base);
    }
    
    Menu(MenuScreen* parent, UiContext* context, const char* title) 
    : MenuScreen(parent, context, title) {
      _c = context;
      _base = MenuScreen(this, context, "Add Step");
      _base.setScreenEnteredCallback(onNewStep);
    }

    static void onNewStep(void* ptr) {
        Menu<MAX_SCREENS>* m = static_cast<Menu<MAX_SCREENS>*>(ptr);
        MenuScreen s(m, m->context(), "Step");
        m->appendMenu(s);
    }

    UiContext * context() {return _c;}
    
    virtual void increment() {
      if(_scrolling) return;
      _last_value = _value;
      _value = (_value + 1) % _screen_count;
      setDirty(true);
      updateScrollData();
    }
    virtual void decrement() {
      if(_scrolling) return;
      _last_value = _value;
      _value = (_value - 1 + _screen_count) % _screen_count;
      setDirty(true);
      updateScrollData();
    }

    MenuScreen* enter() {
      if(_scrolling) {
        return _sub_menus[_last_value] -> enter();
      } 
      else {
        return _sub_menus[_value] -> enter();
      }
    }

    bool appendMenu(MenuScreen screen) {
      if (_screen_count < MAX_SCREENS) {
        _sub_menus_o[_screen_count] = screen;
        _sub_menus[_screen_count] = &_sub_menus_o[_screen_count];
        _screen_count++;
        return true;
      }
      else {
        return false;
      }
    }

    void updateScrollData() {
      prev_screen = (_value - 1 + _screen_count) % _screen_count;
      next_screen = (_value + 1) % _screen_count;
      next_next_screen = (next_screen + 1) % _screen_count;
    
      // test for scroll direction
       if ( (_last_value == next_screen) && (scroll_offset != 0) ) {
        // Serial.println("s up");
        // scroll up
        scroll_offset = 5;
        list_top = 18;
        _frames_to_go = 4; // (20px/5px) + 1
      }
      else if ( (_last_value == prev_screen) && (scroll_offset != 0) ) {
       // Serial.println("s dn");
        //scroll down
        scroll_offset = -5;
        list_top = 58;
        _frames_to_go = 4;
      }
      else {
       // Serial.println("s no");
        // don't scroll
        scroll_offset = 20;
        list_top = 38;
        _frames_to_go = 1;
      }
      _scrolling = true;
    }

    void draw(UiContext* context) {
      context->setFont(context->fontLarge());

      //draw selection bar first
      context->display->setDrawColor(1);
      context->display->drawBox(0, 22, context->display->getDisplayWidth(), 20);

      // draw the list
      context->display->setDrawColor(2); // xor
      context->display->setCursor(8, list_top - 20);
      if(_sub_menus[prev_screen] != nullptr)
        _sub_menus[prev_screen]->printTitle(context);
          
      context->display->setCursor(8, list_top);
      if(_sub_menus[_value] != nullptr)
        _sub_menus[_value]->printTitle(context);
      
      context->display->setCursor(8, list_top + 20);
      if(_sub_menus[next_screen] != nullptr)
        _sub_menus[next_screen]->printTitle(context);
         
      context->display->setCursor(8, list_top + 40);
      if(_sub_menus[next_next_screen] != nullptr)
        _sub_menus[next_next_screen]->printTitle(context);
         
      // now black out behind the title
      context->display->setDrawColor(0); //0
      context->display->drawBox(0, 0, context->display->getDisplayWidth(), 22);
          
      context->display->setDrawColor(1);
      context->display->setCursor( (context->display->getDisplayWidth() - titleWidth(context) ) >> 1, 16);
      MenuScreen::printTitle(context);
      //draw seperator
      context->display->drawLine(0, 20, context->display->getDisplayWidth(), 20);

      //list-top now starts at 22
      //text lower-left is 39
    }

    void setDirty(bool dirty) {
      /*Serial.print("Mm set dirty? ");
      Serial.print(dirty);
      Serial.print(" is dirty? ");
      Serial.print(isDirty());
      Serial.print(" is scroll? ");
      Serial.println(_scrolling);*/
      if(!_scrolling) {
        MenuScreen::setDirty(dirty);
      }
      else {
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
};
