//set drivers 
  #include <SPI.h>
  #include <EEPROMex.h>
  #include <Bounce.h>
  #include <RGBLED.h>
  #include <FanController.h>

//audio setup
  #include <Audio.h>
  #include <Wire.h>
  #include <SD.h>

int period = 1000;
int period2 = 2000;
unsigned long time_now = 0;
 
void setup() {
    Serial.begin(115200);
}
 
void loop() {
    if(millis() >= time_now + period + period2){
        time_now += period;
    }

}