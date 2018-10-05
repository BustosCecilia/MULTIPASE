/**
 * MULTIPASE Nodo Ethernet
 * 
 * ----------esquema sensor interrupt-----------------
 * Sensor normal cerrado NC configurado con resistencia PULLUP en pin 21
 * 
 *                        _vcc
 *                        |
 *             R pullup   R     SW
 *                        |    _|_PUERTA ABIERTA (chequear esto)
 * Arduino pin 21 ]_______|___.   ._____
 *                          |___||__|   |
 *                 filtro      +||      |
 *               antirrebote    C=1uF   |
 *                                      |
 *                                    __|__
 *                                     ___
 *                                      _
 *                                    
 *                        _vcc
 *                        |
 *                        R
 *                        |       PUERTA CERRADA
 * Arduino pin 21 ]_______|___._|_._____
 *                          |___||__|   |
 *                             +||      |
 *                              C       |
 *                                      |
 *                                    __|__
 *                                     ___
 *                                      _                          
 * ---------------------------------------------------
 */

#include <ArduinoHttpClient.h>
#include <SPI.h>
#include <MFRC522.h>  // mfrc522 module library
#include <Wire.h>  // i2C/1-wire library
#include <SD.h>  // SD card library
#include <Ethernet.h>

//----declaración de pines-------
//RFID
#define RST_PIN   9  // RST pin for mfrc522 module
#define SS_PIN    5  // Slave Select pine for mfrc522 module
//LEDS
const int ledRojo = 43;// pin 43
const int ledVerde = 41;//pin 41
const int buzzer = 45;  // speaker or buzzer on pin 8
//ExternalInterrupt
const int sensor=21;
volatile boolean estadoSensor=false; //false cerrado; true abierto ??
boolean estado=false;

//----prototipo de funciones-------
void mfrc522init(void);
void ESPinit(void);
void printWifiStatus(void);
void LedInit(void);
void interrupt(void);
void sensorInit(void);
void estadoCerradura(int statusCode);

int postthttp(String code);
int postthttp(void);

void errorBeep();  // error while reading (unknown tag)
void LEDSoff();
void OKtone();
void noOKtone();

//--------variables globales--------
MFRC522 mfrc522(SS_PIN, RST_PIN);  // define mfrc522 reader class
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
IPAddress server(192,168,1,104);  // numeric IP for Google (no DNS)
//char server[] = "www.google.com";    // name address for Google (using DNS)

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 1);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;
// inicializo http//////////////////////////////////////////////////////////
HttpClient http = HttpClient( client, server, 8000); // instancie un objeto http


void setup() {

  LedInit();  
  OKtone();
  sensorInit();

  Serial.begin(9600);  // initialize serial for debugging// Open serial communications and wait for port to open:
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  SPI.begin();  // run SPI library first; if not, mfrc522 will not work
  mfrc522.PCD_Init();  // initializing mfrc522, start mfrc522 library
  mfrc522init(); //inicializa el mfrc522
  //ESPinit();  //se conecta a la red de WIFI

   // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Serial.println("Try to configure using IP address 192.168.0.1");
    Ethernet.begin(mac, ip);
   // while(true);//no hago nada porque no pude ponerme ip
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  // print your local IP address:
  printIPAddress();

  digitalWrite(ledRojo , LOW);
  digitalWrite(ledVerde , LOW);
}

void loop() {
 
  String code=readTag();//leo tarjetas
  int statusCode = postthttp(code); // mando UID card a base de datos para validar
  estadoCerradura(statusCode);
}

void printIPAddress(){
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();
}

/**
 * @brief      Inicializa el mfrc522
 */
