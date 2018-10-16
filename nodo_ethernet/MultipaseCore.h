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
   int ledAmarillo; // pin 46
   int buzzer;  // speaker or buzzer on pin 8
  //ExternalInterrupt
   //int sensor;
   //static volatile boolean estadoSensor; //false cerrado; true abierto ??
   //boolean estado;
   
  public:
    MultipaseCore();
    void LedInit(void);
    //void interrupt(void);
    //int getSensorPin(void);
    //void setEstado(boolean _estado);
    //void setEstadoSensor(boolean _estado);
    //bool getEstado(void);
    //bool getEstadoSensor(void);
    //void sensorInit(void);

    void errorBeep(void);  // error while reading (unknown tag)
	  void LEDSoff(void);
    void LEDverde(void);
    void LEDrojo(void);
    void LEDamarillo(void);
	  void OKtone(void);
    void noOKtone(void);
    

};
 
#endif
