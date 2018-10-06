/*
  Morse.cpp - Library for flashing Morse code.
  Created by David A. Mellis, November 2, 2007.
  Released into the public domain.
*/
 
#include "Arduino.h"
#include "MultipaseCore.h"
 
MultipaseCore::MultipaseCore()
{
  ledRojo   = 43;// pin 43
  ledVerde  = 41;//pin 41
  buzzer    = 45;  // speaker or buzzer on pin 8
  //ExternalInterrupt
  //sensor    =21;
  //estadoSensor=false; //false cerrado; true abierto ??
  //estado=false;
}
/**
 * @brief      Inicializa los leds indicadores como salida
 */
void MultipaseCore::LedInit(){
  pinMode(ledRojo , OUTPUT);  //definir pin como salida
  pinMode(ledVerde , OUTPUT);  //definir pin como salida  
}

void MultipaseCore::errorBeep(){ // error option
  digitalWrite(ledRojo , HIGH);
  tone(buzzer, 440, 200);
  digitalWrite(ledRojo , LOW);
  delay(300);
  digitalWrite(ledRojo , HIGH);
  tone(buzzer, 440, 200);
  delay(300);
  digitalWrite(ledRojo , LOW);
}
void MultipaseCore::LEDverde(){ // error option
  digitalWrite(ledVerde , HIGH);

}
void MultipaseCore::LEDrojo(){ // error option
  digitalWrite(ledRojo , HIGH);
}
void MultipaseCore::LEDSoff(){
  digitalWrite(ledRojo, LOW);
  digitalWrite(ledVerde, LOW);
}

void MultipaseCore::OKtone () {
  digitalWrite(ledVerde , HIGH);
  digitalWrite(ledRojo , LOW);
  tone(buzzer, 440,300);
  delay(100);
  tone(buzzer, 523, 300);
  delay(1000);
  digitalWrite(ledVerde , LOW);
  digitalWrite(ledRojo , LOW);
}

void MultipaseCore::noOKtone () {
  digitalWrite(ledRojo , HIGH);
  digitalWrite(ledVerde , LOW);
  tone(buzzer, 523, 300);
  delay(100);
  tone(buzzer, 440,300);
  delay(1000);
  digitalWrite(ledRojo , LOW);
  digitalWrite(ledVerde , LOW);
}
/*
int MultipaseCore::getSensorPin(){
  return sensor;
}

void MultipaseCore::setEstado(boolean _estado){
  this->estado=_estado;
}
void MultipaseCore::setEstadoSensor(boolean _estado){
  this->estado=_estado;
}

bool MultipaseCore::getEstado(){
  return estado;
}

bool MultipaseCore::getEstadoSensor(){
  return estadoSensor;
}

void MultipaseCore::sensorInit(){
  pinMode(sensor,INPUT_PULLUP);
  //leo estado inicial del sensor para ver cómo está la cerradura
  estadoSensor=digitalRead(sensor); //devuelve HIGH o LOW
  estado=estadoSensor;
  //attachInterrupt(digitalPinToInterrupt(this->sensor), MultipaseCore::interrupt, CHANGE);
}
*/
