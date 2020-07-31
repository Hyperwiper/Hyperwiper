/*
  Beats for DWP (University of Aizu and YR-Design) beat detection done by Julian Villagas

 - uses Ada fruit ssd1351 display and Teensy 3.1
 - custom board for controlling car windshield wipers.
 - instructions can be found at:
 - 
 contact rob@yr-design.biz
 
 revisions:
 V2.0.2   2020-07-29 disabled joy swtices, disable screen
 V2.0.1   2020-06-22 rotate the screen 180 degree
 V2.0.0   2020-06-08 reflected hardware changes for Teensy 4
 V1.2.6   2020-06-05 setup for Teensy 4
 V1.2.5   2015-07-26 made setup switchable
 V1.2.4   2015-07-26 enable setup mode 
 V1.2.3   2015-07-26 added delay for magnet to stop
 V1.1.9   2015-07-25 removed magnet pin and replaced with open collector 2SC1815 over stop switch
 V1.1.8   2015-07-24 management delay start function improve
 V1.1.7   2015-07-24 improve motor stop switch
 V1.1.6   2015-07-24 EEPROM read/write trigger levels
 V1.1.5   2015-07-23 joy switch up/down trigger level setup
 V1.1.4   2015-07-23 Added magnet delay
 V1.1.3   2015-07-23 setup magnetic switch delay
 V1.1.2   2015-07-12 cleanup display
 V1.1.1   2015-07-11 reassigned pins for audio board and joystick and display to work.
 V1.1.0   2015-07-9 fixed stop reset timer, setup init display.
 V1.0.9   2015-07-07 added extra joy switch buttons
 V1.00    2015-02-16 setup for output control relay on D4
 V0.99    2015-02-13 switch keys detecting 
 V0.98    2015-02-12 added LED controls and setup pins for Teensy
 V0.90    2015-02-05 inti for hardware pins changes.

 */

static char VERSION[] = "V2.0.2";;

//set drivers 
  #include <SPI.h>
  #include <EEPROMex.h>


//audio setup
  #include <Audio.h>
  #include <Wire.h>
  #include <SD.h>


//Pins setup

        int outputPin = 4;
        int magnetPin = 5;
        int ledRedPin = 6;
        int ledGreenPin = 2; //Teensy 4


//init variables setup
        boolean redOn;
        boolean greenOn;
        boolean dotOn;
        boolean beatPulse;
        boolean motorOn= false;
        boolean magnetOn= false;
        long previousMillis = 0;
        int ledState = LOW;  
        long minTimeSet=6000;
        int trigger_level = 0;
        //EEPROM setup
        const int maxAllowedWrites = 80;
        const int memBase          = 120;
        int addressLong;
        int runpassed_interval= 180;
        boolean setupmode = false;
    
        //audio hardware setup
        const int myInput = AUDIO_INPUT_LINEIN;
        AudioInputI2S       audioInput; 
        // don't know why this is necessary, but without it the program doesn't run
        AudioOutputI2S     audioOutput;
        AudioMixer4 monoMix;
        AudioAnalyzeBeatEstimation  beat;

        // Create Audio connections 
        AudioConnection c0(audioInput, 0, monoMix, 0);
        AudioConnection c1(audioInput, 1, monoMix, 1);
        AudioConnection c2(monoMix, 0, beat, 0); 
        AudioConnection c3(audioInput, 0, audioOutput, 0);
        AudioConnection c4(audioInput, 0, audioOutput, 1);

        // Create an object to control the audio shield.
        AudioControlSGTL5000 audioShield;


   
void setup() {
      //serial start 
       Serial.begin(9600);
      
      //EEPROM setup
        EEPROM.setMemPool(memBase, EEPROMSizeTeensy3);
        EEPROM.setMaxAllowedWrites(maxAllowedWrites);    
        addressLong   = EEPROM.getAddress(sizeof(long));
        trigger_level = EEPROM.readLong(addressLong);
  

    //display status on serial
    Serial.print("Setup Mode=");
    Serial.println(setupmode);
    Serial.print("Trigger_level=");
    Serial.println((trigger_level)*10000);


     // setup pins for switches
     pinMode(outputPin, OUTPUT);   //wiper motor pin
     pinMode(ledRedPin, OUTPUT);  //red led 
     pinMode(ledGreenPin, OUTPUT); //green led
     // pinMode(magnetPin, INPUT_PULLUP); // needs to be checked

      // attachInterrupt(magnetPin, onMagnet, FALLING);
      delay(1000);
      Serial.println("Magnet switch :OK");


    // Turn off the LEDs.
  
    digitalWrite(ledRedPin, LOW);
    digitalWrite(ledGreenPin, LOW);
    digitalWrite(outputPin, LOW);

      Serial.println("LED off");
  
    // Audio connections require memory to work.  For more
      AudioMemory(100); // the analysis frame (2048 samples) + 1 block (128 samples)
      // Enable the audio shield and set the output volume.
      audioShield.enable();
      audioShield.inputSelect(myInput);
      audioShield.volume(0.5);
  
      
      // reduce the gain on mixer channels
      monoMix.gain(0, 0.2);
      monoMix.gain(1, 0.2);
      
      delay(1000);
      Serial.println("Audio setup finished");
      Serial.println("Init finished");

  run_motor();
}

elapsedMillis fps;
uint8_t cnt=0;


void loop() {
 
       int ac_triggerlevel=((trigger_level)*10000);
      //sets minimum time for wiper for 
       unsigned long currentT = millis();

// Main loop
   if(magnetOn){
        int minTimeSet=6000;
        minloop(minTimeSet);
        magnetOn= true;
       }else{        
        minloop(minTimeSet);
       } 
       
    
    if(beat.available()){  
       uint32_t ac = beat.ac();//2000;
 
        if (ac >(ac_triggerlevel)){
                  Serial.print("ac=");
                  Serial.println(ac);
                  Serial.print("ac_triggerlevel=");
                  Serial.println(ac_triggerlevel);
                  digitalWrite(ledGreenPin,HIGH);
                  run_motor();
                  beatPulse = true;
                }else{
                  digitalWrite(ledGreenPin,LOW);
                  beatPulse = false;
                }
          }
      }



void minloop(long minTime)
      {
      // create the minimum timing for the wiper motor to wait for next beat
      
         unsigned long currentMillis = millis();
          
        if(currentMillis - previousMillis > minTime) {
          previousMillis = currentMillis;  
            run_motor();
        }
      }

void run_motor() {
      digitalWrite(outputPin, HIGH);
      digitalWrite(ledRedPin, HIGH);
         motorOn= true;
         magnetOn=false;
         Serial.println("motor running");

}

void stop_motor() {

          digitalWrite(outputPin, LOW);
          digitalWrite(ledRedPin, LOW);
          motorOn= false; 
           Serial.println("motor stopped");
}

