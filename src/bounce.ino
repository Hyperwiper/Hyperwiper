
#include <Arduino.h>
#include <Bounce.h>

const int buttonPin = 14;
Bounce pushbutton = Bounce(buttonPin, 5);  // 10 ms debounce

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  delay(1000);
  Serial.begin(9600);
  Serial.println("Pushbutton Bounce library test:");
}

byte previousState = HIGH;         // what state was the button last time
unsigned int count = 0;            // how many times has it changed to low
unsigned long countAt = 0;         // when count changed
unsigned int countPrinted = 0;     // last count printed

void loop() {
  if (pushbutton.update()) {
    if (pushbutton.risingEdge()) {
      count = count + 1;
    Serial.println("magnet is leaving");
      countAt = millis();
    }
    if (pushbutton.fallingEdge()) {
    Serial.println("magnet is arrived");
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