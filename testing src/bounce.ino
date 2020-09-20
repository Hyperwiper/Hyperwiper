/* 
 Beats for Hyperwiper(University of Aizu and YR-Design) beat detection done by Julian Villagas

 - uses Teensy 4.0
 - custom board for controlling car windshield wipers.
 - instructions can be found at: Github...

Modified Bounce program for testing clean reading when magnet comes and goes from a reed switch
By Rob Oudendijk C2020 YR=Design
email rob@yr-design.biz 
*/


#include <Arduino.h>
#include <Bounce.h>

//Pins setup
        int beatRedPin = 2;
        int beatGreenPin = 3;
        int beatBluePin = 4;
  
        int motorRedPin = 5;
        int motorGreenPin = 6;
        int motorBluePin = 9;

        int outputPin = 16;
        int potPin=17;



const int buttonPin = 14;
Bounce pushbutton = Bounce(buttonPin, 10);  // 10 ms debounce
byte previousState = HIGH;         // what state was the button last time
unsigned int count = 0;            // how many times has it changed to low
unsigned long countAt = 0;         // when count changed
unsigned int countPrinted = 0;     // last count printed



void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(beatRedPin, OUTPUT);
  pinMode(beatGreenPin, OUTPUT);
  pinMode(beatBluePin, OUTPUT);
  pinMode(motorRedPin, OUTPUT);
  pinMode(motorGreenPin, OUTPUT);
  pinMode(motorBluePin, OUTPUT);
  pinMode(outputPin, OUTPUT);
  pinMode(potPin, INPUT);


  delay(1000);
  Serial.begin(9600);
  Serial.println("Pushbutton Bounce library test:");
}


void loop() {
  if (pushbutton.update()) {
    if (pushbutton.risingEdge()) {
      count = count + 1;
    Serial.println("magnet is leaving");
      digitalWrite( beatRedPin, LOW);
      digitalWrite( beatGreenPin, HIGH);
      countAt = millis();
    }
    if (pushbutton.fallingEdge()) {
    Serial.println("magnet is arrived");
      digitalWrite( beatRedPin, HIGH);
      digitalWrite( beatGreenPin, LOW);
      countAt = millis();
    }
  } else {
    if (count != countPrinted) {
      unsigned long nowMillis = millis();
      if (nowMillis - countAt > 50) {
        Serial.print("count: ");
        Serial.println(count);
        countPrinted = count;
      }
    }
  }
}