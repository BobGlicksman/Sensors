// Temp_ATtiny:  Sketch to use ATtiny85 and DS18B20 as an SIS compatible temperature sensor.
//  *** This is NOT low power code. ***
//  The code was tested on an Arduino Uno using Serial.print() statements.  Make sure that
//  #define DEBUG is commented out prior to compiling for the ATtiny85.

/********************************* GLOBAL CONSTANTS AND VARIABLES ******************************************/
#define DEBUG // debug mode sends debugging data to the serial monitor via the serial port.

// includes for the DS18B20 temperature sensors
#include <OneWire.h>    // the one wire bus library
#include <DallasTemperature.h>  // the DS18B20 chip library -- uses the OneWire library


// Define the I/O Pins for the ATtiny85
const int oneWireBusPin = 3;  // UNO pin for now
const int transmitPin = 4;    // UNO pin for now

// Global variables for DS18B20 sensors
OneWire oneWire(oneWireBusPin);   // create an instance of the one wire bus
DallasTemperature sensors(&oneWire);  // create instance of DallasTemperature devices on the one wire bus

// Global Variables for the transmission codes
const unsigned long HIGH_TEMP_CODE = 123456;
const unsigned long LOW_TEMP_CODE = 234567;
const unsigned long NORMAL_TEMP_CODE = 345678;

// Temperature limits to trigger transmissions
const float HIGH_TEMP_LIMIT = 85.0; // degrees F
const float LOW_TEMP_LIMIT = 75.0; // degrees F

/********************************* END OF GLOBAL CONSTANTS AND VARIABLES ***********************************/

/********************************* BEGINNING OF setup() ****************************************************/
void setup()
{ 
  //setup serial monitor for debugging - UNO only
  #ifdef DEBUG
    Serial.begin (9600);
    delay(2000);  // leave some time to opent he serial monitor
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
  newTemp = sensors.getTempFByIndex(0);
  
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
   
  delay(2000); // wait 2 seconds before taking another reading.  Replace this with power down for sensor

} 
/********************************* END OF loop() ***********************************************************/

/************************************* BEGINNING OF transmit() *********************************************/
// function to send out an SIS-compatible code
//  arguments:
//    code: the code to transmit


void transmit(unsigned long code)
{
  //*************LEFT OFF HERE *************
  
}


/**************************************** END OF transmit() ************************************************/


