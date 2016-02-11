/* ATtiny85 Low-Power Testing.*/
//  Light up an LED for 1 second and then power down as far as can go.
//  Measure power.  Wake up only by activating !RESET
//  Note: the code works.  The power when sleeping is 0.5 microamps
//  Bob Glicksman; 2/11/16

#include <avr/sleep.h>
#include <avr/interrupt.h>  // won't use interrupts in this test.  Wake up on !RESET
//#include <avr/power.h>      // use for powering down the ADC (does not work - use the macro defined below)
#include <avr/wdt.h>        // will just disable the wdt for this test

// Utility macros for the ADC (note: the power(adc_disable does not seem to work)
#define adc_disable() (ADCSRA &= ~(1<<ADEN)) // disable ADC (before power-off)
#define adc_enable()  (ADCSRA |=  (1<<ADEN)) // re-enable ADC

// constants 
const int LED_PIN = 3;              // chip pin 2 is where LED is connected

void setup()
{
  for (int i = 0; i < 4; i++)
  {
    pinMode(i, INPUT);  // set all I/O pins to input
    digitalWrite(i, LOW); // disable pullups    
  }
  //power_adc_disable(); // ADC uses ~320uA
  adc_disable();
  wdt_disable();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}


void loop()
{

    // light the LED for 1 second
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);

    // put the LED pin back to input, no pullup for low power
    pinMode(LED_PIN, INPUT);
    digitalWrite(LED_PIN, LOW);

    // put the ATtiny85 to sleep
    snooze();
  // Continue after reset
}

void snooze(void)
{
  cli();  // disable interrupts; not used in this test
  sleep_enable();
  sleep_bod_disable();
  sei();  // enable interrupts to wake up; not used in this test
  sleep_cpu();
  sleep_disable();
}