void mfrc522init(void){
  SPI.begin();        // Init SPI bus
  //mfrc522.PCD_Init(SS_PIN, RST_PIN); // Init MFRC522 card
  mfrc522.PCD_Init(); // Init MFRC522 card
  Serial.print(F("Reader "));
  mfrc522.PCD_DumpVersionToSerial();
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
        //esto lo hago para que sólo entre si se ha cambiado el estado de la cerradura
    if (estado != estadoSensor){
      estado = estadoSensor;
      Serial.println("Ha cambiado el estado de la puerta");
      int statusCode = postthttp(); // mando UID card a base de datos para validar
      estadoCerradura(statusCode); //ACÁ DEBERÍA IR OTRA FUNCIÓN estadoCerradura que haga otra cosa VER
    }
    
    // Look for new cards && Verify if the NUID has been readed
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      // Show some details of the PICC (that is: the tag/card)
      Serial.print(F("Card UID:"));
      for (int i=0; i<mfrc522.uid.size;i++){
        cadena=cadena+mfrc522.uid.uidByte[i];
      }
      Serial.println(cadena);
      Serial.print(F("PICC type: "));
      MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
      Serial.println(mfrc522.PICC_GetTypeName(piccType));
      // Halt PICC
      mfrc522.PICC_HaltA();
      // Stop encryption on PCD
      mfrc522.PCD_StopCrypto1();
      cardRead=true;
    } //if (mfrc522[reader].PICC_IsNewC

  }
  return cadena;
}

/**
 * @brief      Imprime por serial el valor de un tag almacenado en un buffer
 *
 * @param      buffer      The buffer
 * @param[in]  bufferSize  The buffer size
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
 
/**
 * @brief      { function_description }
 *
 * @param[in]  statusCode  The status code
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
    //case 500:
    default:
      Serial.println("500 Internal Server Error");
      break;  
  }
}

/**
 * @brief      { function_description }
 *
 * @param[in]  code  The code
 *
 * @return     { description_of_the_return_value }
 */ 
int postthttp(String code){
  HttpClient http = HttpClient( client, server, 8000);  // instancie un objeto http
  String postData="codigo_llave="+code+"&id_acciones=1&id_espacios=1&id_estado=1&timestamp=2018-01-19 03:15:05&hash=q&boton=Actualizar";
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

/**
  * @brief      función que postea el cambio de estado de la cerradura
  * de acuerdo al estado del sensor
  *
  * @param[in]  code  The code
  *
  * @return     { description_of_the_return_value }
  */ 
int postthttp(){
  HttpClient http = HttpClient( client, server, 8000);  // instancie un objeto http
  String postData="codigo_llave=""&id_acciones=1&id_espacios=1&id_estado=1&timestamp=2018-01-19 03:15:05&hash=q&boton=Actualizar";
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

/**
 * @brief      Inicializa los leds indicadores como salida
 */
void LedInit(){
  pinMode(ledRojo , OUTPUT);  //definir pin como salida
  pinMode(ledVerde , OUTPUT);  //definir pin como salida  
}

void interrupt(){
   estadoSensor=!estadoSensor; //El sensor cambió de estado
}

void sensorInit(){
  pinMode(sensor,INPUT_PULLUP);
  //leo estado inicial del sensor para ver cómo está la cerradura
  estadoSensor=digitalRead(sensor); //devuelve HIGH o LOW
  estado=estadoSensor;
  attachInterrupt(digitalPinToInterrupt(sensor), interrupt, CHANGE);
}

void errorBeep(){ // error option
  digitalWrite(ledRojo , HIGH);
  tone(buzzer, 440, 200);
  digitalWrite(ledRojo , LOW);
  delay(300);
  digitalWrite(ledRojo , HIGH);
  tone(buzzer, 440, 200);
  delay(300);
  digitalWrite(ledRojo , LOW);
}

void LEDSoff(){
  digitalWrite(ledRojo, LOW);
  digitalWrite(ledVerde, LOW);
}

void OKtone () {
  digitalWrite(ledVerde , HIGH);
  digitalWrite(ledRojo , LOW);
  tone(buzzer, 440,300);
  delay(100);
  tone(buzzer, 523, 300);
  delay(1000);
  digitalWrite(ledVerde , LOW);
  digitalWrite(ledRojo , LOW);
}

void noOKtone () {
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
ESTADOS

-CERRADA
  --QUERES ABRIRLA ->PASAS TARJETA
    ---ATORIZADO->QUEDA CERRADA ->PENDIENTE DE ABRIR
        ---ABRE LA PUERTA ->ABIERTA
    ---AUTORIZADO->SE ABRE ->ABIERTA
    ---NO AUTORIZADO ->CERRADA ->CERRADO

-ABIERTA
  --QUERES CERRARLA ->PASAS TARJETA
    ---
*/
