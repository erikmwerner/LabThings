#ifndef __PROCESS_MANAGER_H__
#define __MESSAGE_HANDLER_H__

#include "../utilities/ring_buffer.h"

typedef void (*intCallback)(int);

namespace Process_Info {
  enum INFO_CODE : uint8_t {
    Process_Update = 0,
    Command_Started = 1,
    Command_Ended = 2, 
    Process_Ended = 3
  };
}

struct CommandData{
  int8_t code; //< The function code of this command
  int8_t dev_id; //< The device id that this command targets
  uint32_t data0; //< 0th data field. 
  uint32_t data1; //< 1st data field.
  uint32_t duration; //< duration of the command in microseconds
  

  CommandData(
    int8_t code = -1,
    int8_t dev_id = -1,
    uint32_t d0 = 0,
    uint32_t d1 = 0,
    uint32_t dur = 0
  ):
    code(code),
    dev_id(dev_id),
    data0(d0),
    data1(d1),
    duration(dur)
  {}
};

/*! a simple class to hold a small sequence of 
commands to be used once as an interrupt
 */
template <uint8_t LENGTH>
class ProcessVector {
  uint8_t _count = 0;
  uint8_t _next = 0;
  CommandData _v[LENGTH];
public: 
  // zero the memory of this object 
  void reset(){
    memset (this, 0, sizeof (*this));
    _count = 0;
    _next = 0;
  }
  // add a command to the buffer. The count
  // variable is incremented to indicate the buffer
  // has another command
  bool put(const CommandData data) {
    if(_count < LENGTH) {
      _v[_count++] = data;
      return true;
    }
    else {
      return false;
    }
  }
  // if successful, reads the command at the next index
  // into data and increments the next index
  int8_t next(CommandData* data) {
    if(_next < _count) {
      *data = _v[_next++];
      return 0;
    }
    else {
      return -1;
    }
  }
  int8_t get(const uint8_t index, CommandData* data) {
    if(index < _count) {
      *data = _v[index];
      return 0;
    }
    else {
      // error index OOB
      return -1;
    }

  }
  uint8_t count() const { return _count; }
};

template<uint8_t BUFFER_LENGTH, uint8_t INTERRUPT_COUNT, uint8_t INTERRUPT_LENGTH>
class ProcessManager {

  RingBuffer<BUFFER_LENGTH, CommandData> _buffer; ///< sequential command buffer
  ProcessVector<INTERRUPT_LENGTH> _interrupts[INTERRUPT_COUNT]; ///< 

  CommandData _current = CommandData();
  CommandData _last = CommandData();
  CommandData _waiting;
  bool _running = false; ///< flag to track if the protocol is currrently running
  uint8_t _vector = 0; ///< flag to track if the protocol is interrupted. 0 is false. > 0 is the (vector - 1)
  uint8_t _last_vector = 0;
  uint32_t _pause_time = 0; ///< the system time in microseconds that the protocol was paused
  uint32_t _last_command_start_time = 0; ///< the system time in microseconds that the last command was started
  uint32_t _last_elapsed = 0;

  intCallback _command_started_callback = nullptr;
  intCallback _command_ended_callback = nullptr;
  intCallback _process_ended_callback = nullptr;

public:
  void update() {
    // do nothing if the process is stopped
    if(!_running) return;
    
    int32_t dt = LT_current_time_us - _last_command_start_time;

    // if more time has elapsed than the current command duration
    if(dt >= _current.duration) {
      if(_vector > 0) { // run interrupt if necessary
        runInterrupt();
      }
      else { // running sequential command vector
        CommandData next_cmd;
        if( _buffer.takeBack(&next_cmd) == 0 ) {
          _last = _current;
          _current = next_cmd;
          _last_command_start_time = LT_current_time_us;
          updateCommand();
        }
        else {
          // there are no more commands in the buffer. stop the process
          _running = false;
          _last = _current;
          // sets the current command to an invalid command
          // the command code is oob to the handler
          _current = CommandData(); 
          if(_command_ended_callback != nullptr) {
            (_command_ended_callback)(_vector);
          }
          if(_process_ended_callback != nullptr) {
            (_process_ended_callback)(0);
          }
        }
      } 
    }
    else {
      // the current command is still valid
    }
  }

  const CommandData* currentCommand() const {return &_current;}
  const CommandData* previousCommand() const {return &_last;}

  const int8_t peek(const uint8_t queue, const uint8_t index, CommandData* c) const {
    if(queue == 0) {
      return _buffer.get(index, c);
    }
    else if(queue <= INTERRUPT_COUNT) {
      return _interrupts[queue-1].get(index, c);
    }
    else {
      return -1;
    }
  }

  /*!
   */
  inline void updateCommand() {
    if(_command_started_callback != nullptr) {
        (_command_started_callback)(_current.code);
    }
    if(_command_ended_callback != nullptr) {
        (_command_ended_callback)(_last_vector);
    }
  }
  /**
   * @brief 
   * 
   * @return uint32_t the amount of time remaining until the current 
   * command is finished in microseconds. Returns 0 if there is no time remaining
   */
  uint32_t remainingTime() const {
    uint32_t end_time = _last_command_start_time + _current.duration;
    return (end_time  > LT_current_time_us) ? (end_time - LT_current_time_us) : 0;
  }

