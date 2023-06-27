#include "twiInterface.h"
#include "tmTcInterface.h"
#include <Wire.h>
#include "hdlc.h"
//#include "LowPower.h"

#include <avr/wdt.h>

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

static uint16_t updateTemperatureMeasurement(ClientAddress const address)
{
  uint16_t a = 0,b = 0, count=5;

  sendCommand(address, Commands::cmdRequestTemperatureMeasurement, a);
  do {
    sendCommand(address, Commands::cmdRequestTemperatureMeasurement, a);
    sendCommand(address, Commands::cmdRequestTemperatureMeasurement, b);
    if(!(--count)) {
      return 0;
    }

  } while(abs(a-b)> 10);

  return b;
}

static uint16_t updateHumidityMeasurement(ClientAddress const address)
{
  uint16_t a = 0,b = 0, count = 20;


  do {
    sendCommand(address, Commands::cmdRequestMoistureMeasurement, a);
    sendCommand(address, Commands::cmdRequestMoistureMeasurement, b);
    if(!(--count)) {
      return 0;
    }
  }while(abs(a-b)>50);
  //sendCommand(address, Commands::cmdRequestMoistureMeasurement, c);

  return b; //equal(a,b,c);
}

static void updateMeasurements( void )
{
  for(unsigned index = 0; index < foundClients; ++index) {
    uint16_t parameter = 0x55aa;
    ClientAddress const address = availableClients[index].address;
    if(sendCommand(address, Commands::cmdPing, parameter) && 0x55aa == parameter){
      if(availableClients[index].types & cTemperature) {
        availableClients[index].temperature = updateTemperatureMeasurement(address);
      }
      if(availableClients[index].types & cHumidity) {
        availableClients[index].humidity = updateHumidityMeasurement(address);
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
  if(availableClients[index].types & cTemperature && availableClients[index].temperature > 0) {
    Serial.print("true,\"Temperature\":"); Serial.print(convertTemperature(availableClients[index].temperature));
  } else {
    Serial.print("false");
  }
  Serial.print(",\"hasMoisture\":");  
  if(availableClients[index].types & cHumidity && availableClients[index].humidity > 0) {
    Serial.print("true,\"Moisture\":"); Serial.print(availableClients[index].humidity);
  }else {
    Serial.print("false");
  }
  Serial.print("}");
}

static int readAnalgoValue() 
{
  ADCSRA |= _BV(ADSC); // start the conversion
  while (bit_is_set(ADCSRA, ADSC)); // ADSC is cleared when the conversion finishes

  int const result = (ADCL | (ADCH << 8)); // combine bytes & correct for temp offset (approximate)} 

  return result;
}

static float getTemperature()
{
  ADMUX = 0xC8; // turn on internal reference, right-shift ADC buffer, ADC channel = internal temp sensor
  delay(10);
  int temp = 0;
  for(int i=0; i<16; ++i) temp+=readAnalgoValue();

  return (temp/16.0)-356 + 21;
}

static uint16_t getCurrentSensor()
{
  ADMUX = 0x40; // turn on internal reference, right-shift ADC buffer, ADC channel = internal temp sensor
  delay(10);

  int result = 0;
  for(int i = 0; i<16; ++i) {
    result += readAnalgoValue();   
  }

  return result;
}


static uint16_t getVoltageSensor()
{
  ADMUX = 0x41; // turn on internal reference, right-shift ADC buffer, ADC channel = internal temp sensor
  delay(10);

  int result = 0;
  for(int i = 0; i<16; ++i) {
    result += readAnalgoValue();   
  }

  return result;
}
void (*SoftwareReset)(void) = 0;


void setup() {
  wdt_enable(WDTO_8S);
  wdt_reset();
  
  // put your setup code here, to run once:
  Serial.begin(19200);
  twiInitialize();

  Serial.print("Searching for clients...");
  scanBus();
  Serial.print(" found ");
  Serial.println(foundClients);

  if(!foundClients) {
    SoftwareReset();    
  }

  ADMUX = 0xC8; // turn on internal reference, right-shift ADC buffer, ADC channel = internal temp sensor
  delay(10);  // wait a sec for the analog reference to stabilize  
}

void loop() {
  
  unsigned long cycleStart = millis();
  static unsigned long duration = 0;

  wdt_reset();
  
  static unsigned sequence = 0;
  ++sequence;
  updateMeasurements();
  Serial.print("\30{");
  Serial.print("\"sequence\":");       Serial.print(sequence);
  Serial.print(",\"cycleStart\":");    Serial.print(cycleStart);
  Serial.print(",\"cycleDuration\":"); Serial.print(duration);  
  Serial.print(",\"temperature\":");   Serial.print(getTemperature());
  Serial.print(",\"current\":");       Serial.print(getCurrentSensor());
  Serial.print(",\"voltage\":");       Serial.print(getVoltageSensor());
     
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
  unsigned long const cycleEnd = millis();
  duration = cycleEnd - cycleStart;

  if(duration < 7000) {
    delay(6000 - duration);
  }
  //LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_ON);
}
