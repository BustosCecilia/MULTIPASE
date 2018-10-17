/*
  Morse.h - Library for flashing Morse code.
  Created by David A. Mellis, November 2, 2007.
  Released into the public domain.
*/
#ifndef MultipaseCore_h
#define MultipaseCore_h
 
#include "Arduino.h"
 
class MultipaseCore
{
  
  private:
    //LEDS
   int ledRojo;// pin 43
   int ledVerde;//pin 41
   int ledAmarillo; // pin 47
   int buzzer;  // speaker or buzzer on pin 8

   
  public:
    MultipaseCore();
    void LedInit(void);
    void errorBeep(void);  // error while reading (unknown tag)
	  void LEDSoff(void);
    void LEDverde(void);
    void LEDrojo(void);
    void LEDamarillo(void);
	  void OKtone(void);
    void noOKtone(void);
};
 
#endif
