/*
  Beats for Hyperwiper(University of Aizu and YR-Design) beat detection done by Julian Villagas

 - uses Teensy 4.0
 - custom board for controlling car windshield wipers.
 - instructions can be found at: Github...
 
 contact rob@yr-design.biz
 
 revisions:
 V2.0.8   2020-10-17 intergrated lastest beat detection from Julian and Camilo
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



To be done (2020-10-18):

    Variables needed for storage:
      - Pot timing -                                                  pot_time
      - one wipe pluse lengh                                          one_wipe_time
      - windows beats number for sliding window                       
      - start time pulse and actual reed relay signal recieving       run_reed_leave_time
      - aquired reed relay signal and stoptime                        return_reed_rest_time


    setup test for getting delay of start motor and activity on magnet
    setup marker for reading the variables storage to and from eeprom and init the memory used.

    EEPROMex.h modified:
      #define EEPROMSize1k8        1080
      #define EEPROMSizeTeensy40    EEPROMSize1k8

 */

static char VERSION[] = "V2.0.8";;

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
        int magnetPin = 14;
        int motorPin = 16;
        int potPin=17;


//init variables setup
        Bounce pushbutton = Bounce(magnetPin, 10);  
        boolean beatPulse;
        boolean motorOn= false;
        boolean magnetOn= false;
        long previousMillis = 0;
        int ledState = LOW;  
        long minTimeSet=6000;
        int trigger_level = 0;
        int run_reed_leave_time = 0;

        //EEPROM setup
        const int maxAllowedWrites = 80;
        const int memBase          = 120;
        int addressLong;
        int runpassed_interval= 180;
        boolean setupmode = false;

        //RGB LED pins setup
        RGBLED rgbLedBeat(2,3,4,COMMON_CATHODE);
        RGBLED rgbLedMotor(5,6,9,COMMON_CATHODE);

        //Setup motor pin and reed pins


        //Audio setup
        AudioInputI2S       audioInput;
        AudioOutputI2S      headphones;
        AudioMixer4         monoMix;
        AudioFilterBiquad   LPfilter;
        AudioAnalyzeFFT1024  myFFT;
        AudioEffectDelay    delayForL;
        AudioEffectDelay    delayForR;

        AudioConnection c0(audioInput, 0, monoMix, 0);
        AudioConnection c1(audioInput, 1, monoMix, 1);
        AudioConnection c2(monoMix, 0, LPfilter, 0);
        AudioConnection c3(LPfilter, 0, myFFT, 0);
        AudioConnection c4(audioInput, 0, delayForL, 0);
        AudioConnection c5(audioInput, 1, delayForR, 1);
        AudioConnection c6(delayForL, 0, headphones, 0);
        AudioConnection c7(delayForR, 0, headphones, 1);

        // Specify which audio card we're using
        AudioControlSGTL5000 audioShield;

        // Define global constants here
        const int myInput = AUDIO_INPUT_LINEIN; // Are we using mic or line?
        const int numReadings = 7; // How many ODFs are we using for average?
        const int noBins = 512; // number of real bins used in the FFT (total number of bins should be double this)
        const int LPorder = 7; // Linear predictor order
        // const int led = 13;
        const int ONtime_mills = 10;   // time for better delay function
        unsigned long ledStarted = 0;

        // Specifty global variables here
        unsigned long curDel = 0;     // Current delay
        double avgDelay = 0.0;  // Average delay
        float prevMags[noBins][LPorder]; // Previous magnitudes for LP prediction
        double ks[LPorder]; // weigths of the previous samples in the prediction of the current
        float minLevel = 20; //signal must be higher that 1% to report a beat

        unsigned int minDelay = 29;   // How long should we wait at least before the next beat?
        double lambda = .6; // weight of the median
        double alpha = .6;  // weight of the mean
        double m = 0.0;     // the mean of the previous ODFs
        double median = 0.0;     // the median of the previous ODFs

        void lp(int k) {
          // Computes the linear prediction weights for the previous values of a given bin k
          float wk1[LPorder];
          float wk2[LPorder];
          float wkm[LPorder];

          wk1[0] = prevMags[k][0];
          wk2[LPorder - 2] = prevMags[k][LPorder - 1];

          for (int i = 1; i < LPorder - 1; ++i) {
            wk1[i] = prevMags[k][i];
            wk2[i - 1] = prevMags[k][i];
          }

          for (int i = 0; i < LPorder; ++i) {
            double num = 0.0;
            double denom = 0.0000000000000001; // to avoid NaNs

            for (int j = 0; j < (LPorder - i); ++j) {
              num += wk1[j] * wk2[j];
              denom += wk1[j] * wk1[j] + wk2[j] * wk2[j];
            }
            ks[i] = 2.0 * num / denom;

            for (int j = 0; j < i; ++j) {
              ks[j] = wkm[j] - ks[i] * wkm[i - j];
            }
            for (int j = 0; j <= i; ++j) {
              wkm[j] = ks[j];
            }
            for (int j = 0; j < (LPorder - i - 1); ++j) {
              wk1[j] -= wkm[i] * wk2[j];
              wk2[j] = wk2[j + 1] - wkm[i] * wk1[j + 1];
            }
          }
        }

            //display magnet info  setup 
            byte previousState = HIGH;         // what state was the button last time
            unsigned int count = 0;            // how many times has it changed to low
            unsigned long countAt = 0;         // when count changed
            unsigned int countPrinted = 0;     // last count printed


    void run_motor() {
            rgbLedMotor.writeRGB(0,255,0);
            motorOn= true;
                digitalWrite(motorPin, LOW);
            // magnetOn=false;
            Serial.println("motor running");

    }

    void stop_motor() {
              rgbLedMotor.writeRGB(255,0,0);
              motorOn= false; 
                  digitalWrite(motorPin, HIGH);
              Serial.println("motor stopped");
    }

    void minloop(long minTime)
      {
      // create the minimum timing for the wiper motor to wait for next beat
         unsigned long currentMillis = millis();  
        if(currentMillis - previousMillis > (unsigned)minTime) {
          previousMillis = currentMillis;  
          // run motor
            rgbLedMotor.writeRGB(0,255,0);
            motorOn= true;
                digitalWrite(motorPin, LOW);
            // magnetOn=false;
            Serial.println("motor running");
        }
      }
          
   void setup() {
          //serial start 
            Serial.begin(9600);
            delay(1000);
          
          //EEPROM setup for Teensy4
            EEPROM.setMemPool(memBase, EEPROMSizeTeensy40);
            EEPROM.setMaxAllowedWrites(maxAllowedWrites);    
            addressLong   = EEPROM.getAddress(sizeof(long));
            trigger_level = EEPROM.readLong(addressLong);

          // setup pins for switches
              pinMode(motorPin, OUTPUT);
              pinMode(magnetPin, INPUT_PULLUP);
              pinMode(potPin, INPUT);

          // Audio connections require memory to work.
          AudioMemory(150); // To Do: check the actual memory used and free the rest

          // Enable the audio shield and set the output volume.
          audioShield.enable();
          audioShield.audioPreProcessorEnable();
          audioShield.autoVolumeControl(2, 3, 0, -30, 5, 50); //enable compressor

          audioShield.inputSelect(myInput);
          audioShield.volume(.3);  //0.8 is the max undistorted output on the headphones

          // reduce the gain on mixer channels
          monoMix.gain(0, 0.5);
          monoMix.gain(1, 0.5);

          // Butterworth filter, 12 db/octave
          LPfilter.setLowpass(0, 4000, .6);

          // Configure the window algorithm to use
          myFFT.windowFunction(AudioWindowHanning1024);

          delayForL.delay(0, 1000 * noBins / 44100);
          delayForR.delay(0, 1000 * noBins / 44100);
    

    //test for collecting time for delay start_motor and reed_read (new name for reed activity.)
            //run motor
            rgbLedMotor.writeRGB(0,255,0);
            motorOn= true;
                digitalWrite(motorPin, LOW);
            //calculate time for reaching reedswitch
            while (digitalRead(magnetPin)==1){
              run_reed_leave_time=millis();
            }

      //display status on serial
          Serial.print("Version=");
          Serial.println(VERSION);
          Serial.print("Setup Mode=");
          Serial.println(setupmode);
          Serial.print("Trigger_level=");
          Serial.println((trigger_level)*10000);
          Serial.print("magnetOn=");
          Serial.println(magnetOn);
          Serial.print("Time from startpulse to reaching magnet is:");
          Serial.println (run_reed_leave_time);
          Serial.println("Audio setup finished");
          Serial.println("Init finished");
      

          rgbLedMotor.writeRGB(255,0,0);//red for motor passed reed switch
              //display end of setup
      delay(1000);



}



