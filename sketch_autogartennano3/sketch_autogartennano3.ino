#include <EEPROM.h>
#include <SimpleTimer.h>
//wifi
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


boolean detected = false; //sendet ID solange nicht erkannt
//String id ;
String busRead, recevedPacket;
// EEPROM VALUES = FOR WATERSENSOR = 1-8
int moisureSensorValueCounter = 0;
SimpleTimer timer;
int moisureSensorValues[8][5] =
{ {50, 50, 50, 50, 50},
  {50, 50, 50, 50, 50},
  {50, 50, 50, 50, 50},
  {100, 100, 100, 100, 100},
  {100, 100, 100, 100, 100},
  {100, 100, 100, 100, 100},
  {100, 100, 100, 100, 100},
  {100, 100, 100, 100, 100}
};
int temperaturSensorValues[8][5] =
{ {20, 20, 20, 20, 20},
  {20, 20, 20, 20, 20},
  {20, 20, 20, 20, 20},
  {20, 20, 20, 20, 20},
  {20, 20, 20, 20, 20},
  {20, 20, 20, 20, 20},
  {20, 20, 20, 20, 20},
  {20, 20, 20, 20, 20}
};


// es wir mehrmals gemessen um ausreiser auszuschließen
String id = String("#AAAAAA--W") + "005" + "SENSORID" + 50  + "*"; //0,7 //7,10//10,13//13,21//21-64 // speicher   1-9 
String id2 = String("#AAAAAA--T") + "005" + "SENSORID" + 50  + "*"; //0,7 //7,10//10,13//13,21//21-64 // speicher   10-19 
//WIfi
RF24 radio(7, 8); // CE, CSN
const byte reciverAddress[6] = "00001";
const byte transmitterAddress[6] = "00000";


void setup() {
  //General
  Serial.begin(9600);
  //Timer
  timer.setInterval(1000, saveStats); // ein mal pro stunde
  //WIFI
  radio.begin();
  radio.openWritingPipe(transmitterAddress);
  radio.openReadingPipe(0, reciverAddress);
  radio.setPALevel(RF24_PA_MIN);
  //IO
  pinMode(2, OUTPUT);
  EEPROMWriteInt(1, 50);
  

  
}


void loop() {
    timer.run();
  if (detected == false) { // detect gibt an ob gerat bereits registriert //
    writeValues(id); 
    delay(10);
     writeValues(id2);
     delay(10);
     //writeValues(String( EEPROM.length()));
  }

 
 

  busRead = Serial.readStringUntil('*');  //* PrÃ¼ft ob eingabe korrekt endet und liest von datenleitung 

  //Wifi Listening
  delay(5);
  radio.startListening(); //Wifi Listening
   if (radio.available() && busRead=="") { 
    char wifiText[32] = "";
    radio.read(&wifiText, sizeof(wifiText));
    Serial.println(wifiText);
    busRead=wifiText;
  }


  if (busRead != "") {
    if (busRead.indexOf('#') > 0 && busRead.indexOf('#') != -1) { // loescht fehlerhaften oder nicht vorhandenem  praefix
    }

    if (busRead.startsWith("#AAAAAA")) {          //PrÃ¼ft ob eingabe korrekt startet und korrekt angesprochen wird
      detected = true;
      recevedPacket = busRead;
      startFunction();
    }
  }

    //Serial.println(busRead);
  boolean pruefeSensor = checkMoisureSensor(1, analogRead(0));
  controlWatering(1+1,pruefeSensor); // sensor nummer +1 entspricht digitalen ausgang

}


void saveStats() {

  //  EEPROMWriteInt(10000, 11551);
   //Serial.println( EEPROMReadInt(10000));
  
  int ThempAdress=200;
  int MoisureAdress=100;
  int h,m,s;
  s = millis() / 1000;
  m = s / 60;
  h = s / 3600;
  s = s - m * 60;
  m = m - h * 60;
  if(s>=3){
    h=0;
    ThempAdress++;
    MoisureAdress++;
    }
}


void startFunction() {
  if (recevedPacket.substring(13, 16).equals("SET")) { //setter
    if (recevedPacket.substring(13, 21).equals("SETVALUE")) {
      setValue();
    }
  }
  else if (recevedPacket.substring(13, 16).equals("GET")) { //Getter
    if (recevedPacket.substring(13, 21).equals("GETWATER")) {
      externalReadNewValue();
    }
  }

 
}


void writeValues(String sendPacket) {
  if (sendPacket.indexOf('#') == 0  && sendPacket.indexOf('*') == sendPacket.length() - 1) {
    Serial.println(sendPacket);
       delay(5);
        radio.stopListening();
        const char wifiText[64];
        sendPacket.toCharArray(wifiText,64);
        radio.write(&wifiText, sizeof(wifiText));
  }
}

void setValue() {
  int sensorNumber = recevedPacket.substring(10, 13).toInt();
  int newSensorValue = recevedPacket.substring(21, recevedPacket.length()).toInt();
  EEPROMWriteInt(sensorNumber, newSensorValue);
  if (recevedPacket.substring(7, 10).equals("--W")) {
    for(int i=0; i<5;i++){
      moisureSensorValues[sensorNumber][i]=newSensorValue;
      }
  } else if (recevedPacket.substring(7, 10).equals("--T")) {
    for(int i=0;i<5;i++){
      temperaturSensorValues[sensorNumber][i]=newSensorValue;
      }
  }
  
  externalReadNewValue();
}



void externalReadNewValue() {
  if (recevedPacket.substring(7, 10).equals("--W")) {
    writeValues("#AAAAAA" + recevedPacket.substring(7, 13) + "SETWATER" + EEPROMReadInt(recevedPacket.substring(10, 13).toInt()) + "*");
  }

}

void EEPROMWriteInt(int address, int value)
{
  byte two = (value & 0xFF);
  byte one = ((value >> 8) & 0xFF);
  EEPROM.update(address, two);
  EEPROM.update(address + 1, one);
}
int EEPROMReadInt(int address)
{
  long two = EEPROM.read(address);
  long one = EEPROM.read(address + 1);

  return ((two << 0) & 0xFFFFFF) + ((one << 8) & 0xFFFFFFFF);
}


boolean checkMoisureSensor(int sensorNumber, int sensorValue) {
  boolean needWater = true;
  int percentageSensorValue = map(sensorValue, 1024, 275, 0, 100); // 1024=trocken 275=Sensor in wasser;﻿
  if (percentageSensorValue > 100) { // runde übergroße werte ab
    percentageSensorValue = 100;
  }
  moisureSensorValues[sensorNumber - 1][moisureSensorValueCounter % 5] = percentageSensorValue; // schreibe aktuellen wert
  moisureSensorValueCounter++;
  for (int i = 0; i < 5; i++) {
    if (moisureSensorValues[sensorNumber - 1][i] >   EEPROMReadInt(1)) { // solange ein sollwert  > messwert ist gieße nicht
      needWater = false;
      return needWater; // =="break"
    }
  }
  return needWater;

}


void controlWatering(int sensorNumber, bool needWater) {
 
    if(needWater==1){
      digitalWrite(sensorNumber, HIGH);   // turn the LED on (HIGH is the voltage level)
    }
    else{
       digitalWrite(sensorNumber, LOW);
    }
}




