
#include <UTFT.h>
#include <URTouch.h>
#include <LinkedList.h>

//
boolean stateReady = false;
String state = "INIT";
String prevState = "INIT";


// Declare which fonts we will be using
extern uint8_t BigFont[]; //Smallfont
//extern uint8_t SmallFont[];

// Remember to change the model parameter to suit your display module!
UTFT myGLCD(SSD1963_800, 38, 39, 40, 41); // (byte model, int RS, int WR,int CS, int RST, int SER) :
URTouch  myTouch( 6, 5, 4, 3, 2); // InitTouchscreen

//SENSORVARIABLEN
//FEUCHTIGKEIT
LinkedList<String> moisure;
LinkedList<String> temperatur;

//COMMUNICATION
String busRead = ""; //Initialized variable to store recieved data
String recevedPacket = "";
LinkedList<String> sensorList;
LinkedList<String> sensorTypes;
String activeSensor; // variable contains the selected sensor
//Draw menu
boolean isDrawed = false;
LinkedList<String> homeButtonList;
LinkedList<String> controlButtonList;



void setup()
{
  homeButtonList.add("Status");
  homeButtonList.add("Automate");
  homeButtonList.add("Control");
  homeButtonList.add("Add Sensor");

  controlButtonList.add("Watering");
  controlButtonList.add("Temperatur");
  //randomSeed(analogRead(0));

  Serial1.begin(9600);
  Serial1.setTimeout(50); //sonst wartet er beim read until zu lange und empfängt nichts vom sensor
  Serial.begin(9600);
  // Setup the LCD
  myGLCD.InitLCD(); //landscape
  myGLCD.setFont(BigFont);
  myGLCD.clrScr();
  // SETUP TOUCH
  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM); //TOUCHGENAUIGKEIT
  //
  startScreen();
}

void loop()
{
  //Communication

  busRead = Serial1.readStringUntil('*');  //Prüft ob eingabe korrekt endet

  if (busRead != "" && busRead.indexOf('#') != -1) {
    if (busRead.indexOf('#') > 0 ) { // loescht fehlerhaften praefix
      busRead.remove(0, busRead.indexOf('#') );
    }

    if (busRead.startsWith("#") ) {          //Prüft ob eingabe korrekt startet und ob # vorhanden
      recevedPacket = busRead;
          Serial.println(recevedPacket);
      if (recevedPacket.substring(13, 16).equals("SET")) {
        readValues(recevedPacket);
      }
    }
  }

  //int buf[798];
  //int r;
  long xTouch, yTouch;

  // Clear the screen and draw the frame

  if (myTouch.dataAvailable() == true) {
    myTouch.read();
    xTouch = myTouch.getX();
    yTouch = myTouch.getY();
    //Serial.println(state);
    if ((xTouch != -1) and (yTouch != -1))
    {
      drawMenu(state, xTouch, yTouch);// touchAction(xTouch, yTouch);
      navigationAction(xTouch, yTouch);

    }
  }
  // Draw some random rounded rectangles
}


void addSensor() {
  String sensorID = recevedPacket.substring(0, 11);
  String listSensorId;
  boolean idAvailable = false;
  for (int i = 0 ; i < sensorList.size(); i++) {
    listSensorId = sensorList.get(i).substring(0, 11);
    if (listSensorId.equals(sensorID)) {
      idAvailable = true;
      break;
    }
  }

  if (idAvailable == false ) {
    // Serial.println(recevedPacket);
    sensorList.add(recevedPacket);
  }
  for (int i = 0; i < sensorList.size(); i++) {
    myGLCD.print(sensorList.get(i).substring(0, 13), 230, (i * 20) + 10);
    //writeValues(sensorID);
  }
}


String getSensorWithID(String id) {
  for (int i = 0 ; i < sensorList.size(); i++) {
    if (sensorList.get(i).substring(0, id.length()).equals(id)) {
      return sensorList.get(i);
    }
  }
}

String formatNumber(int number) {
  String numberAsString(number);
  if (number < 100) {
    numberAsString = "0" + numberAsString;
    if (number < 10) {
      numberAsString = "0" + numberAsString;
    }
  }
  return numberAsString;
}



