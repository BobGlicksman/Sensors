/***********************************************************************************************************/
// Temp_ATtiny:  Sketch to use ATtiny85 and DS18B20 as an SIS compatible temperature sensor.
//  *** This is NOT low power code. ***
//  The code was tested on an Arduino Uno using Serial.print() statements.  Make sure that
//  #define DEBUG is commented out prior to compiling for the ATtiny85.
//
//  Version 001.  Has three defined constants for trip codes:  high temperature, low temperature, and
//  return to normal temperature.  Each code is sent in one burst when it becomes a new event.  The
//  temperature is read out at an interval determined by a defined constant.  
//  (c) 2015,2016 by Bob Glicksman and Jim Schrempp
/***********************************************************************************************************/
// Version 001.  This version is not low power.  It uses delay() to implement timing intervals.  This
//  version does not test battery status, as it is designed for powered use.
/***********************************************************************************************************/

/********************************* GLOBAL CONSTANTS AND VARIABLES ******************************************/
//#define DEBUG // debug mode sends debugging data to the serial monitor via the serial port.
#define FAHRENHEIT // comment this line out to use centigrade scale.

// Temperature limits to trigger transmissions
const float HIGH_TEMP_LIMIT = 85.0; // degrees F; use degrees C if #define FAHRENHEIT is commented out
const float LOW_TEMP_LIMIT = 75.0;  // degrees F; use degrees C if #define FAHRENHEIT is commented out

// Time interval between temperature measurements, in milliseconds
const unsigned long INTERVAL = 2000;  // 2 seconds for testing; use something like 15 minutes or one hour

// Global constants for the transmission codes
const unsigned long HIGH_TEMP_CODE = 123456;
const unsigned long LOW_TEMP_CODE = 234567;
const unsigned long NORMAL_TEMP_CODE = 345678;
const int BAUD_TIME = 500;  // basic signalling unit is 500 us
const int CODE_BURST = 20;  // each code is sent 20 times

// Pin definitions
const int transmitPin = 3; // transmitter on Digital pin 3 (chip in 2) for ATtiny85
//const int transmitPin = 4;   // transmitter on Digital pin 4 for Uno (testing)  
const int oneWireBusPin = 4;  // one wire bus on Digital pin 4 (chip in 3) for ATtiny85
//const int oneWireBusPin = 3;  // one wire bus on Digital pin 4 for Uno (testing)
 
// includes for the DS18B20 temperature sensors
#include <OneWire.h>    // the one wire bus library
#include <DallasTemperature.h>  // the DS18B20 chip library -- uses the OneWire library

// Global variables for DS18B20 sensors
OneWire oneWire(oneWireBusPin);   // create an instance of the one wire bus
DallasTemperature sensors(&oneWire);  // create instance of DallasTemperature devices on the one wire bus

/********************************* END OF GLOBAL CONSTANTS AND VARIABLES ***********************************/

/********************************* BEGINNING OF setup() ****************************************************/
void setup()
{ 
  //setup serial monitor for debugging - UNO only
  #ifdef DEBUG
    Serial.begin (9600);
    delay(2000);  // leave some time to open the serial monitor
    Serial.println("Ready ... \n");
  #endif
  
  // setup the DS18B20 for 12 bits and non-blocking operation
  sensors.begin();      // startup the temperature sensor instance
  sensors.setResolution ( 12 ); // set the resolution to 12 bits
  sensors.setWaitForConversion ( false ); // set for non-blocking operation
  
  // setup the transmitter pin
  pinMode(transmitPin, OUTPUT);
  
}
/********************************* END OF setup() **********************************************************/

