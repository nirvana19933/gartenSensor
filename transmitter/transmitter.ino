#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>



RF24 radio(7, 8); // CE, CSN
const byte reciverAddress[6] = "00001";
const byte transmitterAddress[6] = "00000";
String busRead = ""; //Initialized variable to store recieved data
String recevedPacket = "";


char mystr[10]; //Initialized variable to store recieved data


void setup() {
  Serial.begin(9600);
  Serial.setTimeout(5); //sonst wartet er beim read until zu lange und empfängt nichts vom sensor
    
  radio.begin();
  radio.openWritingPipe(reciverAddress);
  radio.openReadingPipe(0, transmitterAddress);
  radio.setPALevel(RF24_PA_MIN);



}
void loop() {

busRead=Serial.readStringUntil('*');

  if (busRead != "" && busRead.indexOf('#') != -1) {
    if (busRead.indexOf('#') > 0 ) { // loescht fehlerhaften praefix
      busRead.remove(0, busRead.indexOf('#') );
    }

    if (busRead.startsWith("#") ) {          //PrÃ¼ft ob eingabe korrekt startet und ob # vorhanden
      recevedPacket = busRead+"#";
      Serial.println(recevedPacket);
    
    }
  }


    
//read ende
  delay(5);
  radio.startListening();
  if (radio.available()) {
    char text[64] = "";
    radio.read(&text, sizeof(text));
    Serial.println(text);
  }
    delay(5);
  radio.stopListening();
//  char serialText[]=
  char mystr[recevedPacket.length()+1];
  recevedPacket.toCharArray(mystr, recevedPacket.length()+1); //+1 wegen null initializer
  //mystr.toCharArray(busRead,busRead.length());
  radio.write(&mystr, sizeof(mystr));
}

