//set drivers 
  #include <SPI.h>
  #include <EEPROMex.h>
  #include <Bounce.h>
  #include <RGBLED.h>

//audio setup
  #include <Audio.h>
  #include <Wire.h>
  #include <SD.h>



int period = 3000;
unsigned long time_now = 0;

void setup() {
    Serial.begin(115200);
}

void loop() {
    time_now = millis();

    Serial.println("Hello");

    while(millis() < time_now + period){

    }
}