  /**
   * @brief 
   * 
   * @return uint32_t the amount of time that has passed since
   * the current command started in microseconds.
   */
  uint32_t currentCommandElapsedTime() const {
    return (LT_current_time_us - _last_command_start_time);
  }

  void setCommandStartedCallback( intCallback f ) {
      _command_started_callback = f;
  }

  void setCommandEndedCallback( intCallback f ) {
      _command_ended_callback = f;
  }

  void setProcessEndedCallback( intCallback f ) {
      _process_ended_callback = f;
  }

/**
 * @brief returns the current queue that the process manager is running.
 * 
 * @return uint8_t from 0 to INTERRUPT_COUNT
 */
  uint8_t currentVector() const { return _vector; }

  /*!
  * @brief 
  * @param q the queue to access. q == 0 is the main buffer. 
  * q == 1 : INTERRUPT_COUNT is an interrupt vector
  * if q > INTERRUPT_COUNT, returns 0
  * @return the number of spaces available in the queue
  */
  uint8_t available(const uint8_t q) const {
    if(q == 0) {
      return ( _buffer.size() - _buffer.count() );
    }
    else if (q <= INTERRUPT_COUNT) {
      return ( INTERRUPT_LENGTH - _interrupts[q-1].count() );
    }
    else {
      return 0;
    }
  }

  /*!
  * @brief 
  * @param q the queue to access. q == 0 is the main buffer. 
  * q == 1 : INTERRUPT_COUNT is an interrupt vector
  * if q > INTERRUPT_COUNT, returns 0
  * @return the capacity of the queue
  * 
  */
  uint8_t capacity(const uint8_t q) const {
    if(q == 0) {
      return _buffer.size();
    }
    else if(q <= INTERRUPT_COUNT) {
      return INTERRUPT_LENGTH;
    }
    else {
      return 0;
    }
  }

  bool queueCommand(const uint8_t queue, const CommandData data) {
    if(queue == 0) {
      // return true if there was room and the command was buffered
      return (_buffer.put(data) == 0 ? true : false);
    }
    else if(queue <= INTERRUPT_COUNT) {
      //Serial.print("attempting to enqueue to: ");
      //Serial.println(q);
      return _interrupts[queue-1].put(data);
    }
    else {
      return false;
    }
  }

  /*!
  * @brief start an interrupt. If v is a valid interrupt vector, 
  * the current command and the current time are saved
  * and the interrupt vector is run
  * @param v the vector to start. 0 < v <= INTERRUPT_COUNT
  * @return true if the interrupt was successfully started
  */
  bool triggerInterrupt(const uint8_t v) {
    // 0 is the ring buffer
    if(v == 0) return false;
    // 0 < v <= INTERRUPT_COUNT
    if(v <= INTERRUPT_COUNT) {
       CommandData next_cmd; 
      if( _interrupts[v-1].next(&next_cmd) == 0 ) {
        _last_elapsed = currentCommandElapsedTime();
        _waiting = _current;
        _last = _current;
        _current = next_cmd;
        _last_command_start_time = LT_current_time_us;
        _last_vector = _vector;
        _vector = v;
        updateCommand();
        return true;
      }
    }
    return false;
  }

  /*!
  * @brief attempts to run the next command in the current
  * interrupt vector.
   */
  void runInterrupt() {
    CommandData next_cmd; 
    if( _interrupts[_vector-1].next(&next_cmd) == 0 ) {
      _last = _current; // the previous command has been saved in last and waiting 
      _current = next_cmd;
      _last_command_start_time = LT_current_time_us;
      updateCommand();
    }
    else {
      // the interrupt is done. resume the buffered process
      finishInterrupt();
    }
  }

  /*!
  * @brief cleans up after an interrupt vector has finished
  * (has no more commands).
   */
  void finishInterrupt() {
    _interrupts[_vector-1].reset();
    _last = _current;
    _current = _waiting;
    _last_command_start_time = (LT_current_time_us - _last_elapsed);
    uint8_t temp = _vector;
    _vector = _last_vector;
    _last_vector = temp;
    updateCommand();
  }

  void setRunning(const bool isRunning) {
    if(isRunning) {
      _last_command_start_time += (LT_current_time_us - _pause_time);
      DPRINTF("Protocol started. Time elapsed");
      DPRINTLN(LT_current_time_us - _last_command_start_time);
    }
    else {
      _pause_time = LT_current_time_us;
    }
    _running = isRunning;
  }

  const bool isRunning() const {return _running;}

  void reset() {
    _buffer.reset();
    for(uint8_t i = 0; i<INTERRUPT_COUNT; ++i) {
      _interrupts[i].reset();
    }
    _running = false; 
    _vector = 0; 
    _last_vector = 0;
    _pause_time = 0; 
    _last_command_start_time = 0;
    _last_elapsed = 0;
    _current = CommandData();
    _last = CommandData();
  }
};

// global external variable to keep track of time
extern uint32_t LT_current_time_us;

#endif // End __PROCESS_MANAGER_H__ include guard
