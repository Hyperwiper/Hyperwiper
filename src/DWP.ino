/*
  Beats for DWP (University of Aizu and YR-Design) beat detection done by Julian Villagas

 - uses Ada fruit ssd1351 display and Teensy 3.1
 - custom board for controlling car windshield wipers.
 - instructions can be found at:
 - 
 contact rob@yr-design.biz
 
 revisions:
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

static char VERSION[] = "V1.2.";;

//Display setup for SSD1351
//notes SI 11 and CS 9 are used for the audio board and other too.
  // #define sclk 10
  // #define mosi 7
  // #define cs   3
  // #define rst  14
  // #define dc   12





// Color definitions
  /* some RGB color definitions                                                 */
  #define BLACK           0x0000      /*   0,   0,   0 */
  #define NAVY            0x000F      /*   0,   0, 128 */
  #define DARKGREEN       0x03E0      /*   0, 128,   0 */
  #define DARKCYAN        0x03EF      /*   0, 128, 128 */
  #define MAROON          0x7800      /* 128,   0,   0 */
  #define PURPLE          0x780F      /* 128,   0, 128 */
  #define OLIVE           0x7BE0      /* 128, 128,   0 */
  #define LIGHTGREY       0xC618      /* 192, 192, 192 */
  #define DARKGREY        0x7BEF      /* 128, 128, 128 */
  #define BLUE            0x001F      /*   0,   0, 255 */
  #define GREEN           0x07E0      /*   0, 255,   0 */
  #define CYAN            0x07FF      /*   0, 255, 255 */
  #define RED             0xF800      /* 255,   0,   0 */
  #define MAGENTA         0xF81F      /* 255,   0, 255 */
  #define YELLOW          0xFFE0      /* 255, 255,   0 */
  #define WHITE           0xFFFF      /* 255, 255, 255 */
  #define ORANGE          0xFD20      /* 255, 165,   0 */
  #define GREENYELLOW     0xAFE5      /* 173, 255,  47 */
  #define PINK            0xF81F


//set drivers 
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1351.h>
  #include <SPI.h>
  #include <EEPROMex.h>

  // Adafruit_SSD1351 tft = Adafruit_SSD1351(cs, dc, mosi, sclk, rst); 
  Adafruit_SSD1351 tft = Adafruit_SSD1351(3, 12, 7, 10, 14); 


//audio setup
        #include <Audio.h>
        #include <Wire.h>
        #include <SD.h>


//Pins setup

        int outputPin = 4;
        // int magnetPin = 5;
        int ledRedPin = 6;
        int ledGreenPin = 8;

        //joystick pins setup
        const int JOY_A_PIN = 15;
        const int JOY_B_PIN = 16;
        const int JOY_C_PIN = 17;
        const int JOY_D_PIN = 20;
        const int JOY_E_PIN = 2;        


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



