#ifndef TWI_INTERFACE_H
#define TWI_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

/**
 * @brief twiSendChar will put the provided character into the sendbuffer
 * @return If character could be put into the send buffer
 */
EXTERNC bool twiSendChar(char);

/**
 * @brief twiCharAvailable retruns true if there is a character in the receive buffer
 * @return True - Character available
 */
EXTERNC bool twiCharAvailable( void );

/**
 * @brief twiReceiveChar Returns the received character, NULL if none was received
 * @return
 */
EXTERNC char twiReceiveChar( void );

/**
 * @brief twiInitialize Initializes the TWI Interface
 */
EXTERNC void twiInitialize( void );

/**
 * @brief twiSleep will enter IDLE, if USI allows it right now
 */
EXTERNC void twiSleep( void );

#undef EXTERNC

#endif
