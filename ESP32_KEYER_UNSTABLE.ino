//*********************************************************
//******************acknowledgement************************
//*********************************************************
//This software is based on
//* Arduino iambic CW keyer v2.0
//* for Ham radio usage
//* Richard Chapman
//* KC4IFB
//* February, 2009
//* Richard Chapman KC4IFB published an article in the Sep/Oct 2009 QEX magazine */
//*************************************************************************
//*******************Modified by Warwick Mclean MD7HFH*********************
//*************************   01/09/2020  *********************************
//*************************************************************************
#include "EEPROM.h"
#define EEPROM_SIZE 1024
String letter = "abcdefghijklmnopqrstuvwxyz0123456789.,? ";
String message = "006017021009002020011016004030013018007005015022027010008003012024014025029019063062060056048032033035039047106115076222";

String letraCw = "";
String messageCw = "";
String comando = "";
String sel = "";
String lectura = "";
String valor = "";
int wpm;
int senL;
int senR;
int pRep;
int sound = 0;
int tune = 0;
int Time;
unsigned long Time2 = 0;
unsigned long Time3 = 0;
unsigned long Time4 = 0;
unsigned long Time5 = 0;
unsigned long Time6 = 0;
int beepPin;  //EEPROM Stored Address  Pin D18
int ledPin;   //EEPROM Stored Address 50  Pin D2

//Key Type
int keyerType = 0;  //EEPROM Stored Address 60
const int straight = 1;
const int paddle = 0;

const int on = 1;
const int off = 0;

int a = 0;
int Pada;
int Padb;

unsigned int tSleep;

int DOTIN;
int DASHIN;
int KEYOUTL;

#define IDLE 0
#define DASH 1
#define DOT 2
#define DELAY 3

int dotLength;
int Pdelay;
int Pds;
int dotVal;
int dashVal;
int oldDotVal;
int oldDashVal;
int currEltEndTime;
int currElt;
int nextElt;
int lastElt;
int dit = 56;
int dah = 3 * dit;
int wdsp = 7 * dit;

void setup() {
  setCpuFrequencyMhz(80);
  delay(100);
  EEPROM.begin(EEPROM_SIZE);
  delay(500);
  //if not the first time there will be a 14548
  if (EEPROM.readUInt(5) != 14548) {
    EEPROM.writeUInt(0, 20);  //WPM
    EEPROM.writeUInt(5, 14548);
    EEPROM.writeUInt(30, 0);
    EEPROM.writeUInt(35, 1);  //Pdelay
    EEPROM.writeUInt(40, 1);
    EEPROM.writeUInt(50, 2);
    EEPROM.writeUInt(60, 0);
    EEPROM.writeUInt(70, 32);  //Dit Pin
    EEPROM.writeUInt(80, 21);  //Dah Pin
    EEPROM.writeUInt(90, 0);   //Repeat sequence
    EEPROM.commit();
  }
  a = 0;
  pinMode(16, OUTPUT);        //Jack out to Radio
  digitalWrite(16, LOW);      //Initialise Pin as output - Connected to 2N7000 Transistor
  pinMode(2, OUTPUT);         //LED On Board D2
  pinMode(18, OUTPUT);        //Buzzer Pin D18
  pinMode(22, OUTPUT);        //Bumper Pin for Muting Buzzer
  pinMode(21, INPUT_PULLUP);  //Initialise dit pin as input
  digitalWrite(21, HIGH);
  pinMode(32, INPUT_PULLUP);  //Initialise dah pin as input
  digitalWrite(32, HIGH);
  delay(100);

  lastElt = IDLE;
  currElt = IDLE;
  nextElt = IDLE;
  Time = millis();
  currEltEndTime = Time;
  dotVal = digitalRead(DOTIN);
  dashVal = digitalRead(DASHIN);
  oldDotVal = dotVal;
  oldDashVal = dashVal;
}

