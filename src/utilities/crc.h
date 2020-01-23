#ifndef __CRC_H__
#define __CRC_H__

const uint32_t crc_table[16] = { ///<  16-entry (64 byte) LUT calculated by pycrc to speed up calculations
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c};

static uint32_t crc32(uint8_t const *buffer, const uint8_t len)
{
  uint32_t crc = 0xFFFFFFFF;
  for (uint8_t index = 0; index < len; ++index)
  {
    crc = crc_table[(crc ^ buffer[index]) & 0x0F] ^ (crc >> 4);
    crc = crc_table[(crc ^ (buffer[index] >> 4)) & 0x0F] ^ (crc >> 4);
    crc = ~crc;
  }
  //Serial.print("SENT CHECKSUM:");
  //Serial.println(crc);
  return crc;
}

#endif //End __CRC_H__ include guard
