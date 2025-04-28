//*******************acknowledgement***********************
//This software is based on
//* Arduino iambic CW keyer v2.0
//* for Ham radio usage
//* Richard Chapman KC4IFB
//* February, 2009
//* Richard Chapman KC4IFB published an article in the Sep/Oct 2009 QEX magazine */
// *******************************************************
#include "BluetoothSerial.h"         
#include "EEPROM.h"

#define Threshold 20  

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

String letter = "abcdefghijklmnopqrstuvwxyz0123456789.,? ";
String message = "006017021009002020011016004030013018007005015022027010008003012024014025029019063062060056048032033035039047106115076222";

String letraCw = "";
String messageCw = "";
String comando = "";
String sel = "";
String lectura = "";
String valor = "";

int vel;
int senL;
int senR;
int touchLevel;
int pRep;
int sound = 0;
int learn = 0;
int tune = 0;
int Time;

unsigned long Time2 = 0;
unsigned long Time3 = 0;
unsigned long Time4 = 0;
unsigned long Time5 = 0;
unsigned long Time6 = 0;

int tonePin = 23;
int levelTone = 200;
int beepPin;
int ledPin;
int straight = 0;
int a = 0;
int Pada;
int Padb;

unsigned int tSleep;
unsigned int touchVel = 0;
unsigned int touchMen = 0;

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

