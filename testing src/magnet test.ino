/*
  http://www.arduino.cc/en/Tutorial/Debounce
*/
#include <Arduino.h>
//set drivers 
  #include <SPI.h>
  #include <EEPROMex.h>

//audio setup
  #include <Audio.h>
  #include <Wire.h>
  #include <SD.h>



// constants won't change. They're used here to set pin numbers:
const int buttonPin = 14;    // the number of the pushbutton pin
const int ledPin = 2;      // the number of the LED pin

//Pins setup
        // int beatRedPin = 2;
        int beatGreenPin = 3;
        int beatBluePin = 4;
  
        // int motorRedPin = 5;
        int motorGreenPin = 6;
        int motorBluePin = 9;

        // int magnetPin = 14;
        int outputPin = 16;
        int potPin=17;

// Variables will change:
int ledState = LOW;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = HIGH;   // the previous reading from the input pin

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

void setup() {
  //serial start 

  // pinMode(beatRedPin, OUTPUT);
  pinMode(beatGreenPin, OUTPUT);
  pinMode(beatBluePin, OUTPUT);

  // pinMode(motorRedPin, OUTPUT);
  pinMode(motorGreenPin, OUTPUT);
  pinMode(motorBluePin, OUTPUT);

  pinMode(outputPin, OUTPUT);
  // pinMode(magnetPin, INPUT_PULLUP);
  pinMode(potPin, INPUT);

 
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  // set initial LED state
  digitalWrite(ledPin, ledState);

  delay(1000);
  Serial.println("Magnet switch setup done");
}

void loop() {

  int reading = digitalRead(buttonPin);

  if (reading == lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    Serial.println(ledState);
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState ==HIGH) {
        ledState = !ledState;
      }
    }
  }
  
  if(reading ==LOW and lastButtonState ==HIGH) {
    Serial.println("magnet is arriving");
  }
  if(reading ==HIGH and lastButtonState ==LOW) {
    Serial.println("magnet is leaving");
  }

  // set the LED:
  digitalWrite(ledPin, ledState);

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
  delay(10);
}


