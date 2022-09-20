#ifndef HDLC_H
#define HDLC_H

#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC bool hdlcSendBuffer(void const * const buffer, size_t const bufferSize);
EXTERNC bool hdlcReceiveBuffer(void *const buffer, size_t const bufferSize);

#undef EXTERNC

#endif
