// test file for the LEDs

#include <Arduino.h>

//set drivers 
  #include <SPI.h>
  #include <EEPROMex.h>

//audio setup
  #include <Audio.h>
  #include <Wire.h>
  #include <SD.h>  

/*New pins setup for Prototype board
Teensy4 digital in/outs:

OUT:
Motor on/off	=
Motor LED B 	=9
Motor LED G 	=8
Motor LED R 	=7
Beat B 			=4
Beat G			=3
Beat R			=2

IN:
Reed-switch	=
PotentMeter		=

*/


//Pins setup

        int beatRedPin = 2;
        int beatGreenPin = 3;
        int beatBluePin = 4;
  
        int motorRedPin = 7;
        int motorGreenPin = 8;
        int motorBluePin = 9;


void setup() {
        //serial start 
    Serial.begin(9600);
    delay(1000);
    Serial.print("Setup started=");

  pinMode(beatRedPin, OUTPUT);
  pinMode(beatGreenPin, OUTPUT);
  pinMode(beatBluePin, OUTPUT);

  pinMode(motorRedPin, OUTPUT);
  pinMode(motorGreenPin, OUTPUT);
  pinMode(motorBluePin, OUTPUT);

}

void loop() {
    digitalWrite(beatRedPin, HIGH);
    digitalWrite(beatGreenPin, HIGH);
    digitalWrite(beatBluePin, HIGH); 
delay(1000);
    digitalWrite(beatRedPin, LOW);
    digitalWrite(beatGreenPin, LOW);
    digitalWrite(beatBluePin, LOW); 
delay(1000);
    digitalWrite(motorRedPin, HIGH);
    digitalWrite(motorGreenPin, HIGH);
    digitalWrite(motorBluePin, HIGH); 
delay(1000);
    digitalWrite(motorRedPin, LOW);
    digitalWrite(motorGreenPin, LOW);
    digitalWrite(motorBluePin, LOW); 
delay(1000);
}