//sensorTypes.get(i).substring(7 , 10).equals(Sensors.get(i).substring(7 , 10))

void getSensorTypes(String sensorID, LinkedList<String> &Sensors) {
  sensorTypes.clear();
  boolean sensorAlreadyAvailable;
  for (int i = 0 ; i < Sensors.size(); i++) {
    sensorAlreadyAvailable = false;
    if (sensorID.equals(Sensors.get(i).substring(0 , 7)) ) {
      //Serial.print(Sensors.get(i).substring(7 , 10));
      for (int j = 0; j < sensorTypes.size(); j++) {
        if (sensorTypes.get(j).substring(0 , 10).equals(Sensors.get(i).substring(0, 10))) {
          sensorAlreadyAvailable = true;
          break;
        }
      }
    }
    if (sensorAlreadyAvailable == false) {
      //Serial.print(Sensors.get(i).substring(7 , 10));
      sensorTypes.add(Sensors.get(i).substring(7 , 10));

    }
  }

  // return sensorTypes;
}





// holt sensoren vom typ xy
LinkedList<String> getSensorByType(String type, LinkedList<String> Sensors) {
  LinkedList<String> tempSensorList;
  String sensorType;
  for (int i = 0; i < Sensors.size(); i++) {
    sensorType = Sensors.get(i).substring(7, 10);
    //Serial.print(sensorType);
    // Serial.print("\n");
    if (sensorType.equals(type)) {
      // String newName=
      Sensors.add(Sensors.get(0).substring(0, 7));
    }
  }
  return tempSensorList;
}

// holt sensoren vom ein mal auch wenn 2 typen vorhanden sind 
LinkedList<String> getSensorByName() {
  LinkedList<String> sensorNames;
  boolean sensorAvailable;
  for (int i = 0; i < sensorList.size(); i++) {
    sensorAvailable=false;
     for (int j = 0; j < sensorNames.size(); j++) {
         if(sensorNames.get(j).substring(0,7).equals(sensorList.get(i).substring(0,7))){
          sensorAvailable=true;
          break;
          }
      }
      if(sensorAvailable==false){
         sensorNames.add(sensorList.get(i).substring(0,7));
        }
  }
  return sensorNames;
}





// holt sensorwert von einem sensor
String getSensorValue(String id, LinkedList<String> &Sensors) {
  String listSensorId;
  for (int i = 0 ; i < Sensors.size(); i++) {
    if (Sensors.get(i).substring(0, 13).equals(id)) {
      return Sensors.get(i).substring(21);
      break;
    }
  }
}




void readValues(String recevedPacket) {
  if (recevedPacket.substring(13, 21).equals("SETWATER")) {
    isDrawed = false;
    for (int i = 0; i < sensorList.size(); i++) {
      if (sensorList.get(i).substring(0, 13).equals(recevedPacket.substring(0, 13))) {
        sensorList.set(i, recevedPacket);
        Serial.println(sensorList.get(i));
        drawMenu("WATERSENSOR", -1, -1);// touchAction(xTouch, yTouch);
      }
    }
  }
}

void writeValues(String sendPacket) {
  if (sendPacket != "") {
    if (sendPacket.startsWith("#") == false) {
      sendPacket = "#" + sendPacket;
    }
    if (sendPacket.endsWith("*") == false)
    {
      sendPacket = sendPacket + "*";
    }
    Serial1.println(sendPacket);
    Serial.println(sendPacket);
  }
}



//drawframeandHeadline
void startScreen() {
  myGLCD.clrScr();
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setColor(255, 255, 0);
  myGLCD.print("Willkommen in deinem Autonomen Garten", CENTER, 220);
  delay(500);
  myGLCD.clrScr();
  drawMenu("HOME", -1, -1);
  navigartion();
}

