/*
  Beats for Hyperwiper(University of Aizu and YR-Design) beat detection done by Julian Villagas

 - uses Teensy 4.0
 - custom board for controlling car windshield wipers.
 - instructions can be found at: Github...
 
 contact rob@yr-design.biz
 
 revisions:
 V2.3.3   2021-05-13 Setup for inbetween full wiper moves accent of beat
 V2.3.2   2021-05-05 Fixed PWM issue and delay issues
 V2.3.1   2921-05-05 Update version numbering
 V2.3.0   2021-05-05 Beat detction not locking up (need still PWM going)
 V2.2.9   2021-03-19 Moved pot delay timing from run routine to between the beat detections and run motor routine
 V2.2.8   2021-03-18 Code fixing for start/stop start bug
 V2.2.7   2021-02-13 Cleaned up code.
 V2.2.6   2021-02-13 Changed schematic Kicad to MOSFET only.
 V2.2.5   2021-02-09 Prepare for PWM and MOSFETS for controling power
 V2.2.4   2021-01-07 Mapped the pot meter for timing of one beat 60-130BPM
 V2.2.3   2021-01-04 Fixed code for correctly pot meter change reading.
 V2.2.2   2021-01-04 Remove code for wipe test at startup and added pot delay for run_motor function.
 V2.2.1   2020-12-32 Modified reyurn wiper magnet code
 V2.2.0   2020-12-31 Added stoptime after wiper passes magnet switch on return pass
 V2.1.9   2020-12-21 Audiomix disabled and pot level detect adjusted.
 V2.1.8   2020-12-12 Updated beat LED displaying and level addjusted for dispayling serial out.
 V2.1.7   2020-12-10 Started beat timing array construction.
 V2.1.6   2020-12-07 Longer displaying beat detection in loop.
 V2.1.5   2020-12-07 Fixed stop motor after returning wiper delay.
 V2.1.4   2020-12-07 Fixe d startup bug that would not allow to function to be called before setup.
 V2.1.3   2020-12-06 Setup for calculate one full test wipe.
 V2.1.2   2020-12-05 Setup for potmenter halving the wipe time
 V2.1.1   2020-11-18 Added start_motor_reach_magnet timeing 
 V2.1.0   2020-10-24 Made minloop longer for easier checking and 
 V2.0.9   2020-10-20 Updated for tests for magnet timing/leaving variables.
 V2.0.8   2020-10-17 intergrated lastest beat detection from Julian and Camilo
 V2.0.7   2020-10-08 added test for startup calculating the startup delay and storing in EEPROM
 V2.0.6   2020-17-09 added debouncer for cleaner signal of red switch
 V2.0.5   2020-13-09 pin assigned for other pins
 V2.0.4   2020-09-11 pin assigned for RGB LEDs
 V2.0.3   2020-08-19 changed pins for production prototype setup.
 V2.0.2   2020-07-29 disabled joy switches, disable screen
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



To be done (2021-03-20):
     
      -add new variables for delay_time_run in init, beatdetect part run command remove and replaced with set delay_time_run boolean, make a routine with timing in loop to let it always run though the routine to check the time and trigger the run_motor command and also set the delay_time_run boolean to false.

      - windows beats number for sliding window                      
      info: BPM should be between 60 and 130. So 1000ms till 460ms

      Done:
      - add pot timing to the beat detect/start time
      - if wiper leaves it should always diabled magnet switch for the time that the run_reed_leave_time is calculetted puls some extra (maybe 500mSec)
      - add stop motor time with the pass magnet information that is calculated from the start-motor-reach-magnet time in the setup
      - start time pulse and actual reed relay signal receiving       run_reed_leave_time
      - read potmeter value for setup delay of beat in main loop
      - convert run_reed_leave_time to half of one_wipe_time
      - acquired reed relay signal and stoptime                        return_reed_rest_time
      - Pot timing -                                                  pot_time


    setup test for getting delay of start motor and activity on magnet
    setup marker for reading the variables storage to and from eeprom and init the memory used.

    EEPROMex.h modified:
      #define EEPROMSize1k8        1080
      #define EEPROMSizeTeensy40    EEPROMSize1k8

 */

static char VERSION[] = "V2.3.3";;

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

// setup for PWM 
  #define SENSOR_THRESHOLD 1000
  #define SENSOR_PIN A10 //pin IO04
  #define PWM_PIN 10 // pin D10

// Start init's -------------------------------------------------------------------------------------------------------------------------------------------------

//Pins setup
        int magnetPin = 14;
        int motorPin = 16;
        int potPin=17;
        unsigned potRead;
        int one_wipe_time_procentage;
        int potTime;

