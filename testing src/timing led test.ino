//set drivers 
  #include <SPI.h>
  #include <EEPROMex.h>
  #include <Bounce.h>
  #include <RGBLED.h>

//audio setup
  #include <Audio.h>
  #include <Wire.h>
  #include <SD.h>
  
  
  
  
int ledState = LOW;  

const unsigned long eventInterval = 5000;
unsigned long previousTime = 0;


//RGB LED pins setup
RGBLED rgbLedBeat(2,3,4,COMMON_CATHODE);
RGBLED rgbLedMotor(5,6,9,COMMON_CATHODE);

unsigned long previous_beatled_Millis = 0;        // will store last time LED was updated
int beatcount =1;

// constants won't change:
const long interval = 1000;           // interval at which to blink (milliseconds)

void setup() {
  rgbLedBeat.writeRGB(0,0,255);
  delay (1000);
}

void loop() {

    if (ledState) {
        rgbLedBeat.writeRGB(0,0,255);
    }
    else{
        rgbLedBeat.writeRGB(0,0,0);
    }

  unsigned long beatledMillis = millis();

  if (beatledMillis - previous_beatled_Millis >= interval) {
    previous_beatled_Millis = beatledMillis;
    if (beatcount==1) {
      ledState=true;
      beatcount++;
    }else{
      ledState=false;
    }
  }
}