void loop() {
  if (a == 0) {
    messageCw = EEPROM.readString(100);
    wpm = EEPROM.readUInt(0);
    if (wpm < 5) wpm = 5;
    if (wpm > 70) wpm = 70;
    dotLength = (1200 / wpm);
    Pdelay = EEPROM.readUInt(35);
    if (Pdelay < 1) Pdelay = 1;
    if (Pdelay > 90) Pdelay = 90;
    Pds = Pdelay * dotLength / 50;
    tSleep = EEPROM.readUInt(30);
    sound = EEPROM.readUInt(40);
    if (sound == on) {
      beepPin = 18;
    } else {
      beepPin = 22;
    };
    ledPin = EEPROM.readUInt(50);
    keyerType = EEPROM.readUInt(60);
    DOTIN = EEPROM.readUInt(70);
    DASHIN = EEPROM.readUInt(80);
    pRep = EEPROM.readUInt(90);
    beeper();
    a = 1;
  }

  if (keyerType == straight) {
    if (digitalRead(DOTIN) == LOW) {
      digitalWrite(KEYOUTL, HIGH);
      digitalWrite(ledPin, HIGH);
      digitalWrite(beepPin, HIGH);
      if (tune == on) {
        menuOptions("tun");
      }
    } else {
      if (tune == off) {
        digitalWrite(KEYOUTL, LOW);
        digitalWrite(ledPin, LOW);
        digitalWrite(beepPin, LOW);
      }
    }
  } else {
    oldDotVal = dotVal;
    oldDashVal = dashVal;
    Pada = digitalRead(DOTIN);
    Padb = digitalRead(DASHIN);
    if ((Pada == 0) && (Padb == 1)) {
      Time5 = millis();
      dotVal = 0;
      dashVal = 1;
    } else if ((Pada == 1) && (Padb == 0)) {
      Time5 = millis();
      dotVal = 1;
      dashVal = 0;
    } else if ((Pada == 0) && (Padb == 0)) {
      if (millis() - Time5 <= Pds) {
      } else {
        dotVal = 0;
        dashVal = 0;
      }
    } else {
      dotVal = 1;
      dashVal = 1;
    }
    Time = millis();
    switch (currElt) {
      case DASH:
        if ((dotVal == LOW) && (nextElt == IDLE)) {
          nextElt = DOT;
        }
        if (Time >= currEltEndTime) {
          lastElt = DASH;
          currElt = DELAY;
          currEltEndTime = Time + dotLength;
        }
        digitalWrite(KEYOUTL, HIGH);
        digitalWrite(ledPin, HIGH);
        digitalWrite(beepPin, HIGH);
        Time2 = millis();
        if (pRep > 0) stopRep();
        break;

      case DOT:
        if ((dashVal == LOW) && (nextElt = IDLE)) {
          nextElt = DASH;
        }
        if (Time >= currEltEndTime) {
          lastElt = DOT;
          currElt = DELAY;
          currEltEndTime = Time + dotLength;
        }
        digitalWrite(KEYOUTL, HIGH);
        digitalWrite(ledPin, HIGH);
        digitalWrite(beepPin, HIGH);
        if (pRep > 0) stopRep();
        break;

      case IDLE:
        if ((dotVal == LOW) && (dashVal == HIGH)) {
          lastElt = IDLE;
          currElt = DOT;
          currEltEndTime = Time + dotLength;
        } else if ((dotVal == HIGH) && (dashVal == LOW)) {
          lastElt = IDLE;
          currElt = DASH;
          currEltEndTime = Time + 3 * dotLength;
        } else if ((dotVal == LOW) && (dashVal == LOW) && (nextElt == IDLE)) {
          lastElt = IDLE;
          currElt = DOT;
          nextElt = DASH;
          currEltEndTime = Time + 3 * dotLength;
        }
        digitalWrite(KEYOUTL, LOW);
        digitalWrite(ledPin, LOW);
        digitalWrite(beepPin, LOW);
        break;

      case DELAY:
        if (Time >= currEltEndTime) {
          currElt = nextElt;
          if (currElt == DOT) {
            currEltEndTime = Time + dotLength;
          } else if (currElt == DASH) {
            currEltEndTime = Time + 3 * dotLength;
          }
          lastElt = DELAY;
          nextElt = IDLE;
        }
        if ((lastElt == DOT) && (dashVal == LOW) && (nextElt == NULL)) {
          nextElt = DASH;
        } else if ((lastElt == DASH) && (dotVal == LOW) && (nextElt == NULL)) {
          nextElt = DOT;
        }
        digitalWrite(KEYOUTL, LOW);
        digitalWrite(ledPin, LOW);
        digitalWrite(beepPin, LOW);
        break;

      default:
        digitalWrite(KEYOUTL, LOW);
        digitalWrite(ledPin, LOW);
        digitalWrite(beepPin, LOW);
        break;
    }
  }
  if (tSleep > 0) {
    if (millis() - Time2 >= (tSleep * 1000)) {
      digitalWrite(2, HIGH);
      digitalWrite(18, HIGH);
      delay(1000);
      esp_deep_sleep_start();
    }
  }
  if (tune == 1) {
    if (millis() - Time6 >= 10000) menuOptions("tun");
  }
  if (pRep > 0) {
    if (millis() - Time3 >= pRep * 1000) Msg();
  }
}

