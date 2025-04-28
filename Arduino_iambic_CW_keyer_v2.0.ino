/*
* Arduino iambic CW keyer v2.0
* for Ham radio usage
* Richard Chapman
* KC4IFB
* February, 2009
* Richard Chapman KC4IFB published an article in the Sep/Oct 2009 QEX magazine */
// *******************************************************
//  Iambic Morse Code Keyer Sketch with added Side-Tone
// added Message keyer and enable poteniometer speed control
//  Copyright (c) Bob Anding 2017
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details:
//
//  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
//  Boston, MA  02111-1307  USA
//*********************************************************

// I/O pin numbers
#define SPEEDIN 0  // analog pin to read the speed value
#define DOTIN 11   // high -> dot paddle closed
#define DASHIN 12  // high -> dash paddle closed
#define KEYOUT 13  // drives the 2N2222 that closes the key connection

// state of the machine
#define IDLE 0   // doing nothing
#define DASH 1   // playing a dash
#define DOT 2    // playing a dot
#define DELAY 3  // in the dot-length delay between two dot/dashes

// Analog pin for WPM
int sensorPin = A0;  // set input pin for the potentiometer
int dotLength;       // length of a dot in milliseconds
int dotVal;          // value of the dot paddle this sycle of main loop
int dashVal;         // value of the dash paddle this cycle of main loop
int oldDotVal;       // value of dot paddle last cycle
int oldDashVal;      // value of dash paddle last dial
int speedDial;       // raw value read from the potentiometer for speed
int currEltEndTime;  // what time did the current element start    sounding (in milliseconds since powerup)
int currElt;         // sate of what the keyer output is sending right now
int nextElt;         // state the keyer will go into when current element ends
int lastElt;         // previous state of the keyer
int time;
int tonePin = 4;
//// Message Block
int msgPin = 3;                                                                                     // message  input
char message[] = { 0x15, 0x1b, 0x15, 0x1b, 0x15, 0x1b, 0x09, 0x02, 0x06, 0x06, 0x20, 0x0f, 0x1d };  //message = "CQ CQ CQ DE AA5OY"
int msglength = sizeof(message) / sizeof(message[0]);                                               //
int dit = 56;                                                                                       // adjust WPM speed here (Slow =  100, Fast = 35)
int dah = 3 * dit;                                                                                  // sets dah lenght as a fuction of dit length
int wdsp = 7 * dit;                                                                                 // sets word space  lenght as a fuction of dit length
//////////////////////////////////
void setup()  // run once, when the sketch starts
{
  pinMode(KEYOUT, OUTPUT);  // sets modes on the i/o pins
  pinMode(DOTIN, INPUT);
  pinMode(DASHIN, INPUT);
  pinMode(msgPin, INPUT_PULLUP);
  // no need to set SPEED pin as input since it is analog
  digitalWrite(KEYOUT, LOW);  // initially the key is open

  // activate internal pullup resistors
  digitalWrite(DOTIN, HIGH);
  digitalWrite(DASHIN, HIGH);

  // initialize the state variables
  lastElt = IDLE;
  currElt = IDLE;
  nextElt = IDLE;
  time = millis();
  currEltEndTime = time;
}

