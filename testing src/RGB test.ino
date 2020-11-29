/*
  RGBLedExample
  Example for the RGBLED Library
  Created by Bret Stateham, November 13, 2014
  You can get the latest version from http://github.com/BretStateham/RGBLED
*/
#include <RGBLED.h>
#include <Arduino.h>
 #include <SPI.h>
   #include <Audio.h>
  #include <Wire.h>
  #include <SD.h>

RGBLED rgbLedBeat(2,3,4,COMMON_CATHODE);
RGBLED rgbLedMotor(5,6,9,COMMON_CATHODE);
int delayMs = 1000;

void setup() {
  delay(1000);
  Serial.begin(9600);
}

void loop() {
  //Orange
  rgbLedBeat.writeRGB(0,0,255);
  delay(delayMs);
  rgbLedBeat.writeRGB(0,0,0);

  // light green
  rgbLedMotor.writeRGB(0,0,255);
  delay(delayMs);
  rgbLedMotor.writeRGB(0,0,0);

}

