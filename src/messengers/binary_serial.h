#ifndef __BINARY_SERIAL_H__
#define __BINARY_SERIAL_H__

// Advice: https://stackoverflow.com/questions/1445387/how-do-you-design-a-serial-command-protocol-for-an-embedded-system
// See also CRC-8-CCITT: http://www.nongnu.org/avr-libc/user-manual/group__util__crc.html#gab27eaaef6d7fd096bd7d57bf3f9ba083
// https://lentz.com.au/blog/calculating-crc-with-a-tiny-32-entry-lookup-table

#include "../utilities/crc.h"

#define MAX_MESSAGE_LENGTH 64 //< size of the incoming message buffer. Must be smaller than 256 [sizeof(uint8_t)]

typedef void (*intCallback)(int);

/*! BinarySerial is a Lab Things messenger class for transmitting larger amounts of data efficiently and reliably.
* Data are encoded using Serial Line IP (SLIP) packet framing (RFC 1055) with a 32-bit checksum
* format: <END><message><crc><END>
*
* @sa ASCIISerial
*/
class BinarySerial
{
  const uint8_t END = 0xC0;     ///<  Frame end
  const uint8_t ESC = 0xDB;     ///<  Frame escape
  const uint8_t ESC_END = 0xDC; ///<  Transposed frame end
  const uint8_t ESC_ESC = 0xDD; ///<  Transposed frame escape

  /*!
   * @brief an enumeration to define types of error messages that can be returned by the BinarySerial class.
   */
  enum ERROR : uint8_t
  {
    INVALID_CHECKSUM = 0,    //< the value from the last four bytes of the message did not match the checksum for the message
    MAX_LENGTH_EXCEEDED = 1, //< the incoming message buffer was filled and data was overwritten
    SLIP_VIOLATION = 2,      //< recieved data was not formatted according to the SLIP protocol
    MESSAGE_FORMAT = 3       //< recieved data was empty or did not contain a formatted message
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
    // _message contains a packet of length _msg_idx
    if (_msg_idx > 4)
    {
      // read the last four bytes
      uint32_t checksum = 0;
      read(&_message[_msg_idx - 4], checksum);
      // check the crc
      if (checksum == crc32(_message, _msg_idx - 4))
      {
        if (_rx_callback != nullptr)
        {
          // handle the message with the callback functin
          // msg_idx is the number of bytes in the packet, including the checksum bytes if applicable
          (*_rx_callback)(_msg_idx);
        }
      }
      else
      {
        //bad checksum
        if (_error_callback != nullptr)
        {
          (*_error_callback)(INVALID_CHECKSUM);
        }
      }
    }
    else
    {
      // message is less than 5 bytes long
      // there is no data in the packet, ignore it.
      // can optionally send a message format error
      /*if (_error_callback != nullptr) {
        (*_error_callback)(MESSAGE_FORMAT);
      }*/
    }
    _msg_idx = 0;
  }
  /*!
  * @brief Encode and write a byte to the communication stream.
  * Data is sent using SLIP encoding.
  */
  inline void encode(const uint8_t value)
  {
    if (value == END)
    {
      // if value is the same code as an END character, send a
      // special two character code so as not to make the
      // receiver think we sent an END
      _com->write(ESC);
      _com->write(ESC_END);
    }
    else if (value == ESC)
    {
      // if value is the same code as an ESC character,
      // send a special two character code so as not
      // to make the receiver think we sent an ESC
      _com->write(ESC);
      _com->write(ESC_ESC);
    }
    else
    {
      // otherwise, just send the character
      _com->write(value);
    }
  }

public:
  BinarySerial(Stream &s) : _com(&s) {}

  void setMessageReceivedCallback(intCallback c) { _rx_callback = c; }
  void setMessageSentCallback(intCallback c) { _tx_callback = c; }
  void setMessageErrorCallback(intCallback c) { _error_callback = c; }

  /*!
  * @brief Access the buffer containing most recently received message
  * This data is only valid in the callback function. Message data will
  * be overwritten as new data is received
  * @return a pointer to the first byte in the message buffer
  */
  uint8_t* messageData() { return _message; }

  /*!
  * @brief The number of bytes in the current message
  * @return the number of bytes in the message buffer.
  * Bytes from index 0 to messageLength() - 1 are valid data.
  */
  uint8_t messageLength() const { return _msg_idx; }

