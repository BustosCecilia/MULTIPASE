#include <ArduinoHttpClient.h>
#include <SPI.h>
#include <MFRC522.h>  // mfrc522 module library
#include <Wire.h>  // i2C/1-wire library
#include <SD.h>  // SD card library
#include <Ethernet.h>

#define RST_PIN   9  // RST pin for mfrc522 module
#define SS_PIN    5  // Slave Select pine for mfrc522 module

//----prototipo de funciones-------
void mfrc522init(void);
void ESPinit(void);
void printWifiStatus(void);
void LedInit(void);
void estadoCerradura(int statusCode);

int postthttp(String code);
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
//LED
const int ledRojo = 43;// pin 43
const int ledVerde = 41;//pin 41

void setup() {
  /////////setyp wifi
  LedInit();  //definir pin como salida
  digitalWrite(ledRojo , HIGH);
  digitalWrite(ledVerde , HIGH);
  // initialize serial for debugging// Open serial communications and wait for port to open:
  Serial.begin(9600);
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
    Serial.println("Try to congifure using IP address 192.168.0.1");
    Ethernet.begin(mac, ip);
   // while(true);//no hago nada porque no pude ponerme ip
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  // print your local IP address:
  printIPAddress();
  
  Serial.println("connecting...");
  ////////////////////7
  // if you get a connection, report back via serial:
  if (client.connect(server, 8000)) {
    Serial.println("connected");
    // Make a HTTP request:
    client.println("GET /search?q=arduino HTTP/1.1");
    client.println("Host: www.google.com");
    client.println("Connection: close");
    client.println();
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }

  digitalWrite(ledRojo , LOW);
  digitalWrite(ledVerde , LOW);
}

void loop() 
{
 
  String code=readTag();//leo tarjetas
  int statusCode = postthttp(code); // mando UID card a base de datos para validar
  estadoCerradura(statusCode);
}

void printIPAddress()
{
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
 * @brief      Inicializa el wifi, se conecta a la red de WIFI
 * a 115200 Baudios por comunicación Serial1.
 */
/*
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
*/
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
 */
 /*
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
}*/

/**
 * @brief      Inicializa los leds indicadores como salida
 */
void LedInit(){
  pinMode(ledRojo , OUTPUT);  //definir pin como salida
  pinMode(ledVerde , OUTPUT);  //definir pin como salida  
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

