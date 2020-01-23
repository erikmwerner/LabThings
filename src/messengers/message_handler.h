#ifndef __MESSAGE_HANDLER_H__
#define __MESSAGE_HANDLER_H__

#include "commands.h"

//extern "C" {
// callback functions always follow the signature: void cmd(void);
typedef void(*func_ptr) (void* sender);
//};

/*!
 * @brief The MessageHandler class manages an array of pointers
 * to callback functions. A function code is used to index
 * the pointer.
 * 
 * To use:
 * 
 * In global: define a message handler
 * MessageHandler handler;
 * 
 * define a callback function:
 * // note: some compilers will complain if you omit the void* argument
 * void onReadSensorValue(void*) { // do stuff }
 * 
 * In setup(): attach callback function
 * handler.attachFunction(LT::Read_Sensor_Value, onReadSensorValue);
 * 
 * Now whenever handler gets a message
 * handler.handleMessage(msg_id);'
 * If the msg_id == LT::Read_Sensor_Value, onReadSensorValue will be called
 */
class MessageHandler {

    //< a vector to store pointers to attached callback functions
    func_ptr _function_list[MAX_FUNCTIONS] = { nullptr };
    //< a pointer to a default callback function
    func_ptr _default_function = nullptr;
  
  public:
    /*!
     * @brief attempts to call a function that has been
     * attached to a command id. If there is no function
     * at the index, it attempts to call the default callback function
     * 
     * @param index the ID (function code) of the command
     * @param sender an optional pointer at the object
     * that sent the message
     */
    void handleMessage( int index, void* sender = nullptr ) {
      if (index >= 0 && index < MAX_FUNCTIONS) {
        if(_function_list[index] != nullptr) {
          (_function_list[index])(sender);
        }
        else {
          if(_default_function != nullptr) {
            (_default_function)(sender);
          }
        }
      }
      else {
        //DEBUG_PRINT "Warning: function code OOB";
      }
    }

    /*!
     * @brief attachFunction stores a function pointer func_ptr
     * at the index id of the function pointer array. If there is
     * already a function at the index, it is replaced.
     * 
     * example: to call the function checkSensor() when the function code
     * Read_Sensor_Setting is received: 
     * attachFunction(Read_Sensor_Setting, checkSensor)
     * 
     * @param id the function code to assign to the callback
     * @param f the callback function
     */
    void attachFunction( const uint8_t id, func_ptr f ) {
      if (id >= 0 && id < MAX_FUNCTIONS) {
        _function_list[id] = f;
      }
    }

    /*!
     * @brief attachDefaultFunction stores a pointer to a default
     * callback function that can be called when the function
     * code of a message does not have any callback assigned to it.
     * 
     * @param f 
     */
    void attachDefaultFunction( func_ptr f ) {
      _default_function = f;
    }
    
    /*!
     * @brief attachedFunctionCount returns the number
     * of functions that have been assigned to callbacks
     * 
     * @return uint8_t the number of callback functions that have been registered
     */
    uint8_t attachedFunctionCount() {
      uint8_t count = 0;
      for(uint8_t i = 0; i<MAX_FUNCTIONS; ++i) {
        if(_function_list[i] != nullptr) {
          count++;
        }
      }
      return count;
    }
    
    /*!
     * @brief attachedFunctions returns an array of function codes
     * that have been assigned to callback functions
     * 
     * example use:
     * // get the number of attached functions and allocate array
     * uint8_t function_count = handler.attachedFunctionCount();
     * int8_t function_list[function_count] = {0};
     * handler.attachedFunctions(function_list, function_count);
     * 
     * @param data an array to full with function codes that have 
     * callbacks attached to them
     * @param len the length of the data array to full. len 
     * must be greater or equal to the number of functions attached
     * @return true if the list was successfully retrieved
     * @return false if the list could not be retrieved
     */
    bool attachedFunctions( uint8_t data[], const uint8_t len ) {
      if(len < attachedFunctionCount()) {
        return false;
      }
      else {
        uint8_t index = 0;
        for(uint8_t i = 0; i<MAX_FUNCTIONS; ++i) {
         if( (_function_list[i] != nullptr) ) {
            data[index]=i;
           index++;
         }
       }
       return true;
      }
    }
};

#endif //End __MESSAGE_HANDLER_H__ include guard
