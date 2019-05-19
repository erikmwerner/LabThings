#ifndef __BINARY_SERIAL_H__
#define __BINARY_SERIAL_H__

// Lab Things Binary Serial Protocol
// Transmit larger amounts of data more efficiently and reliably
// Uses Serial Line IP (SLIP) packet framing (RFC 1055) with a 32-bit checksum

// Advice: https://stackoverflow.com/questions/1445387/how-do-you-design-a-serial-command-protocol-for-an-embedded-system

// <END>
// <message>
// <crc>
// <END>

#define MAX_MESSAGE_LENGTH 64

typedef void (*intCallback)(int);

const uint32_t crc_table[16] = { ///<  16-entry LUT calculated by pycrc to speed up calculations
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c};
/*!
*
*/
class BinarySerial
{
  const uint8_t END = 0xC0;     ///<  Frame end
  const uint8_t ESC = 0xDB;     ///<  Frame escape
  const uint8_t ESC_END = 0xDC; ///<  Transposed frame end
  const uint8_t ESC_ESC = 0xDD; ///<  Transposed frame escape

  enum ERROR : uint8_t
  {
    INVALID_CHECKSUM = 0,
    MAX_LENGTH_EXCEEDED = 1,
    SLIP_VIOLATION = 2,
    MESSAGE_FORMAT = 3
  };

  Stream *_com = nullptr;
  intCallback _rx_callback = nullptr;
  intCallback _tx_callback = nullptr;
  intCallback _error_callback = nullptr;

  uint8_t _message[MAX_MESSAGE_LENGTH];
  volatile uint8_t _msg_idx = 0;
  volatile bool _escaped = false;

  /*!
  * @brief executes the message recieved callback if a message with more than 4 bytes was received
  * the message stored in the buffer is guaranteed to have at least a 1 byte payload (plus the 4-byte checksum)
  */
  void handleMessage()
  {
    if (_msg_idx > 4) {
      // we have a packed of length _msg_idx
      //check the crc
      uint32_t checksum = _message[_msg_idx - 4];
      checksum |= ((uint32_t)_message[_msg_idx - 3]) << 8 & 0xff00;
      checksum |= ((uint32_t)_message[_msg_idx - 2]) << 16 & 0xff0000;
      checksum |= ((uint32_t)_message[_msg_idx - 1]) << 24 & 0xff000000;
      if (checksum == crc32(_message, _msg_idx - 4)) {
        if (_rx_callback != nullptr) {
          //handle the message with the callback functin
          // msg_idx is the number of bytes in the packet, including the checksum bytes if applicable
          (*_rx_callback)(_msg_idx);
        }
      }
      else {
        //bad checksum
        if (_error_callback != nullptr) {
          (*_error_callback)(INVALID_CHECKSUM);
        }
      }
    }
    else {
      // if there is no data in the packet, ignore it.
      /*if (_error_callback != nullptr) {
        (*_error_callback)(EMPTY_PACKET);
      }*/
    }
    _msg_idx = 0;
  }

  /*static*/ uint32_t crc32(uint8_t const *buffer, const uint8_t len)
  {
    //uint32_t crc = ~0L;
    uint32_t crc = 0xFFFFFFFF;
    for (uint8_t index = 0; index < len; ++index) {
      crc = crc_table[(crc ^ buffer[index]) & 0x0f] ^ (crc >> 4);
      crc = crc_table[(crc ^ (buffer[index] >> 4)) & 0x0f] ^ (crc >> 4);
      crc = ~crc;
    }
    /*Serial.print("MSG IDX: ");
    Serial.print(_msg_idx);
    for(int i = 0; i < _msg_idx; ++i){
      Serial.print(" DATA AT [");
      Serial.print(i);
      Serial.print("]: ");
      Serial.print(_message[i]);
    }
    Serial.print(" CRC IS:");
    Serial.println(crc);*/
    return crc;
  }

public:
  BinarySerial(Stream &s) : _com(&s) {}

  void setMessageReceivedCallback(intCallback c){_rx_callback = c; }
  void setMessageSentCallback(intCallback c){ _tx_callback = c; }
  void setMessageErrorCallback(intCallback c){ _error_callback = c; }

  /*!
  * @brief Access the last message received
  * 
  */
  uint8_t* messageData() { return _message; }

  uint8_t messageLength() { return _msg_idx; }