void factory() {
  EEPROM.writeUInt(5, 11111);
  EEPROM.commit();
  comando = "factory";
  command();
  ESP.restart();
}

void stopRep() {
  pRep = 0;
  EEPROM.writeUInt(90, pRep);
  EEPROM.commit();
  if (pRep > 0) messageCw = "";
}

void menuOptions(const char* selected) {
  if (selected == "mes") {
    messageCw = "";
    comando = lectura.substring(3);
    translate();
    Time2 = millis();
    if (pRep > 0) stopRep();
    Msg();
  }

  if (selected == "rep") {
    messageCw = "";
    int i = lectura.length();
    comando = lectura.substring(3, (i - 4));
    valor = lectura.substring((i - 4), (i));
    pRep = valor.toInt();
    translate();
    Time2 = millis();
    EEPROM.writeString(100, messageCw);
    EEPROM.writeUInt(90, pRep);
    EEPROM.commit();
    Msg();
  }

  if (selected == "st1") {
    messageCw = "";
    int i = lectura.length();
    comando = lectura.substring(3, (i - 4));
    valor = lectura.substring((i - 4), (i));
    pRep = valor.toInt();
    translate();
    Time2 = millis();
    EEPROM.writeString(200, messageCw);
    EEPROM.writeUInt(190, pRep);
    EEPROM.commit();
  }

  if (selected == "st2") {
    messageCw = "";
    int i = lectura.length();
    comando = lectura.substring(3, (i - 4));
    valor = lectura.substring((i - 4), (i));
    pRep = valor.toInt();
    translate();
    Time2 = millis();
    EEPROM.writeString(300, messageCw);
    EEPROM.writeUInt(290, pRep);
    EEPROM.commit();
  }

  if (selected == "st3") {
    messageCw = "";
    int i = lectura.length();
    comando = lectura.substring(3, (i - 4));
    valor = lectura.substring((i - 4), (i));
    pRep = valor.toInt();
    translate();
    Time2 = millis();
    EEPROM.writeString(400, messageCw);
    EEPROM.writeUInt(390, pRep);
    EEPROM.commit();
  } else if (selected == "wpm") {
    comando = lectura.substring(3);
    valor = comando.substring(comando.length() - 3);
    wpm = valor.toInt();
    if (wpm < 10) wpm = 10;
    if (wpm > 70) wpm = 70;
    dotLength = (1200 / wpm);
    Pds = Pdelay * dotLength / 50;
    writeMem();
  } else if (selected == "pds") {
    comando = lectura.substring(3);
    valor = comando.substring(comando.length() - 3);
    Pdelay = valor.toInt();
    if (Pdelay < 1) Pdelay = 1;
    if (Pdelay > 90) Pdelay = 90;
    Pds = Pdelay * dotLength / 50;
    writeMem();
  } else if (selected == "ser") {
    comando = lectura.substring(3);
    valor = comando.substring(comando.length() - 3);
    senR = valor.toInt();
    writeMem();
  } else if (selected == "sel") {
    comando = lectura.substring(3);
    valor = comando.substring(comando.length() - 3);
    senL = valor.toInt();
    writeMem();
  } else if (selected == "tun") {
    if (tune == 0) {
      keyerType = 1;
      tune = 1;
      Time6 = millis();
      digitalWrite(16, HIGH);
      digitalWrite(ledPin, HIGH);
      digitalWrite(beepPin, HIGH);
      delay(500);
    } else {
      keyerType = 0;
      tune = 0;
      digitalWrite(16, LOW);
      digitalWrite(ledPin, LOW);
      digitalWrite(beepPin, LOW);
      delay(500);
    }
    writeMem();
  } else if (selected == "sou") {
    if (sound == off) {
      beepPin = 18;
      sound = on;
    }
    writeMem();
  } else if (selected == "led") {
    if (ledPin == 22) {
      ledPin = 2;
    } else {
      ledPin = 22;
    }
    writeMem();
  } else if (selected == "str") {
    if (keyerType == paddle) {
      keyerType = straight;
    } else {
      keyerType = paddle;
    }
    writeMem();
  } else if (selected == "rev") {
    if (DOTIN == 32) {
      DOTIN = 21;
      DASHIN = 32;
    } else {
      DOTIN = 32;
      DASHIN = 21;
    }
    writeMem();
  } else if (selected == "slp") {
    comando = lectura.substring(3);
    valor = comando.substring(comando.length() - 3);
    tSleep = valor.toInt();
    if (tSleep < 60 && tSleep > 0) tSleep = 60;
    writeMem();
  } else if (selected == "res") {
    ESP.restart();
  } else if (selected == "fct") {
    factory();
  } else lectura = "";
}