//Joy Switch setup             
        volatile boolean joyCntA = false;
        volatile boolean joyCntB = false;
        volatile boolean joyCntC = false;
        volatile boolean joyCntD = false;
        volatile boolean joyCntE = false;
    
            void joyA_Callback()
            {
                joyCntA = true;
            }
            void joyB_Callback()
            {
                joyCntB = true;
            }
            void joyC_Callback()
            {
                joyCntC = true;
            }
            void joyD_Callback()
            {
                joyCntD = true;
            }
            void joyE_Callback()
            {
                joyCntE = true;
            }
       

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
        addressLong      = EEPROM.getAddress(sizeof(long));
        trigger_level = EEPROM.readLong(addressLong);

 
     //Joy Switch setup     
        pinMode(JOY_A_PIN, INPUT_PULLUP);
        pinMode(JOY_B_PIN, INPUT_PULLUP);
        pinMode(JOY_C_PIN, INPUT_PULLUP);
        pinMode(JOY_D_PIN, INPUT_PULLUP);
        pinMode(JOY_E_PIN, INPUT_PULLUP);
        
        attachInterrupt(JOY_A_PIN, joyA_Callback, RISING);     
        attachInterrupt(JOY_B_PIN, joyB_Callback, RISING);     
        attachInterrupt(JOY_C_PIN, joyC_Callback, RISING);     
        attachInterrupt(JOY_D_PIN, joyD_Callback, RISING); 
        attachInterrupt(JOY_E_PIN, joyE_Callback, RISING);     


      //display setup
      tft.begin();
      tft.fillScreen(BLACK);
      tft.setTextColor(ORANGE);
      tft.setTextSize(1);
      tft.print("Dancing Wiper  ");
      tft.setTextColor(ORANGE);
      tft.setTextSize(1);
      tft.println(VERSION);
      tft.setTextSize(1);
      tft.println("");  
      tft.setCursor(0, 120);
      tft.setTextColor(ORANGE);
      tft.println("U-Aizu/YR-Design 2015");
      tft.setCursor(0, 20);
      tft.setTextColor(WHITE);
      tft.println("setup");
      tft.println("-----");
      tft.println("Power         :OK");

    //display status on serial
    Serial.print("Setup Mode=");
    Serial.println(setupmode);
    Serial.print("Trigger_level=");
    Serial.println((trigger_level)*10000);


     // setup pins for switches
     pinMode(outputPin, OUTPUT);   //wiper motor pin
     pinMode(ledRedPin, OUTPUT);  //red led 
     pinMode(ledGreenPin, OUTPUT); //green led
     // pinMode(magnetPin, INPUT_PULLUP);

      // attachInterrupt(magnetPin, onMagnet, FALLING);
      delay(1000);
      tft.println("Magnet switch :OK");
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
      tft.println("Audio board   :OK");
      Serial.println("Audio setup finished");

      Serial.println("Init finished");

      //display status
      tft.setCursor(0,70);
      tft.setTextColor(WHITE);
      tft.println("Status:");
  

  run_motor();
}

elapsedMillis fps;
uint8_t cnt=0;


void loop() {
 
       int ac_triggerlevel=((trigger_level)*10000);
      //sets minimum time for wiper for 
       unsigned long currentT = millis();

      //Joy switch setup
        if (joyCntA){ 
          Serial.println ("Right");
          joyCntA=!joyCntA;
          joyCntB=false;
          joyCntC=false;
          joyCntD=false;
          joyCntE=false;
          run_motor();
        }
      
        if (joyCntB){ 
          Serial.println ("Down");
          joyCntB=!joyCntB;
          joyCntA=false;
          joyCntC=false;
          joyCntD=false;
          joyCntE=false;
          if(setupmode){
            trigger_level--;
            EEPROM.writeLong(addressLong,trigger_level);
            Serial.print("trigger_level=");  
            Serial.println((trigger_level)*10000);
          }
        }
      
          if (joyCntC){ 
          Serial.println ("Left");
          joyCntC=!joyCntC;
          joyCntA=false;
          joyCntB=false;
          joyCntD=false;
          joyCntE=false;         
          if (motorOn) {
               previousMillis=millis() ;
                while ((unsigned long)(millis() - previousMillis) <= runpassed_interval) {
                }
                magnetOn=true;
                stop_motor();
            }
        }
      
          if (joyCntD){ 
          Serial.println ("Up");
          joyCntD=!joyCntD;
          joyCntA=false;
          joyCntB=false;
          joyCntC=false;
          joyCntE=false;
          if(setupmode){
            trigger_level++;
            EEPROM.writeLong(addressLong,trigger_level);
            Serial.print("trigger_level=");
            Serial.println((trigger_level)*10000);
          }
        }
        

        if (joyCntE){ 
          Serial.println ("Enter");
          joyCntE=!joyCntE;
          joyCntA=false;
          joyCntB=false;
          joyCntC=false;
          joyCntD=false;
          if(setupmode){
            trigger_level=0;
            EEPROM.writeLong(addressLong,trigger_level);
            Serial.print("trigger_level=");
            Serial.println((trigger_level)*10000);
          }
        }
         


// Main loop
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
         tft.setCursor(9,80);
         tft.setTextColor(BLACK);
         tft.println("motor stopped");
         tft.setCursor(9,80);
         tft.setTextColor(YELLOW);
         tft.println("motor running"); 
         Serial.println("motor running");

}

void stop_motor() {

          digitalWrite(outputPin, LOW);
          digitalWrite(ledRedPin, LOW);
          motorOn= false;
          //magnetOn=false;
           tft.setCursor(9,80);
           tft.setTextColor(BLACK);
           tft.println("motor running");
           tft.setCursor(9,80);
           tft.setTextColor(YELLOW);
           tft.println("motor stopped");   
           Serial.println("motor stopped");
}