  inline void encode(const uint8_t value)
  {
    if (value == END) {
      //if it's the same code as an END character, send a
      // special two character code so as not to make the
      // receiver think we sent an END
      _com->write(ESC);
      _com->write(ESC_END);
    }
    else if (value == ESC) {
      // if it's the same code as an ESC character,
      // we send a special two character code so as not
      // to make the receiver think we sent an ESC
      _com->write(ESC);
      _com->write(ESC_ESC);
    }
    else {
      // otherwise, just send the character
      _com->write(value);
    }
  }
  /*!
  * @brief send a 
  */
  void sendPacket(uint8_t *packet, uint8_t len)
  {
    // calculate a checksum for the packet
    uint32_t checksum = crc32(packet, len);

    // send an END character to flush out any data that may
    // have accumulated in the receiver due to line noise
    _com->write(END);

    // for each byte in the packet, send the appropriate character sequence
    for(uint8_t i = 0; i < len; ++i) {
        encode(*packet);
        ++packet;
    }

    /*while (len--)
    {
      encode(*packet);
      ++packet;
    }*/
    uint8_t *p = (uint8_t*)&checksum;
    encode(p[0]);
    encode(p[1]);
    encode(p[2]);
    encode(p[3]);
    //encode(checksum >> 8);
    //encode(checksum >> 16);
    //encode(checksum >> 24);
    // tell the receiver the packet is done
    _com->write(END);
  }

  void update()
  {
    // check for rx data
    if (_com->available()) {
      uint8_t next_byte = _com->read();
      //Serial.print("got byte: ");
      //Serial.print(next_byte, HEX);
      //Serial.println("!");
      if (_escaped) {
        // handle escaped condition
        // if the byte is not one of these, there is
        // a protocol violation.  The best bet
        // seems to be to leave the byte alone and
        // just stuff it into the packet
        if (next_byte == ESC_END) {
          next_byte = END;
        }
        else if (next_byte == ESC_ESC) {
          next_byte = ESC;
        }
        _escaped = false;
      }

      else if (next_byte == END) {
        // if it's an END character then we're done with the packet
        if(_msg_idx) {
                handleMessage();
                return;
            }
            else {
                return;
            }
      }
      else if (next_byte == ESC) {
        // if it's an ESC character, set escaped flag and wait
        // for another character
        _escaped = true;
        return;
      }
        // finally, store the character in the message buffer
        if (_msg_idx < MAX_MESSAGE_LENGTH) {
          _message[_msg_idx++] = next_byte;
        }
        else {
          // max packet length exceeded
          _msg_idx = 0;
          if (_error_callback != nullptr) {
            (*_error_callback)(MAX_LENGTH_EXCEEDED);
          }
        }
    }
  }

  // pack 16-bits
  template <typename T> 
  void pack16(uint8_t *packet, T input) {
    uint8_t *p = (uint8_t*)&input;
    packet[0] = p[0];
    packet[1] = p[1];

    //*packet =  (uint8_t)input;
    //++packet;
    //*packet = (uint8_t)(input >> 8);
  }

  // pack 32-bits
  template <typename T> 
  void pack32(uint8_t *packet, T input) {
    uint8_t *p = (uint8_t*)&input;
    packet[0] = p[0];
    packet[1] = p[1];
    packet[2] = p[2];
    packet[3] = p[3];
  }

  // get 16-bit value
  uint16_t get16(uint8_t *packet) {
    uint16_t value = (uint16_t)packet[0] | (uint16_t)packet[1] << 8;
    return value;
  }

  // get 32-bit value
  uint32_t get32(uint8_t *packet) {
    uint32_t value = (uint32_t)packet[0];
    value |= (uint32_t)packet[1] << 8;
    value |= (uint32_t)packet[2] << 16;
    value |= (uint32_t)packet[3] << 24;
    return value;
  }

  void sendAcknowledge(uint8_t code) {
    uint8_t packet[2] = {LT::Acknowledge, code};
    sendPacket(packet, 2);
  }

  void sendError(LT::FN_CODE code) {
    uint8_t packet[2];
    packet[0] = LT::Error;
    packet[1] = code;
    sendPacket(packet, 2);
  }

  /*!
  * @brief overloaded sendError function to send error messages
  * with additional error codes.
  *
  */
  void sendError(uint8_t *codes, uint8_t len) {
    uint8_t packet[1+len];
    packet[0] = LT::Error;
    for(uint8_t i = 0; i<len; ++i) {
      packet[i+1]= codes[i];
    }
    sendPacket(packet, len + 1);
  }
};

#endif //End __BINARY_SERIAL_H__ include guard
