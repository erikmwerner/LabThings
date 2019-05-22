#ifndef __PROCESS_MANAGER_H__
#define __MESSAGE_HANDLER_H__

#include "../utilities/ring_buffer.h"

typedef void (*intCallback)(int);

struct CommandData{
  uint32_t duration;
  uint32_t data;
  int16_t index;
  int8_t code;
  int8_t target;

  CommandData(
    int8_t code = -1,
    int8_t target = -1,
    int16_t index = -1,
    uint32_t data = 0,
    uint32_t duration = 0
  ):
    code(code),
    target(target),
    index(index),
    data(data),
    duration(duration)
  {}
};

template <uint8_t LENGTH>
class ProcessVector {
  uint8_t _count;
  uint8_t _next;
  CommandData _v[LENGTH];
public: 
  void reset(){
    memset (this, 0, sizeof (*this));
  }
  bool put(const CommandData data) {
    if(_count < LENGTH) {
      _v[_count++] = data;
      return true;
    }
    else {
      return false;
    }
  }
  CommandData* next() {
    if(_next < LENGTH) {
      return &_v[_next++];
    }
    else {
      return nullptr;
    }
  }
};

template<uint8_t BUFFER_LENGTH, uint8_t INTERRUPT_COUNT, uint8_t INTERRUPT_LENGTH>
class ProcessManager {

  RingBuffer<BUFFER_LENGTH, CommandData> _buffer;
  CommandData _current;
  CommandData _waiting;
  ProcessVector<INTERRUPT_LENGTH> _interrupts[INTERRUPT_COUNT];
  bool _running = false; ///< flag to track if the protocol is currrently running
  uint8_t _interrupted = 0; ///< flag to track if the protocol is interrupted. 0 is false. >0 is the vector -1
  uint32_t _pause_time = 0; ///< the system time in microseconds that the protocol was paused
  uint32_t _last_command_start_time = 0; ///< the system time in microseconds that the last command was started
  uint32_t _interrupt_start_time = 0;

  intCallback _process_changed_callback = nullptr;
  intCallback _process_ended_callback = nullptr;

public:
  void update() {
    // do nothing if the process is stopped
    if(!_running) return;
    
    int32_t dt = LT_current_time_us - _last_command_start_time;

    if(_current.duration <= dt) {
      // run interrupt if necessary
      if(_interrupted > 0) {
        if(_interrupts[_interrupted-1].next() != nullptr) {
          // continue with the interrupt process
          _current = *_interrupts[_interrupted-1].next();
          updateCommand();
          _last_command_start_time = LT_current_time_us;
        }
        else {
          // the interrupt is done. resume the buffered process
          _interrupted = 0;
          _current = _waiting;
          updateCommand();
          _last_command_start_time += (LT_current_time_us - _interrupt_start_time);
          //Serial.println("INTERRUPT COMPLETED");
        }
      }
      else {
        if(_buffer.takeBack(&_current) == 0) {
          updateCommand();
          _last_command_start_time = LT_current_time_us;
          /*
         // uint32_t t_r = _current._duration - dt;
          uint32_t t_r = LT_current_time_us - (_last_command_start_time + _current._duration);
          //uint32_t t_r = dt - _current._duration;

          Serial.print("TIME REMAIING:");
          Serial.println(t_r);

          Serial.print("COMMAND STARTED! Duration=");
          Serial.print(_current._duration);
          Serial.print("START TIME=");
          Serial.print(_last_command_start_time);
          Serial.print("TIME REMAINING=");
          Serial.println(t_r);*/
        }
        else {
          // there are no more commands in the buffer. stop the protocol
          _running = false;
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

  inline void updateCommand() {
    if(_process_changed_callback != nullptr) {
        (_process_changed_callback)(_current.code);
    }
  }

  uint32_t remainingTime() {
    _last_command_start_time + _current.duration - LT_current_time_us;
  }

  void setProcessChangedCallback( intCallback f ) {
      _process_changed_callback = f;
  }

  void setProcessEndedCallback( intCallback f ) {
      _process_ended_callback = f;
  }

  uint8_t available() const {
    return ( _buffer.size() - _buffer.count() );
  }

  uint8_t capacity() const {
    return _buffer.size();
  }

  bool queueCommand(const uint8_t q, const CommandData data) {
    if(q == 0) {
      // return true if there was room and the command was buffered
      return (_buffer.put(data) == 0 ? true : false);
    }
    else if(q <= INTERRUPT_COUNT) {
      return _interrupts[q-1].put(data);
    }
    else {
      return false;
    }
  }

  bool triggerInterrupt(const uint8_t v) {
    if(v <= INTERRUPT_COUNT) {
      if(_interrupts[v-1].next) {
        _interrupt_start_time = LT_current_time_us;
        _interrupted = v;
        _waiting = _current;
        &_current = _interrupts[v-1].next();
        _last_command_start_time = LT_current_time_us;
        return true;
      }
    }
    return false;
  }

  void setRunning(const bool isRunning) {
    if(isRunning) {
      _last_command_start_time += (LT_current_time_us - _pause_time);
      /*Serial.print("START_TIME:");
      Serial.println(_last_command_start_time);*/
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
    memset (this, 0, sizeof (*this));
  }
};

// bring in a global external variable to keep track of time
extern uint32_t LT_current_time_us;

#endif // End __PROCESS_MANAGER_H__ include guard