//display magnet info  setup 
        byte previousState = HIGH;         // what state was the button last time
        unsigned int count = 0;            // how many times has it changed to low
        unsigned long countAt = 0;         // when count changed
        unsigned int countPrinted = 0;     // last count printed
        unsigned int magnet_delay = 2000;  // delay for after starting magnet leave to prefent stopping directly after start
        boolean justStart=false;

//init variables setup
        Bounce pushbutton = Bounce(magnetPin, 10);  
        boolean motorOn= false;
        boolean magnetOn= false;
        long previousMillis = 0;
        long previousMillis_test = 0;
        long previousMillis_delay = 0;
        unsigned long previous_beatled_Millis = 0;        // will store last time LED was updated
        int beatcount =1;
        int ledState = LOW;  
        int beatPulse=0;
        int trigger_level = 0;
        long run_reed_leave_time = 0;
        long run_reed_return_time = 0;
        long minTimeSet=20000;
        float pot_old_read;
        long stop_time =0;
        int beat_array_count = 0;
        int beatPulseArray[4]={0,0,0,0};

      //EEPROM setup
        const int maxAllowedWrites = 80;
        const int memBase          = 120;
        int addressLong;
        int runpassed_interval= 180;
        boolean setupmode = false;

      //RGB LED pins setup
        RGBLED rgbLedBeat(2,3,4,COMMON_CATHODE);
        RGBLED rgbLedMotor(5,6,9,COMMON_CATHODE);

      // Initialize library
        FanController fan(SENSOR_PIN, SENSOR_THRESHOLD, PWM_PIN);

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
        const int ONtime_mills = 100;   // time for better delay function
        unsigned long ledStarted = 0;

        // Specify global variables here
        unsigned long curDel = 0;     // Current delay
        double avgDelay = 0.0;  // Average delay
        float prevMags[noBins][LPorder]; // Previous magnitudes for LP prediction
        double ks[LPorder]; // weights of the previous samples in the prediction of the current
        float minLevel = 20; //signal must be higher that 1% to report a beat

        unsigned int minDelay = 29;   // How long should we wait at least before the next beat?
        double lambda = .6; // weight of the median
        double alpha = .6;  // weight of the mean
        double m = 0.0;     // the mean of the previous ODFs
        double median = 0.0;     // the median of the previous ODFs
        int guess = 0;

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




// end init -------------------------------------------------------------------------------------------------------------------------------------------------

    void run_motor() {
            rgbLedMotor.writeRGB(0,255,0);
       //start motor     
            //for MOSFET
            fan.setDutyCycle(100);
            // digitalWrite(motorPin, LOW); for relay
            Serial.println("motor_running");
    }

    void stop_motor() {
            //for MOSFET
            fan.setDutyCycle(0);
            // digitalWrite(motorPin, HIGH); for relay
            Serial.println("motor_stopped");
    }

    void minloop(long minTime)
      {
      // create the minimum timing for the wiper motor to wait for next beat
         unsigned long currentMillis = millis();  
        if(currentMillis - previousMillis > (unsigned)minTime) {
          previousMillis = currentMillis;  
          // run motor
            rgbLedMotor.writeRGB(255,0,60);
            motorOn= true;
            // digitalWrite(motorPin, LOW);
            run_motor();
            Serial.println("minloop_motor_started");
        } 
      }


    void delay_run_motor(long delay_time)
      {
      // create the minimum timing for the wiper motor to wait for next beat
         unsigned long currentMillis_delay = millis();  
        Serial.print("Delay Time is =");
        Serial.println(delay_time);
        if(currentMillis_delay - previousMillis_delay > (unsigned)delay_time) {
          previousMillis_delay = currentMillis_delay;  
          // run motor
            rgbLedMotor.writeRGB(255,0,60);
            motorOn= true;
            run_motor();
            Serial.println();
            Serial.println("---------------------------------");
            Serial.println("delay time run");
            Serial.println("---------------------------------");
            Serial.println();
        } 
      }


    void test_beat(long delay_time_test)
      {
      // create the minimum timing for the wiper motor to wait for next beat
         unsigned long currentMillis_test = millis();  
        if(currentMillis_test - previousMillis_test > (unsigned)delay_time_test) {
          previousMillis_test = currentMillis_test;  
          // run motor
            rgbLedMotor.writeRGB(255,255,255);
            Serial.println("test beat");
        } 
      }

