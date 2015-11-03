/*
 DFTT v0.1 - Don't Feed The Trolls
             15 oct 2015
             
             Alberto Piganti aka pighixxx & BQ
             alberto.piganti@bq.com

 DFTT is free software: you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public License as
 published by the Free Software Foundation, either version 3 of
 the License, or any later version.
 DFTT is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details at:
 <http://www.gnu.org/licenses/>.

*/

//Includes
#include <SoftwareSerial.h>         //Software Serial
#include <DFPlayer_Mini_Mp3.h>      //DFRobot MP3 Player
#include "moo.h"                    //Pighixxx Scheduler


moo myOS;                           //Create istance of moo
SoftwareSerial MP3Serial(10, 11);   //Create istance of Software Serial

//Constants
const byte LED = 9;                 //LED pin
const byte BUTTON = 3;              //Button pin
const byte STATE = 5;               //State pin
const int LEDTIME = 35;             //LED Timer refresh
const int LEDMIN = -50;             //Min LED Fade
const int LEDMAX = 200;             //Max LED Fade
const int CMDTIME = 200;            //Wait time for serial commands
const int LONGPRESS = 3000;         //Time for detect a long button press (ms)
const int SHORTPRESS = 200;         //Time for detect a short button press (ms)
const int VERYSHORT = 3;            //Number of very short presses
const bool DEBUG = true;            //Debug function

//Variables
int LEDStatus = -50;
int LEDToward = 2;
int shortpresses = 0;
int current;                        //Current state of the Button  
int countseconds = 0;  
int resetseconds = 0;
bool startloop = false; 
byte previous = HIGH;               //Previous state of the Button
unsigned long firstTime;            //How long since the Button was pressed for first time
unsigned long TimePressed;          //How long the Button was held

//Setup
void setup () {
  //Setup pins
  pinMode(BUTTON,INPUT_PULLUP);     //Internal pullups for button
  pinMode(STATE,INPUT);             //MP3 State signal
  pinMode(LED,OUTPUT);              //LED
  digitalWrite(LED,HIGH);           //LED on when initialize system
  myOS.begin();                     //Initialize Scheduler
	Serial.begin (9600);              //Init Serial
  MP3Serial.begin (9600);           //Init Software Serial
  wait();
  if (DEBUG) Serial.println("DEBUG ON");
	mp3_set_serial (MP3Serial);       //Set Serial for DFPlayer module 
  wait();
	//DFPlayer settings
	mp3_set_volume (22);              //Set initial Volume
  wait();
  mp3_set_EQ (3);                   //Set Equalizer
  wait();  
  //Add LED tasks
  myOS.addTask(LedFade,myOS.convertMs(LEDTIME));
  //Add Countdown task
  myOS.addTask(Countdown,myOS.convertMs(1000),PAUSED);
  //Add Blink task
  myOS.addTask(LedBlink,myOS.convertMs(100),PAUSED);
  //Add MP3 Check task
  myOS.addTask(checkState,myOS.convertMs(500),PAUSED);
  //Add Check time
  myOS.addTask(checkTime,myOS.convertMs(1000),PAUSED);
}

//Loop
void loop () {  
  if (digitalRead(STATE)==0) {
    //Music on
  }
  else
  {
    //Button
    current = digitalRead(BUTTON);
    if (current == LOW && previous == HIGH) {
      setFastBlink();                 //Start LED fast blinking
    }
    if (current == LOW && previous == HIGH && millis()- firstTime > SHORTPRESS) {
      firstTime = millis();           //If the buttons becomes pressed remember the time 
    }
    if (current == LOW) {
      TimePressed=(millis()-firstTime);
      //Start Countdown
      if (TimePressed>LONGPRESS && myOS.getTaskStatus(Countdown)==PAUSED) myOS.restartTask(Countdown);
    }
    if (current == HIGH && previous==LOW) {   
      //Reset the counter if the button is not pressed
      if (DEBUG) {Serial.println("");Serial.print("TIME: ");}
      Serial.println(TimePressed);
      if (TimePressed>SHORTPRESS && startloop==false) {
        if (DEBUG) Serial.println("Play KB!");
        //Play the first song
        play_KillBill();
      }
      else
      {
        if (DEBUG) Serial.print("Very short press :)");
        shortpresses++;
        if (myOS.getTaskStatus(checkTime)==PAUSED) myOS.restartTask(checkTime);
        if (shortpresses==VERYSHORT) {
          //play laughter
          if (DEBUG) Serial.print("Play LAUGHTER!");
          play_Laughter();
          shortpresses=0;
        }
      }
      //Stop Countdown Task
      myOS.pauseTask(Countdown);
      restoreFade();
      //Reset variables
      startloop = false;
      TimePressed = 0;
      countseconds = 0;
      //Restore LED
      if (myOS.getTaskStatus(LedBlink)==SCHEDULED) myOS.pauseTask(LedBlink);
    }
    previous = current;
  }
}

