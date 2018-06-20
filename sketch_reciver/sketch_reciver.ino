#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7, 8); // CE, CSN
const byte reciverAddress[6] = "00001";
const byte transmitterAddress[6] = "00000";
void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(transmitterAddress);
  radio.openReadingPipe(0, reciverAddress);
  radio.setPALevel(RF24_PA_MIN);

}
void loop() {

  //delay(5);
  radio.startListening();
  if (radio.available()) {
    char text[32] = "";
    radio.read(&text, sizeof(text));
    Serial.println(text);
  }
 //   delay(5);
 // radio.stopListening();
 // const char text2[] = "Hello transmitter";
 // radio.write(&text2, sizeof(text2));
}
