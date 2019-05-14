#ifndef __MESSAGE_HANDLER_H__
#define __MESSAGE_HANDLER_H__

#include "commands.h"

//extern "C" {
// callback functions always follow the signature: void cmd(void);
typedef void(*func_ptr) ();
//};

class MessageHandler {

    // list of all attached callback functions
    func_ptr _function_list[MAX_FUNCTIONS] = { NULL };
    func_ptr _default_function = NULL;
  public:
    // route the message to
    void handleMessage( int index ) {
      if (index >= 0 && index < MAX_FUNCTIONS) {
        if(_function_list[index] != NULL) {
          (_function_list[index])();
        }
        else {
          if(_default_function != NULL) {
            (_default_function)();
          }
        }
      }
      else {
        // Error: function index > MAX_FUNCTIONS
      }
    }

    void attachFunction( const uint8_t id, func_ptr f ) {
      if (id >= 0 && id < MAX_FUNCTIONS) {
        _function_list[id] = f;
      }
    }
    
    void attachDefaultFunction( func_ptr f ) {
      _default_function = f;
    }
    
    uint8_t getAttachedFunctionCount() {
      uint8_t count = 0;
      for(uint8_t i = 0; i<MAX_FUNCTIONS; ++i) {
        if(_function_list[i] != NULL) {
          count++;
        }
      }
      return count;
    }
    
    void getAttachedFunctions( uint8_t data[] ) {
      //uint8_t max_count = sizeof(data)/sizeof(data[0]);
      uint8_t index = 0;
      for(uint8_t i = 0; i<MAX_FUNCTIONS; ++i) {
        if( (_function_list[i] != NULL) ) {
          data[index]=i;
          index++;
        }
      }
    }
};

#endif //End __MESSAGE_HANDLER_H__ include guard
