
/*
 WiFiEsp example: WebClient

 This sketch connects to google website using an ESP8266 module to
 perform a simple web search.

 For more details see: http://yaab-arduino.blogspot.com/p/wifiesp-example-client.html
--------------conexión wifi-----------------------
  Se conecta el puerto serie de debug del módulo wifi al serial rx19 y tx18
 del arduino mega según el esquema:
 ------------|       |---------------
 wifiesp  gnd|-------|gnd       Arduino 
   debug 3.3v|-------|3.3v      Mega
  serial   Rx|-------|Rx pin 19
           Tx|-------|Tx pin 18
 ------------|       |--------------

--------------conexión rfid-----------------------
 --------------|       |---------------
 rfid-rc522    |       |        Arduino 
    spi       1|-------|12/50     Mega
    ports     2|-------|
     ___      3|-------|13/
miso|1|2|vcc  4|-------|11/
sck |3|4|mosi 5|-------|
rst |5|6|gnd  6|-------|
          rst 9|-------|9
          ss 10|-------|8
 --------------|       |--------------

 ALT+Q para DOXIGEN
//////////////////////////////////////////////////////////////////////////
PAGINAS LIBRERIA HTTP:https://arduino.stackexchange.com/questions/45625/simple-get-request-with-esp8266httpclient
https://github.com/arduino-libraries/ArduinoHttpClient/blob/master/examples/SimplePost/SimplePost.ino

////////////////////////////////////////////////////////////////////////////
una vez que recibe la autorizacion se tiene que fijar en el estado del sensor si esta abierto o cerrado
*/
//LIBRERIAS WIFI
#include "WiFiEsp.h"
//LIBRERIA HTTP
#include <ArduinoHttpClient.h>

//LIBRERIAS RFID
#include <SPI.h>
#include <MFRC522.h>
#include <stdlib.h>
// Emulate Serial1 on pins RX, TX if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(19, 18); // RX, TX
#endif

#define SS_PIN 8
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class


//----prototipo de funciones-------
void RFIDinit(void);
void ESPinit(void);
void printWifiStatus(void);
void post(void);
void post(String code);
void postResponse(void);
void LedInit(void);
void estadoCerradura(int statusCode);

int postthttp(String code);
//--------variables globales--------
char ssid[] = "domingo";         // your network SSID (name)
char pass[] = "32797989";        // your network password
//char ssid[] = "SCHAUFELE";     // your network SSID (name)
//char pass[] = "51269340";      // your network password
//char ssid[] = "mingo";         // your network SSID (name)
//char pass[] = "1234567890x";   // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

//char server[] = "arduino.cc";
IPAddress server(192,168,1,104);

// Initialize the Ethernet client object
WiFiEspClient client;

// inicializo http//////////////////////////////////////////////////////////
//HttpClient http = HttpClient( client, server, 8000); // instancie un objeto http

//LED
const int ledRojo = 43;// pin 43
const int ledVerde = 41;//pin 41

/**
 * @brief      { function_description }
 */
void setup(){
  LedInit();  //definir pin como salida
  // initialize serial for debugging
  Serial.begin(115200);
  digitalWrite(ledRojo , HIGH);
  digitalWrite(ledVerde , HIGH);
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  RFIDinit(); //inicializa el rfid
  ESPinit();  //se conecta a la red de WIFI
  digitalWrite(ledRojo , LOW);
  digitalWrite(ledVerde , LOW);

 
}

/**
 * @brief      { function_description }
 */
void loop()
{
 
  String code=readTag();//leo tarjetas
  int statusCode = postthttp(code); // mando UID card a base de datos para validar
  estadoCerradura(statusCode);
}

/**
 * @brief      { function_description }
 */
void RFIDinit(void){
  SPI.begin();        // Init SPI bus
  //rfid.PCD_Init(SS_PIN, RST_PIN); // Init MFRC522 card
  rfid.PCD_Init(); // Init MFRC522 card
  Serial.print(F("Reader "));
  rfid.PCD_DumpVersionToSerial();
}
/**
 * @brief      { function_description }
 */
 
//se conecta a la red de WIFI
void ESPinit(){
  // initialize serial for ESP module
  Serial1.begin(115200);
  // initialize ESP module
  WiFi.init(&Serial1);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  // you're connected now, so print out the data
  Serial.println("You're connected to the network");
  
  printWifiStatus();
}

/**
 * @brief      Lee un tag.
 *
 * @return     Un string que contiene el valor del tag.
 */
String readTag(void){
  String cadena="";
  boolean cardRead=false;
  while(!cardRead){
    // Look for new cards && Verify if the NUID has been readed
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      // Show some details of the PICC (that is: the tag/card)
      Serial.print(F("Card UID:"));
      for (int i=0; i<rfid.uid.size;i++){
        cadena=cadena+rfid.uid.uidByte[i];
      }
      Serial.println(cadena);
      Serial.print(F("PICC type: "));
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      Serial.println(rfid.PICC_GetTypeName(piccType));

      // Halt PICC
      rfid.PICC_HaltA();
      // Stop encryption on PCD
      rfid.PCD_StopCrypto1();
      cardRead=true;
    } //if (mfrc522[reader].PICC_IsNewC
  }
    return cadena;
}
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * @brief      { function_description }
 */
void printWifiStatus(){
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


/**
 * @brief      { function_description }
 */

void LedInit(){
  pinMode(ledRojo , OUTPUT);  //definir pin como salida
  pinMode(ledVerde , OUTPUT);  //definir pin como salida  
}

/**
 * @brief      { function_description }
 */
 
void estadoCerradura(int statusCode){
   int permiso=statusCode;
   
   switch (permiso) {
  case 200:
    Serial.println("Permitido, ahora ver accion a seguir al evaluar estado de la cerradura");
     digitalWrite(ledRojo , LOW);
     digitalWrite(ledVerde , HIGH);
    break;
  case 302:
      Serial.println("No Permitido");
      digitalWrite(ledRojo , HIGH);
      digitalWrite(ledVerde , LOW);
    break;
  case -2:
      Serial.println("Servidor desconectado");
    break;
  case 500:
  Serial.println("500 Internal Server Error");
 digitalWrite(ledRojo , HIGH);
  digitalWrite(ledVerde , HIGH);
}
 
}
/**
 * @brief      { function_description }
 */
 
int postthttp(String code){
  HttpClient http = HttpClient( client, server, 8000);  // instancie un objeto http
  String postData="codigo_llave="+code+"&id_acciones=1&id_espacios=2&id_estado=3&timestamp=2018-02-2 03:15:05&hash=q&boton=Actualizar";
  String contentType = "application/x-www-form-urlencoded";
  int statusCode=0;
 Serial.println("making POST request");
 http.post("/Multipase/Accesos/", contentType, postData);

  // read the status code and body of the response
  statusCode = http.responseStatusCode();
  Serial.print("Status code: ");
  Serial.println(statusCode);
  http.stop();  
  return statusCode;
  }

