#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC uint16_t computeCrc(uint8_t const* buffer, size_t bufferSize);

#undef EXTERNC

#endif
