#include <EEPROM.h>
boolean detected = false; //sendet ID solange nicht erkannt
//String id ;
String busRead, recevedPacket;
// EEPROM VALUES = FOR WATERSENSOR = 1-8
int moisureSensorValueCounter = 0;
int moisureSensorValues[8][5] =
{ {100, 100, 100, 100, 100},
  {100, 100, 100, 100, 100},
  {100, 100, 100, 100, 100},
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
String id = String("#AAAAAA--W") + "008" + "SENSORID" + 50  + "*"; //0,7 //7,10//10,13//13,21//21-* // speicher   1-9 
String id2 = String("#AAAAAA--T") + "008" + "SENSORID" + 50  + "*"; //0,7 //7,10//10,13//13,21//21-* // speicher   10-19 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(1000000);
}


void loop() {
  if (detected == false) {
    writeValues(id); 
    delay(10);
     writeValues(id2);
     delay(10);
  }

  busRead = Serial.readStringUntil('*');  //PrÃ¼ft ob eingabe korrekt endet
  Serial.println(busRead);
  boolean pruefeSensor = checkMoisureSensor(1, analogRead(0));
  if (busRead != "") {
    if (busRead.indexOf('#') > 0 && busRead.indexOf('#') != -1) { // loescht fehlerhaften oder nicht vorhandenem  praefix
      busRead.remove(0, busRead.indexOf('#') );
    }

    if (busRead.startsWith("#AAAAAA")) {          //PrÃ¼ft ob eingabe korrekt startet und korrekt angesprochen wird
      detected = true;
      recevedPacket = busRead;
      startFunction();
    }
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
    if (moisureSensorValues[sensorNumber - 1][i] > EEPROMReadInt(sensorNumber)) { // solange ein sollwert  > messwert ist gieße nicht
      needWater = false;
      break;
    }
  }
  return needWater;

}



