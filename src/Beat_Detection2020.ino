// Beat detection using energy in different frequency bins.
// Julian Villegas, 2020

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
//#include <SD.h>

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
const int led = 13;
const int ONtime = 10;
const int ONtime_mills = 10;   // time for better delay function
int ledState = LOW;
unsigned long ledStarted = 0;

// Specifty global variables here
unsigned long curDel = 0;     // Current delay
double avgDelay = 0.0;  // Average delay
float prevMags[noBins][LPorder]; // Previous magnitudes for LP prediction
double ks[LPorder]; // weigths of the previous samples in the prediction of the current
float minLevel = 10; //signal must be higher that 1% to report a beat



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

void setup() {
  pinMode(led, OUTPUT);

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
}

void loop() {
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
          digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
        } else {
          digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
        }
        digitalWrite(LED_BUILTIN, ledState);
      }
    }

    if ((curDel >= 2.0 * round(avgDelay)) && (curDel > minDelay) && (sum >= minLevel)) { // There's a beat
      curDel = 0;
      guess = 1;

      //better delay function, so we do not miss any beats (commented by rob)
      if (currentMillis - ledStarted >= ONtime_mills) {
        ledStarted = currentMillis;
        if (ledState == LOW) {
          digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
        } else {
          digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
        }
        digitalWrite(LED_BUILTIN, ledState);
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
