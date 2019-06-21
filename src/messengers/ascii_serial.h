#ifndef __ASCII_SERIAL_H__
#define __ASCII_SERIAL_H__

// Lab Things ASCII Serial Protocol
// Transmit data in a simple and human-readable form

// Messages are sent in ASCII form between start and end characters.
// When a start character is received, all previous message data is discarded
// If message length exceeds MAX_MESSAGE_LENGTH, all message data is discarded
// When an end character is received, all stored message data is processed
// If a start character is not the first character in the message, all message data is discarded

// The default message delimiters are listed below, but can be changed in the constructor
//                                Decimal    Hex    ASCII
// start of transmission          60        3C      <
// field separator                44        2C      ,
// end of transmission            62        3E      >

// Example message from the master to the slave:
// < Message ID, Value , ... , Value >

// Example message from the slave to the master:
// < Message ID, Value, ... , Value >

#define MAX_MESSAGE_LENGTH 64

typedef void(*intCallback) (int);

class ASCIISerial {
    Stream* _com = nullptr;
    const char _delimiters[4]; // default is {'<', ',', '>',0};
    char _message[MAX_MESSAGE_LENGTH];
    volatile uint8_t _msg_idx = 0;
    volatile bool _pending_msg = false;
    intCallback _rx_callback = nullptr;
    intCallback _tx_callback = nullptr;

    void handleByte(char data) {
      if ( (char)data == _delimiters[0] ) {
        resetMessage();
      }
      _message[_msg_idx++] = (char)data;
      if ( (char)data == _delimiters[2] ) {
        // got a message?
        if ( _message[0] == _delimiters[0] ) {
          if (_rx_callback != nullptr) {
            //handle the message
            char* token = strtok(_message, _delimiters);
            if(token) {
              int cmd = atoi(token);
              (*_rx_callback)(cmd);
            }
          }
          resetMessage();
        }
        else {
          resetMessage();
        }
      }
      if (_msg_idx > MAX_MESSAGE_LENGTH) {
        resetMessage();
        _com->println("Error: max message size exceeded");
      }
    }

    void resetMessage() {
      _msg_idx = 0;
      _pending_msg = false;
      memset(_message, 0, sizeof(_message));
    }

  public:
    ASCIISerial(Stream &s, const char som = '<', const char sep = ',', const char eom = '>')
    :_com(&s), _delimiters( {som, sep, eom, 0} ) {}

    void setMessageReceivedCallback(intCallback c) {
      _rx_callback = c;
    }
    void setMessageSentCallback(intCallback c) {
      _tx_callback = c;
    }
    
    void update() {
      // check for new data
      while (_com->available()) {
        handleByte(_com->read());
      }
    }
    bool hasPendingMessage() {
      return _pending_msg;
    }
    int getNextArgInt() {
      char* token = strtok(NULL, _delimiters);
      return strtol(token, NULL, 10);
    }
    long getNextArgLong() {
      char* token = strtok(NULL, _delimiters);
      return strtol(token, NULL, 10);
    }
    unsigned long getNextArgULong() {
      char* token = strtok(NULL, _delimiters);
      return strtoul(token, NULL, 10);
    }
    double getNextArgDouble() {
      char* token = strtok(NULL, _delimiters);
      return strtod(token, NULL);
    }
    char* getNextArgChar() { 
      char* token = strtok(NULL, _delimiters);
      uint8_t copy_size = strlen(token) + 1;
      char arg[copy_size];
      strncpy(arg, token, copy_size);
      return arg;
    }
};
#endif //End __ASCII_SERIAL_H__ include guard
