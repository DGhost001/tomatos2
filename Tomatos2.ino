#include "twiInterface.h"
#include "tmTcInterface.h"
#include <Wire.h>
#include "hdlc.h"
#include "LowPower.h"

enum ClientTypes{
  cTemperature = 2,
  cHumidity    = 1,
  cPump        = 4
};


struct Clients
{
    ClientAddress address;
    uint8_t types;
    uint8_t firmwareVersion;
    uint16_t temperature;
    uint16_t humidity;
};

static Clients availableClients[128];
static uint8_t foundClients;

static void scanBus( void )
{
  foundClients = 0;
  for(unsigned index = 1; index < 128; ++index) {
    ClientAddress const address =  static_cast<ClientAddress>(index);
    uint16_t parameter = 0x55aa;
    if(sendCommand(address, Commands::cmdPing, parameter)) {
        if(0x55aa == parameter) {
            if(sendCommand(address, Commands::cmdRequestId, parameter))
            {
                availableClients[foundClients].address = address;
                availableClients[foundClients].types = parameter & 0xff;
                availableClients[foundClients].firmwareVersion = (parameter >> 8) & 0xff;
                availableClients[foundClients].temperature = 0;
                availableClients[foundClients].humidity = 0;
                ++foundClients;
            }
        }
    }
  }
}

static void updateMeasurements( void )
{
  for(unsigned index = 0; index < foundClients; ++index) {
    uint16_t parameter = 0x55aa;
    ClientAddress const address = availableClients[index].address;
    if(sendCommand(address, Commands::cmdPing, parameter) && 0x55aa == parameter){
      if(availableClients[index].types & cTemperature) {
        sendCommand(address, Commands::cmdRequestTemperatureMeasurement, availableClients[index].temperature);
      }
      if(availableClients[index].types & cHumidity) {
        sendCommand(address, Commands::cmdRequestMoistureMeasurement, availableClients[index].humidity);
      }     
    }
  }    
}

static float convertTemperature(uint16_t const value)
{
    float const k = 1.0/16.0;
    float const offset = -290; //Offset at 21 deg
    float result = (value * k + offset) + 21;

    return result;
}

static void printMeasurement(unsigned const index)
{
  Serial.print("{\"Sensor\":"); Serial.print(availableClients[index].address); Serial.print(",");
  Serial.print("\"Firmware\":"); Serial.print(availableClients[index].firmwareVersion); Serial.print(",");
  Serial.print("\"hasTemperature\":");
  if(availableClients[index].types & cTemperature) {
    Serial.print("true,\"Temperature\":"); Serial.print(convertTemperature(availableClients[index].temperature));
  } else {
    Serial.print("false");
  }
  Serial.print(",\"hasMoisture\":");  
  if(availableClients[index].types & cHumidity) {
    Serial.print("true,\"Moisture\":"); Serial.print(availableClients[index].humidity);
  }else {
    Serial.print("false");
  }
  Serial.print("}");
}

int readAnalgoValue() 
{
  ADCSRA |= _BV(ADSC); // start the conversion
  while (bit_is_set(ADCSRA, ADSC)); // ADSC is cleared when the conversion finishes

  int const result = (ADCL | (ADCH << 8)); // combine bytes & correct for temp offset (approximate)} 

  return result;
}

float getTemperature()
{
  ADMUX = 0xC8; // turn on internal reference, right-shift ADC buffer, ADC channel = internal temp sensor
  delay(10);
  int temp = 0;
  for(int i=0; i<16; ++i) temp+=readAnalgoValue();

  return (temp/16.0)-356 + 21;
}

uint16_t getCurrentSensor()
{
  ADMUX = 0x40; // turn on internal reference, right-shift ADC buffer, ADC channel = internal temp sensor
  delay(10);

  int result = 0;
  for(int i = 0; i<16; ++i) {
    result += readAnalgoValue();   
  }

  return result;
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);
  twiInitialize();

  Serial.print("Searching for clients...");
  scanBus();
  Serial.print(" found ");
  Serial.println(foundClients);

  ADMUX = 0xC8; // turn on internal reference, right-shift ADC buffer, ADC channel = internal temp sensor
  delay(10);  // wait a sec for the analog reference to stabilize
}

void loop() {
  static unsigned sequence = 0;
  ++sequence;
  updateMeasurements();
  Serial.print("\30{");
  Serial.print("\"sequence\":"); Serial.print(sequence);
  Serial.print(",\"temperature\":");
  Serial.print(getTemperature());
  Serial.print(",\"current\":");
  Serial.print(getCurrentSensor());   
  Serial.print(",\"Sensors\":["); 
  for(unsigned index = 0; index < foundClients; ++index)
  {
    printMeasurement(index);
    if(index<(foundClients-1)) {
      Serial.print(",");
    } 
  }
  Serial.print("]");
  Serial.println("}");

  Serial.flush();
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_ON);
}
