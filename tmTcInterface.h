#ifndef TM_TC_INTERFACE
#define TM_TC_INTERFACE

#include <stdbool.h>
#include <stdint.h>

typedef uint8_t ClientAddress;
enum class Commands {
  cmdPing = 0,
  cmdRequestId = 1,
  cmdRequestMoistureMeasurement=2,
  cmdRequestTemperatureMeasurement=3,
  cmdUpdateClientAddress=4
};


/** This function send a command to the client
  * @param address This is the clients address
  * @param cmdId This is the command to be send
  * @param parameter This is the parameter for the command. If the command was successful the parameter will be updated
  * @retval true The Command was executed successfully and the parameter was updated (Received a response from client)
  * @retval false The Command was not executed successfully (no response from client / wrong response)
  */  
bool sendCommand(ClientAddress const address, Commands const cmdId, uint16_t &parameter);

#endif