void loop()  // run over and over again
{
  loadWPM((int)(analogRead(sensorPin) * (20. / 1023.) + .5));  //loadWPM(15); // Fix speed at 15 WPM
  dotLength = ((analogRead(sensorPin) * (65. / 1023.) + 35));  //load CW message WPM
  if ((digitalRead(msgPin) == LOW)) Msg();                     // if message is low play message
  // get the speed (need to do this every iteration)

  // read the paddles
  oldDotVal = dotVal;  // save the old values to detect a transition, so you can debounce
  oldDashVal = dashVal;
  dotVal = digitalRead(DOTIN);  // read the current values of the paddles
  dashVal = digitalRead(DASHIN);

  // short delay for a debounce  -- turns out not needed
  //if ((oldDotVal != dotVal) || oldDashVal != dashVal) {
  //    delay(5);
  //}

  //get the current time
  time = millis();
  /******************************
      STATE MACHINE FOR PADDLES 
    ** ***************************/

  switch (currElt) {  // cases based on what current state is

    case DASH:
      if ((dotVal == LOW) && (nextElt == IDLE)) {  // going from dash to iambic mode
        nextElt = DOT;
      }
      if (time >= currEltEndTime) {  // at end of current dash
        lastElt = DASH;              // a delay will follow the dash
        currElt = DELAY;
        currEltEndTime = time + dotLength;
      }
      digitalWrite(KEYOUT, HIGH);  // close the keyer output while the dash is being sent
      tone(4, 800);
      break;
    case DOT:
      if ((dashVal == LOW) && (nextElt = IDLE)) {  // going from dot to iambic mode
        nextElt = DASH;
      }
      if (time >= currEltEndTime) {  // at end of current dot
        lastElt = DOT;               // a delay will follow the dot
        currElt = DELAY;
        currEltEndTime = time + dotLength;
      }
      digitalWrite(KEYOUT, HIGH);  // close the keyer outout while the dot is being sent
      tone(4, 800);
      break;

    case IDLE:                                     // not sending, nor finishing the delay after a dot or dash
      if ((dotVal == LOW) && (dashVal == HIGH)) {  // only dot paddle pressed, go to DOT mode
        lastElt = IDLE;
        currElt = DOT;
        currEltEndTime = time + dotLength;
      } else if ((dotVal == HIGH) && (dashVal == LOW)) {  // only dash paddle pressed, go to DASH mode
        lastElt = IDLE;
        currElt = DASH;
        currEltEndTime = time + 3 * dotLength;
      } else if ((dotVal == LOW) && (dashVal == LOW) && (nextElt == IDLE)) {
        // if both paddles hit at same time (rare, but happens)
        lastElt = IDLE;
        currElt = DOT;
        nextElt = DASH;
        currEltEndTime = time + 3 * dotLength;  // it is an iambic keyer, not a trochaic keyer
      }
      digitalWrite(KEYOUT, LOW);  // keyer output is open in IDLE mode
      noTone(4);
      break;

    case DELAY:                      // waiting for a dot-length delay after sending a dot or dash
      if (time >= currEltEndTime) {  // check to see if there is a next element to play
        currElt = nextElt;
        if (currElt == DOT) {
          currEltEndTime = time + dotLength;
        } else if (currElt == DASH) {
          currEltEndTime = time + 3 * dotLength;
        }
        lastElt = DELAY;
        nextElt = IDLE;
      }
      // during the delay, if either paddle is pressed, save it to play after the delay
      if ((lastElt == DOT) && (dashVal == LOW) && (nextElt == NULL)) {
        nextElt = DASH;
      } else if ((lastElt == DASH) && (dotVal == LOW) && (nextElt == NULL)) {
        nextElt = DOT;
      }
      // key output is open during the delay
      digitalWrite(KEYOUT, LOW);
      noTone(4);
      break;
    default:
      noTone(4);
      digitalWrite(KEYOUT, LOW);
      break;
  }
}

void loadWPM(int wpm) {
  dotLength = 1200 / wpm;
}

void Msg() {
  //pinMode(tonePin,OUTPUT); // AUDIO OUTPUT pin
  //pinMode(13,OUTPUT); // PTT OUTPUT pin
  int element = 0;  // letter of message
  char temp;
  int LSB = 0;
  int dit = dotLength;  // use pot to reset WPM of messgae
  //digitalWrite(PTT, HIGH); // turn on PTT
  delay(500);  //
  for (element = 0; element < msglength; element++)  //  read cw message
  {
    temp = message[element];
    for (int x = 0; temp > 1; x++)  // read cw message and check for EOM = 0xff// while(temp !=1) //  End of Character = 1
    {
      LSB = bitRead(temp, 0);  //LSB of temp, each loop shifts right one
      delay(dit);              //delay(dah * 1.3); //add element space
      if (LSB == 1) {
        digitalWrite(KEYOUT, HIGH);  // turn on PTT
        tone(tonePin, 800);          //tone(A5,800,dah); //send dah, tone(pin, frequency, duration)
        delay(dah);                  //delay(dah); //add element space
        noTone(tonePin);
      } else {
        digitalWrite(KEYOUT, HIGH);  // turn on PTT
        tone(tonePin, 800);          //tone(A5,800,dit); //send dit, tone(pin, frequency, duration)
        delay(dit);                  //add element space
        noTone(tonePin);
      }
      //}
      temp = temp >> 1;           // shift right one, when temp = 1 quit loop
      digitalWrite(KEYOUT, LOW);  // turn off PTT
    }
    delay(dah);  //add dah space
  }
  delay(500);  // wait
}