void setup()
{
    setCpuFrequencyMhz(80);
    delay(100);
    EEPROM.begin(500);
    delay(100);
    EEPROM.begin(500);
    delay(100);
    EEPROM.readUInt(5);
    delay(100);
    a = EEPROM.readUInt(5);//si no es la primera vez habrá un 14548 (CP Montalban de Cordoba)

    if (a != 14548)
    {
        EEPROM.writeUInt(0, 20);
        EEPROM.writeUInt(5, 14548);
        EEPROM.writeUInt(15, 0);
        EEPROM.writeUInt(25, 40);
        EEPROM.writeUInt(30, 0);
        EEPROM.writeUInt(35, 1);
        EEPROM.writeUInt(40, 1);
        EEPROM.writeUInt(50, 2);
        EEPROM.writeUInt(60, 0);
        EEPROM.writeUInt(70, 32);
        EEPROM.writeUInt(80, 21);
        EEPROM.writeUInt(90, 0);
        EEPROM.commit();
    }
    a = 0;

    ledcSetup(0, 800, 8);
    ledcAttachPin(tonePin, 0);
    pinMode(16, OUTPUT);
    pinMode(2, OUTPUT);
    pinMode(18, OUTPUT);
    pinMode(22, OUTPUT);
    pinMode(21, INPUT_PULLUP);
    pinMode(32, INPUT_PULLUP);
    digitalWrite(16, LOW);
    digitalWrite(21, HIGH);
    digitalWrite(32, HIGH);

    delay(100);
    SerialBT.begin("ESP32_KEYER");
    delay(1000);

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

void loop()
{
    if (a == 0)
    {
        messageCw = EEPROM.readString(100);
        vel = EEPROM.readUInt(0);
        if (vel < 5) vel = 5;
        if (vel > 70) vel = 70;
        dotLength = (1200 / vel);
        Pdelay = EEPROM.readUInt(35);
        if (Pdelay < 1) Pdelay = 1;
        if (Pdelay > 90) Pdelay = 90;
        Pds = Pdelay * dotLength / 50;
        learn = EEPROM.readUInt(15);
        if (learn == 0)
        {
            KEYOUTL = 16;
        }
        else
        {
            KEYOUTL = 22;
        }

        touchLevel = EEPROM.readUInt(25);
        if (touchLevel > 90) touchLevel = 30;
        tSleep = EEPROM.readUInt(30);
        sound = EEPROM.readUInt(40);
        if (sound == 1) beepPin = 18;
        else beepPin = 22;
        ledPin = EEPROM.readUInt(50);
        straight = EEPROM.readUInt(60);
        DOTIN = EEPROM.readUInt(70);
        DASHIN = EEPROM.readUInt(80);
        pRep = EEPROM.readUInt(90);

        beeper();
        a = 1;
    }

    while (SerialBT.available())
    {
        lectura = SerialBT.readStringUntil('#');
        lectura.toLowerCase();
        sel = lectura.substring(0, 3);
        menuBl();
    }

    touchRead(27);
    if (touchRead(27) < touchLevel)
    {
        delay(1);
        if ((touchRead(27) < touchLevel) && (touchRead(13) < touchLevel))
        {
            vel--;
            if (vel < 10) vel = 10;
            dotLength = (1200 / vel);
            beeper();
            delay(100);
            pRep = 0;
            Time4 = millis();
            touchVel = 1;
        }
        if ((touchRead(27) < touchLevel) && (touchRead(13) > touchLevel))
        {
            if (pRep == 0)
            {
                messageCw = EEPROM.readString(200);
                pRep = EEPROM.readUInt(190);
                Time2 = millis();
                EEPROM.writeString(100, messageCw);
                EEPROM.writeUInt(90, pRep);
                EEPROM.commit();
                Msg();
            }
            else
            {
                beeper();
                delay(300);
                stopRep();
            }
        }
    }

    touchRead(4);
    if (touchRead(4) < touchLevel)
    {
        delay(1);
        if ((touchRead(4) < touchLevel) && (touchRead(13) < touchLevel))
        {
            vel++;
            if (vel > 70) vel = 70;
            dotLength = (1200 / vel);
            beeper();
            delay(100);
            pRep = 0;
            Time4 = millis();
            touchVel = 1;
        }
        if ((touchRead(4) < touchLevel) && (touchRead(13) > touchLevel))
        {
            if (pRep == 0)
            {
                messageCw = EEPROM.readString(300);
                pRep = EEPROM.readUInt(290);
                Time2 = millis();
                EEPROM.writeString(100, messageCw);
                EEPROM.writeUInt(90, pRep);
                EEPROM.commit();
                Msg();
            }
            else
            {
                beeper();
                delay(300);
                stopRep();
            }
        }
    }

    touchRead(15);
    if (touchRead(15) < touchLevel)
    {
        delay(1);
        if ((touchRead(15) < touchLevel) && (touchRead(13) < touchLevel))
        {
            touchMenu();
        }
        if ((touchRead(15) < touchLevel) && (touchRead(13) > touchLevel))
        {
            if (pRep == 0)
            {
                messageCw = EEPROM.readString(400);
                pRep = EEPROM.readUInt(390);
                Time2 = millis();
                EEPROM.writeString(100, messageCw);
                EEPROM.writeUInt(90, pRep);
                EEPROM.commit();
                Msg();
            }
            else
            {
                beeper();
                delay(300);
                stopRep();
            }
        }
    }

    if (straight == 1)
    {
        if (digitalRead(DOTIN) == LOW)
        {
            digitalWrite(KEYOUTL, HIGH);
            digitalWrite(ledPin, HIGH);
            digitalWrite(beepPin, HIGH);
            if (tune == 1)
            {
                sel = "tun";
                menuBl();
            }
        }
        else
        {
            if (tune == 0)
            {
                digitalWrite(KEYOUTL, LOW);
                digitalWrite(ledPin, LOW);
                digitalWrite(beepPin, LOW);
            }
        }
    }
    else
    {
        oldDotVal = dotVal;
        oldDashVal = dashVal;
        Pada = digitalRead(DOTIN);
        Padb = digitalRead(DASHIN);
        if ((Pada == 0) && (Padb == 1))
        {
            Time5 = millis();
            dotVal = 0;
            dashVal = 1;
        }
        else if ((Pada == 1) && (Padb == 0))
        {
            Time5 = millis();
            dotVal = 1;
            dashVal = 0;
        }
        else if ((Pada == 0) && (Padb == 0))
        {
            if (millis() - Time5 <= Pds)
            {

            }
            else
            {
                dotVal = 0;
                dashVal = 0;
            }
        }
        else
        {
            dotVal = 1;
            dashVal = 1;
        }
        Time = millis();
        switch (currElt)
        {
            case DASH:
                if ((dotVal == LOW) && (nextElt == IDLE))
                {
                    nextElt = DOT;
                }
                if (Time >= currEltEndTime)
                {
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
                if ((dashVal == LOW) && (nextElt = IDLE))
                {
                    nextElt = DASH;
                }
                if (Time >= currEltEndTime)
                {
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
                if ((dotVal == LOW) && (dashVal == HIGH))
                {
                    lastElt = IDLE;
                    currElt = DOT;
                    currEltEndTime = Time + dotLength;
                }
                else if ((dotVal == HIGH) && (dashVal == LOW))
                {
                    lastElt = IDLE;
                    currElt = DASH;
                    currEltEndTime = Time + 3 * dotLength;
                }
                else if ((dotVal == LOW) && (dashVal == LOW) && (nextElt == IDLE))
                {
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
                if (Time >= currEltEndTime)
                {
                    currElt = nextElt;
                    if (currElt == DOT)
                    {
                        currEltEndTime = Time + dotLength;
                    }
                    else if (currElt == DASH)
                    {
                        currEltEndTime = Time + 3 * dotLength;
                    }
                    lastElt = DELAY;
                    nextElt = IDLE;
                }

                if ((lastElt == DOT) && (dashVal == LOW) && (nextElt == NULL))
                {
                    nextElt = DASH;
                }
                else if ((lastElt == DASH) && (dotVal == LOW) && (nextElt == NULL))
                {
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
    if (tSleep > 0)
    {
        if (millis() - Time2 >= (tSleep * 1000))
        {
            digitalWrite(2, HIGH);
            digitalWrite(18, HIGH);
            delay(1000);
            esp_deep_sleep_start();
        }
    }
    if (tune == 1)
    {
        if (millis() - Time6 >= 10000)
        {
            sel = "tun";
            menuBl();
        }
    }
    if (pRep > 0)
    {
        if (millis() - Time3 >= pRep * 1000) Msg();
    }
    if (touchVel == 1)
    {
        if (millis() - Time4 >= 1000)
        {
            comando = String(vel, DEC);
            command();
            stopRep();
            touchVel = 0;
            dotLength = (1200 / vel);
            writeMem();
        }
    }
}

void factory()
{
    EEPROM.writeUInt(5, 11111);
    EEPROM.commit();
    comando = "factory";
    command();
    ESP.restart();
}

void stopRep()
{
    pRep = 0;
    EEPROM.writeUInt(90, pRep);
    EEPROM.commit();
    if (pRep > 0) messageCw = "";
}

void menuBl()
{
    if (sel == "mes")
    {
        messageCw = "";
        comando = lectura.substring(3);
        translate();
        Time2 = millis();
        if (pRep > 0) stopRep();
        Msg();
    }

    if (sel == "rep")
    {
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

    if (sel == "st1")
    {
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
        comando = "saved 1";
        command();
    }

    if (sel == "st2")
    {
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
        comando = "saved 2";
        command();
    }

    if (sel == "st3")
    {
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
        comando = "saved 3";
        command();
    }

    else if (sel == "vel")
    {
        comando = lectura.substring(3);
        valor = comando.substring(comando.length() - 3);
        vel = valor.toInt();
        if (vel < 10) vel = 10;
        if (vel > 70) vel = 70;
        dotLength = (1200 / vel);
        Pds = Pdelay * dotLength / 50;
        writeMem();
    }

    else if (sel == "pds")
    {
        comando = lectura.substring(3);
        valor = comando.substring(comando.length() - 3);
        Pdelay = valor.toInt();
        if (Pdelay < 1) Pdelay = 1;
        if (Pdelay > 90) Pdelay = 90;
        Pds = Pdelay * dotLength / 50;
        writeMem();
    }

    else if (sel == "ser")
    {
        comando = lectura.substring(3);
        valor = comando.substring(comando.length() - 3);
        senR = valor.toInt();
        writeMem();
    }
    else if (sel == "sel")
    {
        comando = lectura.substring(3);
        valor = comando.substring(comando.length() - 3);
        senL = valor.toInt();
        writeMem();
    }
    else if (sel == "tou")
    {
        comando = lectura.substring(3);
        valor = comando.substring(comando.length() - 3);
        touchLevel = valor.toInt();
        if (touchLevel > 90) touchLevel = 90;
        writeMem();
    }
    else if (sel == "tun")
    {
        if (tune == 0)
        {
            straight = 1;
            tune = 1;
            Time6 = millis();
            digitalWrite(16, HIGH);
            digitalWrite(ledPin, HIGH);
            digitalWrite(beepPin, HIGH);
            delay(500);
        }
        else
        {
            straight = 0;
            tune = 0;
            digitalWrite(16, LOW);
            digitalWrite(ledPin, LOW);
            digitalWrite(beepPin, LOW);
            delay(500);
        }
        writeMem();
    }
    else if (sel == "sou")
    {
        if (sound == 0)
        {
            beepPin = 18;
            sound = 1;
            comando = "on";
            command();
        }
        else
        {
            comando = "off";
            command();
            if (learn == 0)
            {
                beepPin = 22;
                sound = 0;
            }
        }
        writeMem();
    }
    else if (sel == "led")
    {
        if (ledPin == 22)
        {
            ledPin = 2;
            comando = "on";
            command();
        }
        else
        {
            ledPin = 22;
            comando = "off";
            command();
        }
        writeMem();
    }
    else if (sel == "lrn")
    {
        if (learn == 0)
        {
            learn = 1;
            beepPin = 18;
            comando = "learn";
            command();
            KEYOUTL = 22;
            stopRep();
        }
        else
        {
            learn = 0;
            KEYOUTL = 16;
            if (sound == 1) beepPin = 18;
            else beepPin = 22;
            comando = "key";
            command();
            stopRep();
        }
        writeMem();
    }
    else if (sel == "str")
    {
        if (straight == 0)
        {
            straight = 1;
            comando = "straignt";
            command();
        }
        else
        {
            straight = 0;
            comando = "paddle";
            command();
        }
        writeMem();
    }
    else if (sel == "rev")
    {
        if (DOTIN == 32)
        {
            DOTIN = 21;
            DASHIN = 32;
            comando = "reverse";
            command();
        }
        else
        {
            DOTIN = 32;
            DASHIN = 21;
            comando = "direct";
            command();
        }
        writeMem();
    }
    else if (sel == "slp")
    {
        comando = lectura.substring(3);
        valor = comando.substring(comando.length() - 3);
        tSleep = valor.toInt();
        if (tSleep < 60 && tSleep > 0) tSleep = 60;
        writeMem();
    }
    else if (sel == "res")
    {
        comando = "reset";
        command();
        ESP.restart();
    }
    else if (sel == "fct")
    {
        factory();
    }
    else lectura = "";
}

void translate()
{
    for (int i = 0; i < comando.length(); i++)
    {
        char letraMess = comando.charAt(i);
        for (int n = 0; n < 40; n++)
        {
            if (letraMess == letter.charAt(n))
            {
                letraCw = message.substring(n * 3, n * 3 + 3);
                messageCw = messageCw + letraCw;
            }
        }
    }
}

void writeMem()
{
    EEPROM.writeUInt(0, vel);
    EEPROM.writeUInt(10, senR);
    EEPROM.writeUInt(15, learn);
    EEPROM.writeUInt(20, senL);
    EEPROM.writeUInt(25, touchLevel);
    EEPROM.writeUInt(30, tSleep);
    EEPROM.writeUInt(35, Pdelay);
    EEPROM.writeUInt(40, sound);
    EEPROM.writeUInt(50, ledPin);
    EEPROM.writeUInt(60, straight);
    EEPROM.writeUInt(70, DOTIN);
    EEPROM.writeUInt(80, DASHIN);
    EEPROM.commit();
    Time2 = millis();
    dotLength = (1200 / vel);
    Pds = Pdelay * dotLength / 50;
}

void beeper()
{
    digitalWrite(2, HIGH);
    digitalWrite(18, HIGH);
    delay(dit);
    digitalWrite(2, LOW);
    digitalWrite(18, LOW);
}

void command()
{
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

void callback()
{
    if (pRep > 0) messageCw = "";
    pRep = 0;
    Time2 = millis();
}

void touchMenu()
{
    Time4 = millis();
    int i = 1;
    touchMen = 1;
    comando = "tun";
    stopRep();
    beeper();
    delay(1000);
    while (i < 20)
    {
        touchRead(15);
        if (touchRead(15) < touchLevel)
        {
            delay(1);
            if (touchRead(15) < touchLevel)
            {
                i++;
                beeper();
                delay(200);
                if (i > 8) i = 1;
                Time4 = millis();
                touchMen = 1;

                if (i == 1) comando = "tun";
                else if (i == 2) comando = "sou";
                else if (i == 3) comando = "lrn";
                else if (i == 4) comando = "str";
                else if (i == 5) comando = "rev";
                else if (i == 6) comando = "led";
                else if (i == 7) comando = "fact";
                else comando = "pds";
            }
        }
        touchRead(27);
        if (touchRead(27) < touchLevel)
        {
            delay(1);
            if (touchRead(27) < touchLevel)
            {
                Time4 = millis();
                touchMen = 1;
                if (i < 8)
                {
                    if (i == 1) sel = "tun";
                    else if (i == 2) sel = "sou";
                    else if (i == 3) sel = "lrn";
                    else if (i == 4) sel = "str";
                    else if (i == 5) sel = "rev";
                    else if (i == 6) sel = "led";
                    else factory();
                    menuBl();
                    writeMem();
                }
                else
                {
                    Pdelay--;
                    if (Pdelay < 1) Pdelay = 1;
                    beeper();
                    delay(100);
                    comando = String(Pdelay, DEC);
                }
            }
        }
        touchRead(4);
        if (touchRead(4) < touchLevel)
        {
            delay(1);
            if (touchRead(4) < touchLevel)
            {
                Time4 = millis();
                touchMen = 1;
                if (i < 8)
                {
                    if (i == 1) sel = "tun";
                    else if (i == 2) sel = "sou";
                    else if (i == 3) sel = "lrn";
                    else if (i == 4) sel = "str";
                    else if (i == 5) sel = "rev";
                    else if (i == 6) sel = "led";
                    else factory();
                    menuBl();
                    writeMem();
                }
                else
                {
                    Pdelay++;
                    if (Pdelay > 90) Pdelay = 90;
                    beeper();
                    delay(100);
                    comando = String(Pdelay, DEC);
                }
            }
        }
        touchRead(13);
        if (touchRead(13) < touchLevel)
        {
            touchRead(15);
            delay(1);
            if ((touchRead(15) < touchLevel) && (touchRead(13) < touchLevel))
            {
                beeper();
                i = 30;
                delay(1000);
            }
        }
        if ((digitalRead(DOTIN) == LOW) || (digitalRead(DASHIN) == LOW))
        {
            beeper();
            i = 30;
            delay(100);
        }
        if ((millis() - Time4 >= 1000) && (touchMen == 1))
        {
            // command();
            // command();
            stopRep();
            touchMen = 0;
            writeMem();
        }
        if (millis() - Time4 >= 30000)
        {
            beeper();
            i = 30;
        }
        if (tune == 1)
        {
            if (millis() - Time6 >= 10000)
            {
                sel = "tun";
                menuBl();
            }
        }
    }
}

void Msg()
{
    int element = 0;
    char temp;
    int LSB = 0;
    int dit = dotLength;
    int dah = 3 * dit;
    int wdsp = 7 * dit;
    Time2 = millis();
    delay(5);
    for (element = 0; element < messageCw.length(); element = element + 3)
    {
        String subStr = messageCw.substring(element, element + 3);
        temp = subStr.toInt();
        if (temp == 222)
        {
            delay(7 * dit);
            goto interword;
        }
        for (int x = 0; temp > 1; x++)
        {
            LSB = bitRead(temp, 0);
            delay(dit);

            if (LSB == 1)
            {
                digitalWrite(KEYOUTL, HIGH);
                digitalWrite(ledPin, HIGH);
                digitalWrite(beepPin, HIGH);
                delay(dah);
            }
            else
            {
                digitalWrite(KEYOUTL, HIGH);
                digitalWrite(ledPin, HIGH);
                digitalWrite(beepPin, HIGH);
                delay(dit);
            }
            temp = temp >> 1;
            digitalWrite(KEYOUTL, LOW);
            digitalWrite(ledPin, LOW);
            digitalWrite(beepPin, LOW);
            if ((digitalRead(DOTIN) == LOW) || (digitalRead(DASHIN) == LOW))
            {
                messageCw = "";
                stopRep();
            }
        }

        delay(dah);
    interword:
        dotLength = (1200 / vel);
    }
    delay(5);
    if (pRep == 0) messageCw = "";
    else messageCw = EEPROM.readString(100);
    lectura = "";
    Time3 = millis();
}
