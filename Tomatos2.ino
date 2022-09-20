#include "twiInterface.h"
#include <Wire.h>
#include "hdlc.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);
  twiInitialize();
}

void loop() {
  // put your main code here, to run repeatedly:
  TelemetryCommand cmd;

  while(true) {
    cmd.cmdId = 0;
    cmd.cmdTag = counter &0xff;
    cmd.parameter = counter;

    Serial.print("Ping ");
    Serial.print(counter);
    Serial.print(" ");
    Wire.beginTransmission(1);
    hdlcSendBuffer(&cmd, sizeof(cmd));
    Wire.endTransmission();
    Serial.print("...");
    delay(10);

    if(Wire.requestFrom(1,16) > 0 && hdlcReceiveBuffer(&cmd, sizeof(cmd))) {
          Serial.print(" Pong ");
          Serial.print(cmd.parameter);
    } else {
      Serial.print("No Response");
    }
    Serial.print("\n");
    ++counter;
    delay(1000);
  }
}