// start setup  -------------------------------------------------------------------------------------------------------------------------------------------------
         
   void setup() {
          //serial start 
            Serial.begin(9600);
            delay(1000);
            Serial.println("Start serial");
          
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
          audioShield.volume(0.8);  //0.8 is the max undistorted output on the headphones

          // reduce the gain on mixer channels
          monoMix.gain(0, 0.5);
          monoMix.gain(1, 0.5);

          // Butterworth filter, 12 db/octave
          LPfilter.setLowpass(0, 4000, .6);

          // Configure the window algorithm to use
          myFFT.windowFunction(AudioWindowHanning1024);

          delayForL.delay(0, 1000 * noBins / 44100);
          delayForR.delay(0, 1000 * noBins / 44100);
    
            //read potmeter 
              potRead = analogRead(potPin);

            //display status on serial
                Serial.println();
                Serial.println("-------- Set up info --------------------");
                Serial.print("Version=");
                Serial.println(VERSION);
                Serial.print("Setup Mode=");
                Serial.println(setupmode);
                Serial.print("magnetOn=");
                Serial.println(magnetOn);
                Serial.print("potential meter level =");
                Serial.println(potRead);
                Serial.println("Audio setup finished");
                Serial.println("-------- End Set up info ----------------");
                Serial.println();
      delay(1000);
}

// end setup -------------------------------------------------------------------------------------------------------------------------------------------------

void loop() {



//logic for switching on the beat led only once
    unsigned long beatledMillis = millis();

    if (beatledMillis - previous_beatled_Millis >= ONtime_mills) {
      previous_beatled_Millis = beatledMillis;
      if (beatcount==1) {
        ledState=true;
        beatcount++;
        Serial.print ("Beat count =");
        Serial.println(beat_array_count);
      }else{
        ledState=false;
      }
    }


//display LED beats
    if (ledState) {
        rgbLedBeat.writeRGB(255,40,0);
    }
    else{
        rgbLedBeat.writeRGB(0,0,0);
    }

 // display magnet status
  if (pushbutton.update()) {
    if (pushbutton.risingEdge()) {
      count = count + 1;
    Serial.println("Wiper magnet_leaving");
      rgbLedMotor.writeRGB(255,60,0);
      countAt = millis();
      magnetOn= false;
    }
    
    if (pushbutton.fallingEdge()) {
    Serial.println("Wiper magnet_arrived");
      rgbLedMotor.writeRGB(0,0,255);
        magnetOn= true;
        stop_motor();
    }
  } else {
    if (count != countPrinted) {
      unsigned long nowMillis = millis();
      if (nowMillis - countAt > 50) {
        countPrinted = count;
      }
    }
  }
  // read potmeter value for setup delay of beat in main loop
      int potRead = map(analogRead(potPin), 0, 1023, 460, 1000);
      if (abs(potRead-pot_old_read)>20){
        pot_old_read=potRead;
      //convert run_reed_leave_time to half of one_wipe_time
        one_wipe_time_procentage= potRead;
        //display motor LED white if potmeter is changed for testing
        rgbLedMotor.writeRGB(255,255,255);
        //serial print pecentage of the pot delay in relation to wipe
        Serial.print("one wipe time is = ");
        Serial.println(one_wipe_time_procentage);
      }
   

  // Main loop 

  // minloop timer
    minloop(minTimeSet);

  //Test beat
    // test_beat(890);

  //Main audio beat detection
  double sum = 0.0;   // a float to store the sum of all the OFTs bin magnitudes
  double thr = 0.0;  // a threshold to issue a beat, e.g., a frame must be 150% louder than the average
  double psd = 0.0;  //the predicted value


  double n;         // a float for reading each FFT bin magnitude

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

      lp(i); // Compute the weights for prediction

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
      // //set beat count and flash led for 1 second
        beatcount=1;
        beat_array_count++;
      // //set time for the beat array 
      //   beatPulseArray[beat_array_count]=millis();
      // Serial.println(beatPulseArray[beat_array_count]);
        Serial.println("beat detected-1");

      //add delay_run_motor boolean set here delete run_motor routine
      delay_run_motor(potRead);
      // run_motor();
    }

    if ((curDel >= 2.0 * round(avgDelay)) && (curDel > minDelay) && (sum >= minLevel)) { // There's a beat
      curDel = 0;
      guess = 1;
      // //set beat count and flash led for 1 second
        beatcount=1;
        beat_array_count++;
      // //set time for the beat array 1
      //   beatPulseArray[beat_array_count]=millis();
        // Serial.println(beatPulseArray[beat_array_count]);
        Serial.println("beat detected-2");
      //add delay_run_motor boolean set here delete run_motor routine
          delay_run_motor(potRead);
      // run_motor();
     }
  }
}
// end loop -------------------------------------------------------------------------------------------------------------------------------------------------