void navigartion() {
  int x, x2;
  int y, y2;
  myGLCD.setColor(0, 0, 255);
  myGLCD.drawLine(0, 440, 800, 440);

  // Draw menueraster with rounded rectangles
  for (int i = 0; i < 2; i++)
  {
    x = 5 + (i * 100);
    y = 445;
    x2 = (i + 1) * (100);
    y2 = 475;
    myGLCD.setColor(0, 255, 0);
    myGLCD.fillRect(x, y, x2, y2);
    myGLCD.setColor(0, 0, 255); //SCHRIFTFARBE
    myGLCD.setBackColor(0, 255, 0);
    switch (i) {
      case 0:   myGLCD.print("Back ", x + 10,   y + 10); break;
      case 1:   myGLCD.print("Home", x + 10,   y + 10); break;
      default: printf(""); break;
    }
  }
  if (state.equals("WATERSENSOR")|| state.equals("TEMPERATUR") ) {
    for (int i = 0; i < 2; i++)
    {
      x = 800 - (i + 1 * 100) - 5;
      y = 445;
      x2 = 800 - (i * 100) - 5;
      y2 = 475;
      myGLCD.setColor(0, 255, 0);
      myGLCD.fillRect(x, y, x2, y2);
      myGLCD.setColor(0, 0, 255); //SCHRIFTFARBE
      myGLCD.setBackColor(0, 255, 0);
      switch (i) {
        case 0:   myGLCD.print("Save", x + 10,   y + 10);
      }
    }
  }

}

void drawHome(int xTouch, int yTouch) {
  int x, x2;
  int y, y2;
  // Draw menÃ¼raster with rounded rectangles
  for (int i = 0; i < 4; i++) {
    x = 5;
    y = 10 + (i * 75);
    x2 = 5 + 200;
    y2 = ((i + 1) * 75);
    if (isDrawed == false) {
      myGLCD.setColor(0, 255, 0);
      myGLCD.fillRect(x, y, x2, y2);
      myGLCD.setColor(0, 0, 255); //SCHRIFTFARBE
      myGLCD.drawLine(210, 0, 210, 440);
      myGLCD.setBackColor(0, 255, 0);
      myGLCD.print(homeButtonList.get(i), x + 20,   y + 20);
    } else {
      if (xTouch > x && xTouch < x2  && yTouch > y  && yTouch < y2 ) {
        switch (i) {
          case 0:   drawMenu("STATUS", -1, -1); break;
          case 1:   drawMenu("AUTOMATE", -1, -1); break;
          case 2:   drawMenu("CONTROL", -1, -1); break;
          case 3:   String sensorID = recevedPacket; //Sensor empfangen
          Serial.println(recevedPacket);
            if (!recevedPacket.equals("")) {
              addSensor();
            } break;
        }
      }
    }
  }
  isDrawed = true;
}




void drawControl(int xTouch, int yTouch) {
  int x, x2;
  int y, y2;
  // Draw menÃ¼raster with rounded rectangles
   LinkedList<String> sensorNameList= getSensorByName();
  for (int i = 0; i < sensorNameList.size(); i++) {
    x = 5;
    y = 10 + (i * 75);
    x2 = 5 + 200;
    y2 = ((i + 1) * 75);
    if (isDrawed == false) {
      myGLCD.setColor(0, 255, 0);
      myGLCD.fillRect(x, y, x2, y2);
      myGLCD.setColor(0, 0, 255); //SCHRIFTFARBE
      myGLCD.drawLine(210, 0, 210, 440);
      myGLCD.setBackColor(0, 255, 0);
      myGLCD.print(sensorNameList.get(i).substring(0, 7), x + 20,   y + 20);
    } else {
      if (xTouch > x && xTouch < x2  && yTouch > y  && yTouch < y2 ) {
        activeSensor = sensorNameList.get(i).substring(0, 7);
        drawMenu("SENSORTYPES", -1, -1); break;
      }
    }
  }
  isDrawed = true;
}



