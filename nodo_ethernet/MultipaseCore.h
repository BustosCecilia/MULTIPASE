/*
  Morse.h - Library for flashing Morse code.
  Created by David A. Mellis, November 2, 2007.
  Released into the public domain.
*/
#ifndef MultipaseCore_h
#define MultipaseCore_h
 
#include "Arduino.h"

#include "ArduinoHttpClient.h"
#include <SPI.h>
#include "MFRC522.h"  // mfrc522 module library
//#include <Wire.h>  // i2C/1-wire library
//#include <SD.h>  // SD card library
#include <Ethernet.h>
 
class MultipaseCore
{
  
  private:
  	//----declaración de pines-------
    //LEDS
   	int ledRojo;// pin 43
   	int ledVerde;//pin 41
   	int ledAmarillo; // pin 45
   	int buzzer;  // speaker or buzzer on pin 4

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

    void mfrc522init(void);
};
 
#endif