void translate() {
  for (int i = 0; i < comando.length(); i++) {
    char letraMess = comando.charAt(i);
    for (int n = 0; n < 40; n++) {
      if (letraMess == letter.charAt(n)) {
        letraCw = message.substring(n * 3, n * 3 + 3);
        messageCw = messageCw + letraCw;
      }
    }
  }
}

void writeMem() {
  EEPROM.writeUInt(0, wpm);
  EEPROM.writeUInt(10, senR);
  EEPROM.writeUInt(20, senL);
  EEPROM.writeUInt(30, tSleep);
  EEPROM.writeUInt(35, Pdelay);  //Pdelay
  EEPROM.writeUInt(40, sound);
  EEPROM.writeUInt(50, ledPin);
  EEPROM.writeUInt(60, keyerType);
  EEPROM.writeUInt(70, DOTIN);
  EEPROM.writeUInt(80, DASHIN);
  EEPROM.commit();
  Time2 = millis();
  dotLength = (1200 / wpm);
  Pds = Pdelay * dotLength / 50;
}

void beeper() {
  digitalWrite(2, HIGH);
  digitalWrite(18, HIGH);
  delay(dit);
  digitalWrite(2, LOW);
  digitalWrite(18, LOW);
}

void command() {
  int i = beepPin;
  int u = KEYOUTL;
  KEYOUTL = 22;
  beepPin = 18;
  messageCw = "";
  translate();
  Msg();
  comando = "";
  messageCw = "";
  if (u == 16) KEYOUTL = 16;
  if (i == 22) beepPin = 22;
}

void callback() {
  if (pRep > 0) messageCw = "";
  pRep = 0;
  Time2 = millis();
}

void Msg() {
  int element = 0;
  char temp;
  int LSB = 0;
  int dit = dotLength;
  int dah = 3 * dit;
  int wdsp = 7 * dit;
  Time2 = millis();
  delay(5);
  for (element = 0; element < messageCw.length(); element = element + 3) {
    String subStr = messageCw.substring(element, element + 3);
    temp = subStr.toInt();
    if (temp == 222) {
      delay(7 * dit);
      goto interword;
    }
    for (int x = 0; temp > 1; x++) {
      LSB = bitRead(temp, 0);
      delay(dit);
      if (LSB == 1) {
        digitalWrite(KEYOUTL, HIGH);
        digitalWrite(ledPin, HIGH);
        digitalWrite(beepPin, HIGH);
        delay(dah);
      } else {
        digitalWrite(KEYOUTL, HIGH);
        digitalWrite(ledPin, HIGH);
        digitalWrite(beepPin, HIGH);
        delay(dit);
      }
      temp = temp >> 1;
      digitalWrite(KEYOUTL, LOW);
      digitalWrite(ledPin, LOW);
      digitalWrite(beepPin, LOW);
      if ((digitalRead(DOTIN) == LOW) || (digitalRead(DASHIN) == LOW)) {
        messageCw = "";
        stopRep();
      }
    }
    delay(dah);
interword:
    dotLength = (1200 / wpm);
  }
  delay(5);
  if (pRep == 0) messageCw = "";
  else messageCw = EEPROM.readString(100);
  lectura = "";
  Time3 = millis();
}
