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
#include "MultipaseCore.h"
#include "ArduinoHttpClient.h"
#include <SPI.h>
#include <MFRC522.h>  // mfrc522 module library
#include <Wire.h>  // i2C/1-wire library
#include <SD.h>  // SD card library
#include <Ethernet.h>

//----declaración de pines-------
//RFID
#define RST_PIN   9  // RST pin for mfrc522 module
#define SS_PIN    5  // Slave Select pine for mfrc522 module

int sensor=21;
static volatile boolean estadoSensor=false; //false cerrado; true abierto ??
int estadoNodo=4; //me fijo estado actual (empiezo en 4 porque es lo mas seguro)
boolean estadoSensorAnterior=false;
/*
 * estadoNodo
1 abierto           estadoSensor=true
2 cerrado           estadoSensor=false
3 pendiente abrir   estadoSensor=false
4 pendiente cerrar  estadoSensor=true
*/
   
boolean estado=false;

//----prototipo de funciones-------
void mfrc522init(void);
void ESPinit(void);
void printWifiStatus(void);

void solicitarAccion(String code);
void estadoCerradura(int statusCode);

int  postthttp(String code);
int  postthttp(int estado);
void sensorInit(void); 

MultipaseCore Core;

//--------variables globales--------
MFRC522 mfrc522(SS_PIN, RST_PIN);  // define mfrc522 reader class
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
IPAddress server(192,168,1,107);  // numeric IP for Google (no DNS)
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

  Core.LedInit();  
  Core.OKtone();
  sensorInit();
  attachInterrupt(digitalPinToInterrupt(sensor), interrupt, CHANGE);


  Serial.begin(9600);  // initialize serial for debugging// Open serial communications and wait for port to open:
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  //SPI.begin();  // run SPI library first; if not, mfrc522 will not work
  //mfrc522.PCD_Init();  // initializing mfrc522, start mfrc522 library
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
  
  Core.OKtone();
  Core.LEDSoff();
}