void drawSensorTypes(int xTouch, int yTouch) {
  //LinkedList<String> tempSensorList = sensorList;//
  getSensorTypes(activeSensor, sensorList);
  int x, x2;
  int y, y2;
  // Draw menÃ¼raster with rounded rectangles
  for (int i = 0; i < sensorTypes.size(); i++) {
    x = 5;
    y = 10 + (i * 75);
    x2 = 5 + 200;
    y2 = ((i + 1) * 75);
    if (isDrawed == false) {
      myGLCD.setColor(0, 255, 0);
      myGLCD.fillRect(x, y, x2, y2);
      myGLCD.setColor(0, 0, 255); //SCHRIFTFARBE
      myGLCD.drawLine(210, 0, 210, 440);
      myGLCD.setBackColor(0, 255, 0);
      if (sensorTypes.get(i).equals("--W")) {
        myGLCD.print("WATER", x + 20,   y + 20);
      }
      if (sensorTypes.get(i).equals("--T")) {
        myGLCD.print("TEMPERATUR", x + 20,   y + 20);
      }

    } else {
      if (xTouch > x && xTouch < x2  && yTouch > y  && yTouch < y2 ) {

        activeSensor.substring(7,10) = sensorTypes.get(i); // setzt bzw ersetzt sensortyp
        if (sensorTypes.get(i).equals("--W")) {
          drawMenu("WATERSENSOR", -1, -1);
        }
        if (sensorTypes.get(i).equals("--T")) {
          drawMenu("TEMPERATUR", -1, -1);
        }

      }
    }
  }
  isDrawed = true;
}

// Zeichne alle sensoren vom typ wasser /Löschen?
void drawWatering(int xTouch, int yTouch) {
  int x, x2;
  int y, y2;
  String sensorName;
  // Draw menuraster with rounded rectangles
  LinkedList<String> waterSensores = getSensorByType("--W", sensorList);
  for (int i = 0; i < waterSensores.size(); i++) {
    x = 5;
    y = 10 + (i * 75);
    x2 = 5 + 200;
    y2 = ((i + 1) * 75);
    if (isDrawed == false) {
      myGLCD.setColor(0, 255, 0);
      myGLCD.fillRect(x, y, x2, y2);
      myGLCD.setColor(0, 0, 255); //SCHRIFTFARBE
      sensorName = waterSensores.get(i).substring(0, 7);
      myGLCD.print(sensorName, x + 20,   y + 20);
    } else {
      if (xTouch > x && xTouch < x2  && yTouch > y  && yTouch < y2 ) {
        activeSensor = waterSensores.get(i).substring(0, 7);
        drawMenu("WATERSENSOR", -1, -1);
      }
    }
  }
  isDrawed = true;
}




void drawWaterSensor(int xTouch, int yTouch) {
  int x, x2, xside, x2side;
  int y, y2 ;
  int sensorNumber = getSensorWithID(activeSensor).substring(10, 13).toInt();
  int moisureAsInt;

  for (int i = 0; i < sensorNumber; i++) {
    int spalte = i / 5; // spalten bis zu 5 sensoren pro reihe
    y = 10 + ((i % 5) * 75);
    y2 = y + 50;
    x = 55 + (spalte * 400);
    x2 = x + 250; // breite von button
    xside = x - 50;
    x2side = x2 + 5;
    if (isDrawed == false) {
      moisure.add("50");
      myGLCD.setColor(0, 255, 0);
      myGLCD.fillRect(x, y, x2, y2);
      myGLCD.fillRect(x2side , y, x2side + 40, y2);
      myGLCD.fillRect(xside, y,  xside + 40 , y2);
      myGLCD.setColor(0, 0, 255); //SCHRIFTFARBE
      myGLCD.print("Moisure in % " , x + 1, y );
      myGLCD.print("new: " + moisure.get(i), x + 5 ,   y + 20);
      myGLCD.print("+", x2side + 15,   y + 20);
      myGLCD.print("-", xside + 15,   y + 20);
    } else {
      if (xTouch >  x2side && xTouch <  x2side + 40  && yTouch > y  && yTouch < y2 ) {
        if (moisure.get(i).toInt() < 99) {
          moisureAsInt = moisure.get(i).toInt();
          moisureAsInt++;
          moisure.set(i, String(moisureAsInt));
        }
      }
      if (xTouch > xside && xTouch < xside + 40 && yTouch > y  && yTouch < y2 ) {
        if (moisure.get(i).toInt() > 0) {
          moisureAsInt = moisure.get(i).toInt();
          moisureAsInt--;
          moisure.set(i, String(moisureAsInt));
        }
      }
      myGLCD.setColor(0, 0, 255); //SCHRIFTFARBE
      //myGLCD.print("is " + getSensorValue(activeSensor  + "000", sensorList), x + 1, y + 20 );
      myGLCD.print("new: " + moisure.get(i), x + 5 ,   y + 20);
    }
  }
  isDrawed = true;
}