void loop() {
 
//sets minimum time for wiper for 
unsigned long currentT = millis();
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
      rgbLedMotor.writeRGB(0,0,255);
      countAt = millis();
      magnetOn= false;
      stop_motor();
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
      
  //Main audio beat detection
  double sum = 0.0;   // a float to store the sum of all the OFTs bin magnitudes
  double thr = 0.0;  // a threshold to issue a beat, e.g., a frame must be 150% louder than the average
  double psd = 0.0;  //the predicted value
  int guess = 0;

  double n;         // a float for reading each FFT bin magnitude
  unsigned long currentMillis = millis();

  // is the FFT available?
  if (myFFT.available()) {

    curDel = curDel + 1;

    // update previous magnitude array
    for (int i = 1; i < LPorder; i++) {
      for (int j = 0; j < noBins; j++) {
        prevMags[j][i - 1] = prevMags[j][i];
      }
    }

    // read current magnitudes
    for (int i = 0; i < noBins; i++) {
      n = myFFT.read(i);
      prevMags[i][LPorder - 1] = n;

      lp(i); // Compute the weigths for prediction

      // predict the value of this bin
      for (int j = 0; j < LPorder; j++) {
        psd += prevMags[i][j] * ks[j];
      }

      // compute the odf with spectral difference and linear prediction
      sum += abs(n - psd);
    }

    // calculate averages:
    m += ( sum - m ) / numReadings; //  running average.
    // this if implements the copysign function
    if ((sum - median) >= 0.0 ) {
      median += m / numReadings;
    } else {
      median += -m / numReadings;
    }
    //dirty median

    thr = lambda * median + alpha * m;

    if ((sum > thr) && (curDel >= minDelay) && (sum >= minLevel)) { // There's a beat
      avgDelay += ( curDel - avgDelay) / numReadings;
      curDel = 0; // restart the counter when the delay is at least the average

      //better delay function, so we do not miss any beats (rob)
      if (currentMillis - ledStarted >= ONtime_mills) {
        ledStarted = currentMillis;
        if (ledState == LOW) {
          rgbLedBeat.writeRGB(255,255,255);   // turn the LED on (HIGH is the voltage level)
        } else {
           rgbLedBeat.writeRGB(0,0,0);    // turn the LED off by making the voltage LOW
        }
     }
    }

    if ((curDel >= 2.0 * round(avgDelay)) && (curDel > minDelay) && (sum >= minLevel)) { // There's a beat
      curDel = 0;
      guess = 1;

      //better delay function, so we do not miss any beats (rob)
      if (currentMillis - ledStarted >= ONtime_mills) {
        ledStarted = currentMillis;
        if (ledState == LOW) {
          rgbLedBeat.writeRGB(255,255,255);   // turn the LED on (HIGH is the voltage level)
        } else {
           rgbLedBeat.writeRGB(0,0,0);    // turn the LED off by making the voltage LOW
        }
     }
    }
    Serial.print(sum, 5);
    Serial.print("\t");
    Serial.print(thr, 5);
    Serial.print("\t");
    Serial.print(guess == 1 ? -200 : 0);
    Serial.print("\t");
    Serial.println(curDel == 0 ? 200 : 0);
  }
}





