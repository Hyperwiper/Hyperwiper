#include <Wire.h>
#include <SPI.h>
#include <SD.h>

unsigned long time = 0;
int beatPulseArray[4]={0,0,0,0};
int beatPulse=0;

void setup() {
	Serial.begin(9600);
}

void missedBeat(int beatPulse){
}

void checkBeat(int bp){
	Serial.print("beat count =");
	Serial.println(bp);
	Serial.print("value =");
	Serial.println(beatPulseArray[bp]);
}


void loop() {
	if (beatPulseArray[0]=0) Serial.print(" first beat");
	for (int i = 0; i < 3; i++) {
		time = millis();
		beatPulseArray[i]=time;
		delay(100);
	}
for (int i = 0; i < 3; i++) Serial.print(beatPulseArray[i]);
Serial.println();

delay(2000);



//beat detected read current time 
//store time beatPulse[x]
//wait for next beat
//store time in beatPulse[x+1]
//subtract first beat time from second beat time
//store difference
//add difference to current time to calculate next beat
//wait for next beat
//store time in beatPulse[x+2]
//if next beat comes within half of the time between first and second beat and half after we do not have missed 
//       a beat so we can calculate next beat and prepare for it.
 //
}