void drawTemperaturSensor(int xTouch, int yTouch) {
  int x, x2, xside, x2side;
  int y, y2 ;
  int sensorNumber = getSensorWithID(activeSensor).substring(10, 13).toInt();
  int temperaturAsInt;

  for (int i = 0; i < sensorNumber; i++) {
    int spalte = i / 5; // spalten bis zu 5 sensoren pro reihe
    y = 10 + ((i % 5) * 75);
    y2 = y + 50;
    x = 55 + (spalte * 400);
    x2 = x + 250; // breite von button
    xside = x - 50;
    x2side = x2 + 5;
    if (isDrawed == false) {
      temperatur.add("20");
      myGLCD.setColor(0, 255, 0);
      myGLCD.fillRect(x, y, x2, y2);
      myGLCD.fillRect(x2side , y, x2side + 40, y2);
      myGLCD.fillRect(xside, y,  xside + 40 , y2);
      myGLCD.setColor(0, 0, 255); //SCHRIFTFARBE
      myGLCD.print("Grad in Celsius" , x + 1, y );
      myGLCD.print("new: " + temperatur.get(i), x + 5 ,   y + 20);
      myGLCD.print("+", x2side + 15,   y + 20);
      myGLCD.print("-", xside + 15,   y + 20);
    } else {
      if (xTouch >  x2side && xTouch <  x2side + 40  && yTouch > y  && yTouch < y2 ) {
        if (temperatur.get(i).toInt() < 99) {
          temperaturAsInt = temperatur.get(i).toInt();
          temperaturAsInt++;
          temperatur.set(i, String(temperaturAsInt));
        }
      }
      if (xTouch > xside && xTouch < xside + 40 && yTouch > y  && yTouch < y2 ) {
        if (temperatur.get(i).toInt() > 0) {
          temperaturAsInt = temperatur.get(i).toInt();
          temperaturAsInt--;
          temperatur.set(i, String(temperaturAsInt));
        }
      }
      myGLCD.setColor(0, 0, 255); //SCHRIFTFARBE
      myGLCD.print("new: " + temperatur.get(i), x + 5 ,   y + 20);
    }
  }
  isDrawed = true;
}


void navigationAction(int xTouch, int yTouch) {
  if (xTouch > 5 && xTouch < 100  && yTouch > 445  && yTouch < 475 ) {
    drawMenu(prevState, -1, -1);
  }
  else if (xTouch > 105 && xTouch < 200  && yTouch > 445  && yTouch < 475 ) {
    drawMenu("HOME", -1, -1);
  }
  else if (xTouch > 695 && xTouch < 795  && yTouch > 445  && yTouch < 475 ) {
   // if (state.equals("WATERSENSOR")){
      int sensorNumber = getSensorWithID(activeSensor).substring(10, 13).toInt();
      for (int i = 0; i < sensorNumber; i++) {
       if (state.equals("WATERSENSOR")){ writeValues(activeSensor  +"--W"+ formatNumber(i) + "SETWATER" + moisure.get(i) );}
       if (state.equals("TEMPERATUR")){ writeValues(activeSensor  +"--T"+ formatNumber(i) + "SETTEMPE" + temperatur.get(i) );}
        delay(10); // delay notwendig sonst macht empfänger probleme und empfängt maximal 4 strings 
        
      }
    }
 // }
}

void drawMenu(String menu, int xTouch, int yTouch) {
  delay(100);
  if (menu != state) {
    prevState = state; // SPEICHER ALTEN state vor wechsel
    myGLCD.clrScr();
    isDrawed = false;
  }
  state = menu; // setze neuen state
  navigartion();

  if (menu == "HOME") {
    drawHome(xTouch, yTouch);
  }
  if (menu == "AUTOMATE") {}
  if (menu == "CONTROL") {
    drawControl(xTouch, yTouch);
  }
  //  if (menu == "WATERING") {
  //  drawWatering(xTouch, yTouch);
  //}
  if (menu == "TEMPERATUR") {
    drawTemperaturSensor(xTouch, yTouch);
  }
  if (menu == "SENSORTYPES") {
    drawSensorTypes(xTouch, yTouch);
  }

  if (menu == "WATERSENSOR") {
    drawWaterSensor(xTouch, yTouch);
  }
}