//Check if the button is pressed 
void checkTime() {
  myOS.pauseTask(checkTime);
  resetseconds++;
  if (resetseconds==3) {
    //Reset the shortpresses counter
    if (DEBUG) Serial.println("Reset shortpresses counter.");
    shortpresses=0;
    resetseconds=0;
  }
  else
  {
    myOS.restartTask(checkTime);
  }
}

//Countdown function
void Countdown() {
  countseconds++;
  //Pause task
  myOS.pauseTask(Countdown);
    switch (countseconds) {
      case 1:
        //Play Three
        startloop=true;
        play_Three();
        if (DEBUG) Serial.println("Play 3");
        wait();
        break;
      case 2:
        //Play Two
        play_Two();
        if (DEBUG) Serial.println("Play 2");
        wait();
        break;
      case 3:
        //Play One & Music
        play_One();
        if (DEBUG) Serial.println("Play 1");
        break;
      case 4:
        play_Horn();
        if (DEBUG) Serial.println("Play HORN!");
        delay (6500);
        if (DEBUG) Serial.println("Play TROLOLO!");
        play_Trololo();   
        wait();
        break;
    }
    //Restart task
    myOS.restartTask(Countdown);
}

//LED Fade
void LedFade() {
   if (LEDStatus<0) digitalWrite(LED,LOW); else analogWrite(LED,LEDStatus);
   LEDStatus=LEDStatus+LEDToward;
   if (LEDStatus==LEDMIN || LEDStatus==LEDMAX) LEDToward=-LEDToward;
}

//LED Blink
void LedBlink() {
  LEDStatus ^= 1;
  digitalWrite(LED,LEDStatus);
}

//Start Checking
void StartChecking() {
  if (myOS.getTaskStatus(LedBlink)==SCHEDULED) myOS.pauseTask(LedBlink);
  digitalWrite(LED,HIGH);
  if (DEBUG) Serial.println("PLAY");
  if (myOS.getTaskStatus(checkState)==SCHEDULED) myOS.pauseTask(checkState);
  myOS.restartTask(checkState);
}

//Check MP3 State
void checkState() {
  //Pause task
  myOS.pauseTask(checkState);
  //Read the status of MP3
  int play_state = digitalRead(STATE);
  if (play_state==1) {
    //Play ended
    if (DEBUG) Serial.println("PLAY Ended");
    //Reset variables
    countseconds = 0;
    TimePressed = 0;
    startloop=false;
    restoreFade();
  }
  else
  {
  //Restart task
  myOS.restartTask(checkState);
  if (DEBUG) Serial.print(".");
  }
}

//LED Fast Blink
void setFastBlink() {
  //Pause the Fade effect
  myOS.pauseTask(LedFade);
  //Start FastBlink task
  LEDStatus=0;
  myOS.restartTask(LedBlink);
}

//Restore Fade Effect
void restoreFade() {
  //Pause LED Blink tasks
  if (myOS.getTaskStatus(LedBlink)==SCHEDULED) myOS.pauseTask(LedBlink);
  //Restore Fade task
  LEDStatus=-50;
  LEDToward=2;
  myOS.restartTask(LedFade);
}

// Wait the serial
void wait() {
  delay(CMDTIME);
}

//Play the ONE file
void play_One() {
  mp3_play (101);
  wait();
}

//Play the TWO file
void play_Two() {
  mp3_play (102);
  wait();
}

//Play the THREE file
void play_Three() {
  mp3_play (103);
  wait();
}

//Play the Horn file
void play_Horn() {
  mp3_play (100);
  wait();
}

//Play the Trololo file
void play_Trololo() {
  mp3_play (2);
  wait();
  StartChecking();
}

//Play the KillBill file
void play_KillBill() {
  mp3_play (1);
  wait();
  StartChecking();
}

//Play the Laughter file
void play_Laughter() {
  mp3_play (200);
  wait();
  StartChecking();
}
