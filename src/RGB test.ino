/*
*    set RGB led legs to;
*    red   --->   digital pin 5 
*    green --->   digital pin 6
*    blue  --->   digital pin 9
*
*/

#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <RGBLED.h>
#include <FanController.h>

RGBLED rgbLedMotor(5,6,9,COMMON_CATHODE);

void setup() {
  // put your setup code here, to run once:
  rgbLedMotor.writeRGB(0,255,0);//green
}

void loop() {
  // put your main code here, to run repeatedly:

}