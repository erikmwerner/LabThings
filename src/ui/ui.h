
#ifndef __UI_H__
#define __UI_H__

#include <U8g2lib.h>
#include <U8x8lib.h>

#include "../devices/device.h"
#include "menu_screen.h"
#include "ui_context.h"

/*!
* The Ui class manages the objects in a graphical user 
* interface. It implements the composite design pattern,
* where the Ui serves as the client and can manipulate a set of
* objects that inherit from the GraphicsItem class.
*
* The Ui class inherits from LT_Device so it can be updated using the 
* DeviceManager update() function. By default, type() returns LT::Ui
*/
class Ui : public LT_Device {
    UiContext* _context; ///< pointer to the graphics context (output device)
    MenuScreen* _current_screen = nullptr; ///< pointer to the current screen
    MenuScreen* _home_screen = nullptr; ///< pointer to default screen
    
    /*
    uint8_t _fps = 0; //hold fps calculation data
    uint8_t _frames = 0;
    uint32_t _t_last_fps_update = 0;
    */
    uint32_t _t_last_input_us = 0; ///<  Track the time of the last user input
    
    uint32_t _return_home_timeout_us = 0; ///< Track time to automatically return home. Set to zero to disable.
    
    uint32_t _screensaver_timeout_us = 0; ///< Track time to start a screensaver. Set to zero to disable

    bool _screensaver_enabled = false; ///< Enable screen saver option.
    
    ///< If set to false, and the screensaver is activated, the display is put to sleep instead.
    bool _screensaver_active = false;

    bool _is_sleeping = false;
    
    /*
    int8_t calcFPS() {
      _frames++;
      if( (LT_current_time_us - _t_last_fps_update) >= 1000000) { // 1 second
        _fps = _frames;
        _frames = 0;
        _t_last_fps_update = LT_current_time_us;
      }
      return _fps;
    }
    */
    
  public:
    Ui( const uint8_t id, UiContext *context ) 
    : LT_Device(id), _context(context){}

    LT::DeviceType type() const {return LT::Ui; }
    
    /**
     * @brief this function is usually called when the ui is attached to a device manager
     * 
     */
    void begin() {
      _context->begin();
      //_t_last_fps_update = LT_current_time_us;
      //if ( _current_screen != NULL ) {
      //  _current_screen->draw(&_context);
      //}
    }

    /**
     * @brief check this ui and all of its objects and draw new graphics
     *  if necessary. This function shoule be called as often as possible. 
     * This function is usually called by the device manager.
     */
    void update() {
      // main drawing function
      if( _current_screen != nullptr ) {
        // only redraw if the screen has changed
        if( _current_screen->isDirty() ) {
          // uses U8G2 to split up the display
          // draw() will be called:
          //  * once for full buffer (F), 
          //  * twice if using half buffer (2)
          //  * four times if using quarter buffer (1)
          // for screens with many elements, full buffer will be much faster
          _context->display->firstPage();
          do {
            _current_screen->draw(_context);
            if( _screensaver_active ) {
              _context->levelWear();
            }
          } while ( _context->display->nextPage() );

          // (optional) calculate FPS
          //calcFPS();
          
          // mark the screen as drawn last
           _current_screen->setDirty(false);
        }
      }
      else {
        // clear the display if the current display is a null pointer
        _context->display->clear();
      }
      
      // check if it is time to return to home screen
      if ( ( _home_screen != nullptr ) && ( _return_home_timeout_us > 0 ) ) {
        if ( _current_screen != _home_screen ) {
          if ( ( LT_current_time_us - _t_last_input_us ) >= _return_home_timeout_us ) {
            setCurrentScreen( _home_screen );
            _current_screen->draw(_context);
          }
        }
      }

      // check if it is time to activate screen saver
      if( _screensaver_timeout_us > 0 ) {
        if ( (LT_current_time_us - _t_last_input_us) >= _screensaver_timeout_us ) {
          // option 1: invert colors to level OLED pixel wear
          if( _screensaver_enabled ) {
            // toggle the screensaver flag
            _screensaver_active = ! _screensaver_active;
            // set the current screen dirty
            if( _current_screen != nullptr ) {
              _current_screen->setDirty(true);
            }
            // update the last input time
            _t_last_input_us = LT_current_time_us;
          }
          // option 2: turn off screen (power saving)
          else {
            // sleep
            if(!_is_sleeping) {
              setSleeping(true);
            }
          }
        }
      }
    }

    /**
     * @brief Set the Ui context
     * 
     * @param context a pointer at the context. This is expected to be
     * a valud UiContext. This context will become the current context.
     */
    void setContext(UiContext *context) {
      _context = context;
    }
    
    // Sleep or wake the monitor. This always resets the screen saver
    void setSleeping( bool isSleeping ) {
      _is_sleeping = isSleeping;
      _screensaver_active = false;
      _context->display->setPowerSave( isSleeping );
    }
    
    bool isSleeping() {
      return _is_sleeping;
    }

    // MenuScreen should not be NULL
    void setCurrentScreen(MenuScreen* screen) {
      _current_screen = screen;
      _current_screen->setDirty(true);
    }


   // enable to XOR all pixels on the OLED every timeout
   // disable to sleep the display upon timeout
    void setScreenSaverEnabled( bool screensaver_enabled ) {
      _screensaver_enabled = screensaver_enabled;
    }
    
    // sets the lengh of time in microseconds 
    // before screensaver function is activated
    // set to zero to disable
    void setScreenSaverTimeout(uint32_t timeout) {
      _screensaver_timeout_us = timeout;
    }
    
    // sets the lengh of time in microseconds 
    // before the Ui attempts to return to a home screen
    // set to zero to disable
    // the Ui will do nothing if no home screen is set
    void setHomeScreenTimeout(uint32_t timeout) {
      _return_home_timeout_us = timeout;
    }
    
    // Warning: MenuScreen can be a NULL pointer
    const MenuScreen* currentScreen(){ 
      return _current_screen;
    }
    
    // sets a screen to return to after _return_home_timeout_us has elapsed
    // SA setHomeScreenTimeout
    void setHomeScreen(MenuScreen* screen) {
      _home_screen = screen;
    }

    // user input functions
    void increment() {
      _t_last_input_us = LT_current_time_us;
      if(_is_sleeping) { 
        setSleeping(false);
      }
      if(_current_screen != nullptr) {
        _current_screen->increment();
      }
    }

    void decrement() {
      _t_last_input_us = LT_current_time_us;
      if(_is_sleeping) { 
        setSleeping(false);
      }
      if(_current_screen != nullptr) {
        _current_screen->decrement();
      }
    }

    void enter() {
      _t_last_input_us = LT_current_time_us;
      if(_is_sleeping) { 
        setSleeping(false);
      }
      if(_current_screen != nullptr) {
        MenuScreen* next_screen = _current_screen->enter();
        if(next_screen != nullptr) {
          setCurrentScreen(next_screen);
        }
      }
    }
};

#endif // End __UI_H__ include guard
