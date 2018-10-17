/*
  MultipaseCore.cpp - Librería de funciones básicas Multipase.
  Created by Multipase, November 2, 2018.
*/
 
#include "Arduino.h"
#include "MultipaseCore.h"
 
/**
 * @brief      Constructs the object.
 */
MultipaseCore::MultipaseCore()
{
  ledRojo   = 43;// pin 43
  ledVerde  = 41;// pin 41
  buzzer    = 47;  // pin 47 speaker or buzzer
  ledAmarillo = 45; // pin 45
}

/**
 * @brief      Inicializa los leds indicadores como salida.
 */
void MultipaseCore::LedInit(){
  pinMode(ledRojo , OUTPUT);  //definir pin como salida
  pinMode(ledVerde , OUTPUT);  //definir pin como salida  
   pinMode(ledAmarillo, OUTPUT);  //definir pin como salida  
}

/**
 * @brief      Emite beep de error con led rojo y se apaga
 */
void MultipaseCore::errorBeep(){ // error option
  LEDSoff();
  digitalWrite(ledRojo , HIGH);
  tone(buzzer, 440, 200);
  digitalWrite(ledRojo , LOW);
  delay(300);
  digitalWrite(ledRojo , HIGH);
  tone(buzzer, 440, 200);
  delay(300);
  digitalWrite(ledRojo , LOW);
}

/**
 * @brief      Enciende led verde y queda encendido.
 */
void MultipaseCore::LEDverde(){ 
  digitalWrite(ledVerde , HIGH);
}

/**
 * @brief      Enciende led rojo y queda encendido.
 */
void MultipaseCore::LEDrojo(){ // error option
  digitalWrite(ledRojo , HIGH);
}

/**
 * @brief      Apaga todos los leds.
 */
void MultipaseCore::LEDamarillo(){ // error option
  digitalWrite(ledAmarillo , HIGH);
}
void MultipaseCore::LEDSoff(){
  digitalWrite(ledRojo, LOW);
  digitalWrite(ledVerde, LOW);
   digitalWrite(ledAmarillo, LOW);
}

/**
 * @brief      Emite beep de OK con led verde y se apaga
 */
void MultipaseCore::OKtone () {
  LEDSoff();
  digitalWrite(ledVerde , HIGH);
  tone(buzzer, 440,300);
  delay(100);
  tone(buzzer, 523, 300);
  delay(1000);
  digitalWrite(ledVerde , LOW);
}

/**
 * @brief      Emite beep de noOK con led rojo y se apaga
 */
void MultipaseCore::noOKtone () {
  LEDSoff();
  digitalWrite(ledRojo , HIGH);
  tone(buzzer, 523, 300);
  delay(100);
  tone(buzzer, 440,300);
  delay(1000);
  digitalWrite(ledRojo , LOW);
}