/********************************* BEGINNING OF loop() *****************************************************/
void loop()
{ 
  static boolean wasNormal = true;  // set to false if the last reading was too high or too low
  float newTemp;
  
  // Start the temperature reading   
    sensors.requestTemperatures(); 

  // wait for conversion - need this because of non-blocking call
  delay(750); // need 750 ms to convert to 12 bits.  Replace this with power down for sensor
  
  // read the temperature value form the DS18B20 in degrees F
  #ifdef FAHRENHEIT
    newTemp = sensors.getTempFByIndex(0);
  #else
    newTemp = sensors.getTempCByIndex(0);
  #endif  
  
  #ifdef DEBUG
    Serial.print("temperature reading = ");
    Serial.println(newTemp);
  #endif
  
  // test for out of normal range if the last reading was normal
  if (wasNormal == true)
  {
    if (newTemp > HIGH_TEMP_LIMIT) // temp went too high
    {
      transmit(HIGH_TEMP_CODE);
      wasNormal = false;
      
      #ifdef DEBUG
        Serial.println("... transmitting high temp code \n");
      #endif
      
    }
    else if (newTemp < LOW_TEMP_LIMIT)  // temp went too low
    {
      transmit(LOW_TEMP_CODE);
      wasNormal = false;
      
      #ifdef DEBUG
        Serial.println("... transmitting low temp code \n");
      #endif
      
    }
    else  // the last reading was normal and the current reading is normal
    {
      wasNormal = true;
      
      #ifdef DEBUG
        Serial.println("... no code transmitted \n");
      #endif      
      
    }
  
  }
  else // last reading was not in normal range
  {
    if ( (newTemp >= LOW_TEMP_LIMIT) && (newTemp <= HIGH_TEMP_LIMIT) )  // now normal again
    {
      transmit (NORMAL_TEMP_CODE);
      wasNormal = true;
      
      #ifdef DEBUG
        Serial.println("... transmitting return-to-normal code \n");
      #endif      
      
    }
    else  // current reading is out of normal range
    {
      wasNormal = false;

      #ifdef DEBUG
        Serial.println("... no code transmitted \n");
      #endif        
      
    }
  }
   
  delay(INTERVAL); // wait before taking another reading.  Replace this with power down for sensor

} 
/********************************* END OF loop() ***********************************************************/

/************************************* BEGINNING OF transmit() *********************************************/
// function to send out an SIS-compatible code
//  arguments:
//    code: the code to transmit


void transmit(unsigned long code)
{
  for (int i = 0; i < CODE_BURST; i++)
  {
    sendCodeWord(code);
  }
  return;  
}
/**************************************** END OF transmit() ************************************************/

/**************************************** sendCodeWord() **********************************************/
// sendCodeWord():  sends a 24 bit code to the transmitter data pin.  The code is encoded according
//  to the EV1527 format, with a zero being one baud unit high and three baun units low, a one being
//  three baud units high and one baun unit low, and a sync being one baud unit high and 31 baud units
//  low.  The pattern is shifted out MSB first with SYNC at the end of the code word.
//
// Parameters:
//  code:  the code word to send (the 24 LSBs of an unsigned long will be sent)

void sendCodeWord(unsigned long code)
{
  const unsigned long MASK = 0x00800000ul;  // mask off all but bit 23
  const int CODE_LENGTH = 24; // a code word is 24 bits + sync
  
  // send the code word bits  
  for (int i = 0; i < CODE_LENGTH; i++)
  {
    if ( (code & MASK) == 0)
    {
      sendZero();
    }
    else 
    {
      sendOne();
    }      
    code = code <<1;
  }
  //send the sync
  sendSync();
  
  return;
}
/************************************ end of sendCodeWord() *******************************************/

/****************************************** sendZero() ************************************************/
// sendZero:  helper function to encode a zero as one baud unit high and three baud units low.

void sendZero()
{
  // a zero is represented by one baud high and three baud low
  digitalWrite(transmitPin, HIGH);
  delayMicroseconds(BAUD_TIME);
  for(int i = 0; i < 3; i++)
  {
      digitalWrite(transmitPin, LOW);
      delayMicroseconds(BAUD_TIME);
  }  
  return; 
}
/************************************** end of sendZero() *********************************************/

/****************************************** sendOne() *************************************************/
// sendOne:  helper function to encode a one as three baud units high and one baud unit low.

void sendOne()
{
  // a one is represented by three baud high and one baud low
  for(int i = 0; i < 3; i++)
  {
      digitalWrite(transmitPin, HIGH);
      delayMicroseconds(BAUD_TIME);
  } 
  digitalWrite(transmitPin, LOW);
  delayMicroseconds(BAUD_TIME); 
  return; 
}
/*************************************** end of sendOne() *********************************************/

/***************************************** sendSync() *************************************************/
// sendSync:  helper function to encode a sync as one baud unit high and 31 baud units low.

void sendSync()
{
  // a sync is represented by one baud high and 31 baud low
  digitalWrite(transmitPin, HIGH);
  delayMicroseconds(BAUD_TIME);
  for(int i = 0; i < 31; i++)
  {
      digitalWrite(transmitPin, LOW);
      delayMicroseconds(BAUD_TIME);
  }  
  return; 
}
/************************************** end of sendSync() *********************************************/