void loop() {
 
  String code=readTag();//leo tarjetas
//  int statusCode = postthttp(code); // mando UID card a base de datos para validar
  //estadoCerradura(statusCode);
  solicitarAccion(code);
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
  mfrc522.PCD_Init();  // initializing mfrc522, start mfrc522 library
  //mfrc522.PCD_Init(SS_PIN, RST_PIN); // Init MFRC522 card
  //mfrc522.PCD_Init(); // Init MFRC522 card
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
  //boolean _estado = Core.getEstado();
  //boolean _estadoSensor=Core.getEstadoSensor();
  while(!cardRead){
        //esto lo hago para que sólo entre si se ha cambiado el estado de la cerradura
    if (estadoSensorAnterior != estadoSensor){
      estadoSensorAnterior=estadoSensor;
      Serial.println("Ha cambiado el estado de la puerta");
      //int statusCode = postthttp(); // mando UID card a base de datos para validar
      //estadoCerradura(statusCode); //ACÁ DEBERÍA IR OTRA FUNCIÓN estadoCerradura que haga otra cosa VER
      if(estadoSensorAnterior){//el sensor se abrió
        Serial.print("estado: ");
        Serial.println(estado);
       switch(estadoNodo){
        case 2://CERRADO-->PENDIENTE DE CERRAR
          Serial.println("SWAT");
          Core.LEDrojo();
          //Llamar a SWAT
          break;
        case 1://ABIERTO--pendiente de cerrar
          //esto no debería pasar porque no pasé tarjeta
          break;
        case 4://PC
          //no debería pasar
          break;
        case 3://PA-->ABIERTO
          estadoNodo=1;
          while(postthttp(estadoNodo)!=200){};
          Serial.println("abierto");
          //prendo led verde
          Core.LEDverde();
          break;
        default:
          //no deberia pasar
          break;
      }
     }else//el sensor se cerró
      switch(estadoNodo){
        case 2://CERRADO
          //esto no debe pasar
          break;
        case 1://ABIERTO--pendiente de ABRIR
          estadoNodo=3;
          while(postthttp(estadoNodo)!=200){};
          Serial.println("Pendiente de abrir");
          //prendo led amarillo y verde
          break;
        case 4://PC-->CERRADO
           estadoNodo=2;
          while(postthttp(estadoNodo)!=200){};
          Serial.println("Cerrau");
          Core.LEDrojo();
          //prendo led rojo
          break;
        case 3://PA-->ABIERTO
          //esto no debería pasar
          break;
        default:
          //no deberia pasar
          break;
      }
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


void solicitarAccion(String _code){
  int accion=0;
  if(estadoNodo==1||estadoNodo==3){
    accion=2;
  }else{
    accion=1;
  }
  int permiso;
  permiso=postthttp(_code,accion);
  switch (permiso) {
    case 200:
      Serial.println("Permitido, ahora ver accion a seguir al evaluar estado de la cerradura");
      Core.OKtone();
       switch(estadoNodo){
        case 2://CERRADO-->pendiente de abrir
          estadoNodo=3;
          while(postthttp(estadoNodo)!=200){};
          Serial.println("Pendiente de abrir");
          Core.OKtone();
          Core.LEDverde();
          break;
        case 1://ABIERTO--pendiente de cerrar
          estadoNodo=4;
          while(postthttp(estadoNodo)!=200){};
          Serial.println("Pendiente de cerrar");
          //prendo led amarillo y rojo (ceciCode)
          Core.OKtone();
          Core.LEDrojo();
          break;
        case 4://PC-->abierto
           estadoNodo=1;
          while(postthttp(estadoNodo)!=200){};
          Serial.println("Abiertirijillo");
          Core.OKtone();
          Core.LEDverde();
          //prendo led verde
          break;
        case 3://PA-->cerrado
          estadoNodo=2;
          while(postthttp(estadoNodo)!=200){};
          Serial.println("Cerrau");
          Core.OKtone();
          Core.LEDrojo();
          //prendo led rojo
          break;
        default:
          //no deberia pasar
          break;
      }
      break;
    case 302:
      Serial.println("No Permitido");
      Core.noOKtone();
      break;
    case -2:
      Serial.println("Servidor desconectado, mal puesta la ip");//mal puesta la ip
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
 * @param[in]  statusCode  The status code
 */
void estadoCerradura(int statusCode){
  int permiso=statusCode;
  switch (permiso) {
    case 200:
      Serial.println("Permitido, ahora ver accion a seguir al evaluar estado de la cerradura");
      Core.OKtone();
      break;
    case 302:
      Serial.println("No Permitido");
      Core.noOKtone();
      break;
    case -2:
      Serial.println("Servidor desconectado, mal puesta la ip");//mal puesta la ip
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
int postthttp(String code, int _accion){
  HttpClient http = HttpClient( client, server, 8000);  // instancie un objeto http
  String postData="codigo_llave="+code+"&id_acciones="+(String)_accion+"&id_espacios=1&id_estado="+(String)estadoNodo+"+&timestamp=2018-01-19 03:15:05&hash=q&boton=Actualizar";
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
int postthttp(int _estado){
  HttpClient http = HttpClient( client, server, 8000);  // instancie un objeto http
  String postData="codigo_llave=""&id_acciones=""&id_espacios=1&id_estado="+(String)_estado+"&timestamp=""&hash=q&boton=Actualizar";
  Serial.println(postData);
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


void interrupt(){
   estadoSensor=digitalRead(sensor); //El sensor cambió de estado
}

void sensorInit(){
  pinMode(sensor,INPUT_PULLUP);
  //leo estado inicial del sensor para ver cómo está la cerradura
  estadoSensor=digitalRead(sensor); //devuelve HIGH o LOW
  estado=estadoSensor;
  //attachInterrupt(digitalPinToInterrupt(this->sensor), interrupt, CHANGE);
}