  /*!
  * @brief send a packet
  * @param the address of the first byte the send
  * @param the number of bytes to send
  */
  void sendPacket(uint8_t *packet, uint8_t len)
  {
    // calculate a checksum for the packet
    uint32_t checksum = crc32(packet, len);

    // send an END character to flush out any data that may
    // have accumulated in the receiver due to line noise
    _com->write(END);

    // for each byte in the packet, send the appropriate character sequence
    for (uint8_t i = 0; i < len; ++i)
    {
      encode(*packet++);
    }
    // send a four byte checksum
    uint8_t *p = (uint8_t *)&checksum;
    encode(p[0]);
    encode(p[1]);
    encode(p[2]);
    encode(p[3]);
    // tell the receiver the packet is done
    _com->write(END);
  }

  /*!
  * @brief checks the communication stream for new data. If data can be read,
  * it is decoded and stored in the incoming message buffer. When an unescaped
  * END byte is recieved, the message is handled.
  */
  void update()
  {
    // check for rx data
    if (_com->available())
    {
      uint8_t next_byte = _com->read();
      //Serial.print("got byte: ");
      //Serial.print(next_byte, HEX);
      //Serial.println("!");
      if (_escaped)
      {
        // handle escaped condition
        // if the byte is not one of these, there is
        // a protocol violation.  The best bet
        // seems to be to leave the byte alone and
        // just stuff it into the packet
        if (next_byte == ESC_END)
        {
          next_byte = END;
        }
        else if (next_byte == ESC_ESC)
        {
          next_byte = ESC;
        }
        else 
        {
          // don't modify next_byte
          // send error message
          if (_error_callback != nullptr)
          {
            (*_error_callback)(SLIP_VIOLATION);
          }
        }
        _escaped = false;
      }

      else if (next_byte == END)
      {
        // if it's an END character then we're done with the packet
        if (_msg_idx)
        {
          handleMessage();
          return;
        }
        else
        {
          return;
        }
      }
      else if (next_byte == ESC)
      {
        // if it's an ESC character, set escaped flag and wait
        // for another character
        _escaped = true;
        return;
      }
      // finally, store the character in the message buffer
      if (_msg_idx < MAX_MESSAGE_LENGTH)
      {
        _message[_msg_idx++] = next_byte;
      }
      else
      {
        // max packet length exceeded
        _msg_idx = 0; // start writing data to the beginning of the buffer again
        if (_error_callback != nullptr)
        {
          (*_error_callback)(MAX_LENGTH_EXCEEDED);
        }
      }
    }
  }
  
  /*!
  * @brief writes value into a buffer
  * @param packet the buffer to write into
  * @param value the value to be written
  * @return the number of bytes written
  */
  template <typename T>
  uint8_t write(uint8_t *packet, const T &value)
  {
    const uint8_t *p = (const uint8_t *)&value;
    uint8_t i;
    for (i = 0; i < sizeof(value); ++i)
    {
      packet[i] = p[i];
    }
    return i;
  }

  /*!
  * @brief read a value from a buffer into value
  * @param packet the buffer to read from
  * @param value the value to write into
  * @return the number of bytes read
  */
  template <typename T>
  uint8_t read(uint8_t *packet, T &value)
  {
    value = *(T *)(packet);
    return sizeof(value);
  }

  /*!
   * @brief creates and sends a packet that a message
   * containing the function code was received
   * 
   * @param code the function code to acknowledge
   */
  void sendAcknowledge(LT::FN_CODE code)
  {
    uint8_t packet[2] = {LT::Acknowledge, code};
    sendPacket(packet, 2);
  }

  /*!
   * @brief creates and sends a packet that a message
   * with the fucntion code was received, but caused an error. This 
   * can be useful for tracking down an error to a specific callback function.
   * 
   * @param code the function code that caused an error
   */
  void sendError(LT::FN_CODE code)
  {
    uint8_t packet[] = {LT::Error, code};
    sendPacket(packet, 2);
  }

  /*!
  * @brief overloaded sendError function to send error messages
  * with additional error codes.
  * @param codes
  * @param len
  */
  void sendError(uint8_t *codes, uint8_t len)
  {
    uint8_t packet[1 + len];
    packet[0] = LT::Error;
    for (uint8_t i = 0; i < len; ++i)
    {
      packet[i + 1] = codes[i];
    }
    sendPacket(packet, len + 1);
  }
};

#endif //End __BINARY_SERIAL_H__ include guard
