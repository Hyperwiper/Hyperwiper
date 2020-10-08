/*
  Beats for Hyperwiper(University of Aizu and YR-Design) beat detection done by Julian Villagas

 - uses Teensy 4.0
 - custom board for controlling car windshield wipers.
 - instructions can be found at: Github...
 
 contact rob@yr-design.biz
 
 revisions:
 V2.0.7   2020-10-08 added test for startup calculating the startup deley ansd storing in EEPROM
 V2.0.6   2020-17-09 added debouncer for cleaner signal of red switch
 V2.0.5   2020-13-09 pin assigned for other pins
 V2.0.4   2020-09-11 pin assigned for RGB LEDs
 V2.0.3   2020-08-19 changed pins for production prototype setup.
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
 V0.90    2015-02-05 init for hardware pins changes.



To be done (2020-10-08):

    Variables needed for storage:
      - Pot timing
      - one wipe pluse lengh
      - windows beats number for sliding window
      - start time pulse and actual reed relay signal recieving
      - aquired reed relay signal and stoptime

    setup test for getting delay of start morot and activity on magnet
    setup marker for reading the variables storage to and from eeprom and init the memory used.

    EEPROMex.h modified:
      #define EEPROMSize1k8        1080
      #define EEPROMSizeTeensy40    EEPROMSize1k8

 */

static char VERSION[] = "V2.0.7";;

//set drivers 
  #include <SPI.h>
  #include <EEPROMex.h>
  #include <Bounce.h>
  #include <RGBLED.h>

//audio setup
  #include <Audio.h>
  #include <Wire.h>
  #include <SD.h>


//Pins setup
        //old data
        // int beatRedPin = 2;
        // int beatGreenPin = 3;
        // int beatBluePin = 4;
  
        // int motorRedPin = 5;
        // int motorGreenPin = 6;
        // int motorBluePin = 9;

        int magnetPin = 14;
        int outputPin = 16;
        int potPin=17;

//init variables setup
        Bounce pushbutton = Bounce(magnetPin, 5);  
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

        //RGB LED pins setup
        RGBLED rgbLedBeat(2,3,4,COMMON_CATHODE);
        RGBLED rgbLedMotor(5,6,9,COMMON_CATHODE);

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
        elapsedMillis fps;
        uint8_t cnt=0;

        //display magnet info  setup 
        byte previousState = HIGH;         // what state was the button last time
        unsigned int count = 0;            // how many times has it changed to low
        unsigned long countAt = 0;         // when count changed
        unsigned int countPrinted = 0;     // last count printed
   
void setup() {
  //serial start 
    Serial.begin(9600);
    delay(100);
  
  //EEPROM setup for Teensy4
    EEPROM.setMemPool(memBase, EEPROMSizeTeensy40);
    EEPROM.setMaxAllowedWrites(maxAllowedWrites);    
    addressLong   = EEPROM.getAddress(sizeof(long));
    trigger_level = EEPROM.readLong(addressLong);


  // setup pins for switches
    // // beatleds
    //   pinMode(beatRedPin, OUTPUT);
    //   pinMode(beatGreenPin, OUTPUT);
    //   pinMode(beatBluePin, OUTPUT);
    // // motorleds
    //   pinMode(motorRedPin, OUTPUT);
    //   pinMode(motorGreenPin, OUTPUT);
    //   pinMode(motorBluePin, OUTPUT);
    // misc pins
      pinMode(outputPin, OUTPUT);
      pinMode(magnetPin, INPUT_PULLUP);
      pinMode(potPin, INPUT);

    delay(1000);
    Serial.println("Magnet switch setup done");
    //display status on serial
      Serial.print("Version=");
      Serial.println(VERSION);
      Serial.print("Setup Mode=");
      Serial.println(setupmode);
      Serial.print("Trigger_level=");
      Serial.println((trigger_level)*10000);
      Serial.print("magnetOn=");
      Serial.println(magnetOn);

  
    // Audio connections require memory to work.  For more
      AudioMemory(100); // the analysis frame (2048 samples) + 1 block (128 samples)
      // Enable the audio shield and set the output volume.
      audioShield.enable();
      audioShield.inputSelect(myInput);
      audioShield.volume(0.5);
  
      
    // reduce the gain on mixer channels
      monoMix.gain(0, 0.2);
      monoMix.gain(1, 0.2);
    
    //display end of setup
      delay(1000);
      Serial.println("Audio setup finished");
      Serial.println("Init finished");

  //run_motor();
}



void loop() {
 
       int ac_triggerlevel=((trigger_level)*10000);
      //sets minimum time for wiper for 
      // unsigned long currentT = millis();
      if(magnetOn){
              int minTimeSet=6000;
              minloop(minTimeSet);
              magnetOn= true;
              // if(motorOn) {
              //     stop_motor();
              //   }
            }else{        
              minloop(minTimeSet);
            } 

  // display magnet status
  if (pushbutton.update()) {
    if (pushbutton.risingEdge()) {
      count = count + 1;
    Serial.println("magnet is leaving");
      rgbLedMotor.writeRGB(255,60,0);
      countAt = millis();
      magnetOn= true;
    }
    if (pushbutton.fallingEdge()) {
    Serial.println("magnet is arrived");
      rgbLedMotor.writeRGB(0,128,0);
      countAt = millis();
      magnetOn= false;

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


  // Main loop
  if(magnetOn){
      //int minTimeSet=6000;
      // minloop(minTimeSet);
      magnetOn= true;
      }else{        
      // minloop(minTimeSet);
      } 
      
  
  if(beat.available()){  
      uint32_t ac = beat.ac();//2000;

  if (ac >(unsigned)(ac_triggerlevel)){
            Serial.print("ac=");
            Serial.println(ac);
            Serial.print("ac_triggerlevel=");
            Serial.println(ac_triggerlevel);
            // digitalWrite(ledGreenPin,HIGH);
            // run_motor();
            beatPulse = true;
          }else{
            // digitalWrite(ledGreenPin,LOW);
            beatPulse = false;
          }
    }
}


void minloop(long minTime)
      {
      // create the minimum timing for the wiper motor to wait for next beat
      
         unsigned long currentMillis = millis();
          
        if(currentMillis - previousMillis > (unsigned)minTime) {
          previousMillis = currentMillis;  
            run_motor();
        }
      }

void run_motor() {
        rgbLedMotor.writeRGB(0,255,0);
         motorOn= true;
         magnetOn=false;
         Serial.println("motor running");

}

void stop_motor() {
          rgbLedMotor.writeRGB(255,0,0);
          motorOn= false; 
           Serial.println("motor stopped");
}
