// Include the library

//set drivers
#include <SPI.h>
#include <Wire.h>
#include <SD.h>


#include <FanController.h>

// Sensor wire is plugged into port 2 on the Arduino.
// For a list of available pins on your board,
// please refer to: https://www.arduino.cc/en/Reference/AttachInterrupt
#define SENSOR_PIN A10 //pin IO04

// Choose a threshold in milliseconds between readings.
// A smaller value will give more updated results,
// while a higher value will give more accurate and smooth readings
#define SENSOR_THRESHOLD 1000

// PWM pin (4th on 4 pin fans)
#define PWM_PIN 10 // pin D10

// Initialize library
FanController fan(SENSOR_PIN, SENSOR_THRESHOLD, PWM_PIN);

/*
   The setup function. We only start the library here
*/
void setup(void)
{
  digitalWrite(PWM_PIN, LOW);
  // start serial portIRF4905
  Serial.begin(9600);
  delay(1000);
  Serial.println("PWM Controller ready");

  // Start up the library
  fan.begin();
  fan.setDutyCycle(100);

  // Get duty cycle
    byte dutyCycle = fan.getDutyCycle();
    Serial.print("Start Duty cycle: ");
    Serial.println(dutyCycle, DEC);

}

void loop(void)
{

  // Get new speed from Serial (0-100%)

    // Parse speed
    int input = Serial.parseInt();

    // Constrain a 0-100 range
    byte target = 100;

    // Print obtained value
    Serial.print("Setting duty cycle: ");
    Serial.println(target, DEC);

    // Set fan duty cycle
    fan.setDutyCycle(target);

    // Get duty cycle
    byte dutyCycle = fan.getDutyCycle();
    Serial.print("Duty cycle: ");
    Serial.println(dutyCycle, DEC);


  // Not really needed, just avoiding spamming the monitor,
  // readings will be performed no faster than once every THRESHOLD ms anyway
  delay(100);
}
