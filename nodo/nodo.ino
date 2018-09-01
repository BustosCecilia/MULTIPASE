/* RFID TAG READER
Author: vonPongrac

Short description: Program read RFID tags. If tags UID is not familiar, sets new user

Hardware: Arduino Mega 2560, LED, Buzzer, RFID module RC522 with tags

Arduino IDE v1.6.3

Copyrights by vonPongrac
*/
// Include librarys
#include <SPI.h>
#include <MFRC522.h>  // RFID module library

#include <SD.h>  // SD card library
#include <Ethernet.h>  // Etrhenret library

#define RST_PIN		6  // RST pin for RFID module
#define SS_PIN		7  // Slave Select pine for RFID module

MFRC522 mfrc522(SS_PIN, RST_PIN);  // define RFID reader class

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // MAC address 
IPAddress ip(192, 168, 1, 177); // IP address
EthernetServer server(80); // define server class in port 80 - HTTP port

String readTag = "";  
int readCard[4];

// Declaration of the functions

int getID();  // read tag


// SETUP
void setup() {
  Serial.begin(9600); // for testing and debugging
  SPI.begin();  // run SPI library first; if not, RFID will not work
  mfrc522.PCD_Init();  // initializing RFID, start RFID library
 
  Ethernet.begin(mac, ip);  // start Ethernet library 
  server.begin();  // start server 
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  SD.begin(4);  // start SD library

}

// MAIN PROGRAM
void loop() {
  int succesRead = getID(); // read RFID tag
  
  // Web server
  EthernetClient client = server.available();  // check for HTTP request
  if (client) { // if HTTP request is available
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
        //  client.println("Refresh: 10");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE html>");
          client.println("<html><head><title>Office Atendance Logger</title><style>");
          client.println(".jumbotron{margin: 1% 3% 1% 3%; border: 1px solid none; border-radius: 30px; background-color: #AAAAAA;}");
          client.println(".dataWindow{margin: 1% 3% 1% 3%; border: 1px solid none; border-radius: 30px; background-color: #AAAAAA;padding: 1% 1% 1% 1%;}");
          client.println("</style></head><body style=\"background-color: #E6E6E6\">");
          client.println("<div class=\"jumbotron\"><div style=\"text-align: center\"> <h1>  Office Atendance Logger </h1> </div> ");
          client.println("</div><div class=\"dataWindow\"><div style=\"text-align: center\"> <h2> User A </h2>");
         // mando dato readTag        
          client.print("<p>");             
          client.print(readTag);            
          client.println("</p>");        
          client.println("</div></body></html>");    
          break;     
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }   
  delay(1000);
}

// FUNCIONES

int getID() { // Read RFID
    // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    //Serial.println("no card");
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return 0;
  }
  // There are Mifare PICCs which have 4 byte or 7 byte UID care if you use 7 byte PICC
  // I think we should assume every PICC as they have 4 byte UID
  // Until we support 7 byte PICCs
  Serial.println(F("Scanned PICC's UID:"));
  readTag = "";
  for (int i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
    readTag=readTag+String(readCard[i], HEX);
  }
  Serial.println(readTag);
 Serial.println("");
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}


