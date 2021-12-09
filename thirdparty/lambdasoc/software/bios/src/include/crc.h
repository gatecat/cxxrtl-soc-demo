#ifndef __CRC_H
#define __CRC_H

#include <stddef.h>
#include <stdint.h>

uint16_t crc16(const uint8_t *buffer, size_t len);
uint32_t crc32(const uint8_t *buffer, size_t len);

#endif
