#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include <stdint.h>

// trick to deduce template argument type from:
// https://stackoverflow.com/questions/7038797/automatically-pick-a-variable-type-big-enough-to-hold-a-specified-number
template<unsigned long long Max>
struct RequiredBits
{
    enum { value =
        Max <= 0xff       ?  8 :
        Max <= 0xffff     ? 16 :
        Max <= 0xffffffff ? 32 :
                            64
    };
};

template<int bits> struct SelectInteger_;
template<> struct SelectInteger_ <8> { typedef uint8_t type; };
template<> struct SelectInteger_<16> { typedef uint16_t type; };
template<> struct SelectInteger_<32> { typedef uint32_t type; };
template<> struct SelectInteger_<64> { typedef uint64_t type; };

template<unsigned long long Max>
struct SelectInteger : SelectInteger_<RequiredBits<Max>::value> {};

// BUFFER_LENGTH must be a power of 2
// if (((size - 1) & size) == 0)
// uses a binary trick to calculate index offsets and avoid modulus
// offset = (_head & (BUFFER_LENGTH - 1)); // equals 0 when head = length
// BUFFER_LENGTH must be a power of 2
// from: http://www.simplyembedded.org/tutorials/interrupt-free-ring-buffer/

template <int BUFFER_LENGTH, class T, class TS = typename SelectInteger<BUFFER_LENGTH>::type>
//template < uint8_t BUFFER_LENGTH , class T >
class RingBuffer {
    //uint8_t _elements = BUFFER_LENGTH;
    TS _head = 0;
    TS _tail = 0;
    T _buffer[BUFFER_LENGTH];// = {0};

public:

    RingBuffer() {
      static_assert((BUFFER_LENGTH & (BUFFER_LENGTH - 1)) == 0,
       "Buffer length must be a power of 2");
    }
    
    int8_t replace(T data, TS index) {
        if(index < BUFFER_LENGTH) {
            const TS offset = ( (_tail+index) & (BUFFER_LENGTH - 1));
            _buffer[offset] = data;
            return 0;
        }
        else {
            return -1;
        }
    }

    // gets data from the specified index
    int8_t get(const TS index, T *data) {
        if(index < BUFFER_LENGTH && ( !isEmpty() )) {
            const TS offset = ( (_tail+index) & (BUFFER_LENGTH - 1));
            *data = _buffer[offset];
            return 0;
        }
        else {
            // error index OOB
            return -1;
        }
    }

    // adds data at the head if there is room
    int8_t put(const T data) {
        if(!isFull()){
            const TS offset = (_head & (BUFFER_LENGTH - 1));
            _buffer[offset] = data;
            _head++;
            return 0;
        }
        else {
            return -1;
        }
    }

    // add data at the head
    // if buffer is full, make room by taking the last element
    int8_t push(const T data, T*taken) {
        if(!isFull()){
            put(data);
            return 0;
        }
        else {
            takeBack(taken);
            put(data);
            return -1;
        }
    }

    // reads and removes data at the tail
    int8_t takeBack(T *data) {
        if(last(data) == 0) {
          _tail++;
          return 0;
        }
        else {
          return -1;
        }
    }
    
    int8_t first(T *data) {
      if(!isEmpty() ) {
            const TS offset = (_head - 1 & (BUFFER_LENGTH - 1));
            *data = _buffer[offset];
            return 0;
        }
        else {
            return -1;
        }
    }
    
    int8_t last(T *data) {
       if(!isEmpty() ) {
            const TS offset = (_tail & (BUFFER_LENGTH - 1));
            *data = _buffer[offset];
            return 0;
        }
        else {
            return -1;
        }
    }
    
    TS size() { return BUFFER_LENGTH;}
    TS count() {return (_head - _tail);}

    bool isFull() {
        return count() == size() ? true : false;
    }
    bool isEmpty() {
        return count() == 0 ? true : false;
    }

};

#endif // End __RING_BUFFER_H__ include guard
