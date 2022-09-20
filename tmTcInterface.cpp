#include <cstdint>
#include "tmTcInterface.h"
#include "hdlc.h"

#include <Wire.h>

#pragma pack(1)
struct TelemetryCommand {
    uint8_t cmdId;
    uint8_t cmdTag;
    uint16_t parameter;
};
#pragma pack()

static uint8_t commandCounter;

bool sendCommand(ClientAddress const address, Commands const cmd, uint16_t &parameter)
{
    bool result = false;
    TelemetryCommand cmd;

    cmd.cmdId = static_cast<int>(cmd);
    cmd.cmdTag = commandCounter;
    cmd.parameter = parameter;

    Wire.beginTransmission(address);
    hdlcSendBuffer(&cmd, sizeof(cmd));
    Wire.endTransmission();

    delay(10); //Allow the client to process the command

    //Request and decode the response from the client
    if(Wire.requestFrom(address,16) > 0 && hdlcReceiveBuffer(&cmd, sizeof(cmd)))
    {
        //If the decoded tag is equal to our requestm this is the response to our command
        if(cmd.cmdTag == commandCounter) {
            parameter = cmd.parameter;
            result = true;          
        }
    }

    return result;
}
