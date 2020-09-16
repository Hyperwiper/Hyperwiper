#include <Arduino.h>
#include <SoftPWM.h>


/*SoftPWMBegin(SOFTPWM_NORMAL);
Begin using SoftPWM. This function must be called first. The input can be SOFTPWM_NORMAL or SOFTPWM_INVERTED.

SoftPWMSet(pin, value);
Write the PWM on a pin. The value may be between 0 to 255. If fade times have been defined, the PWM will change gradually.

SoftPWMSetFadeTime(pin, fadeUpTime, fadeDownTime);
Set speed a PWM pin changes. fadeUpTime and fadeDownTime may be between 0 to 4000 milliseconds.

SoftPWMSet(pin, polarity);
Change the polarity on a single pin. 

Teensy 4.0	PWM pins 
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 18, 19, 22, 23,
24, 25, 28, 29, 33, 34, 35, 36, 37, 38, 39

*/


void setup()
{
  SoftPWMBegin();
  
  SoftPWMSet(13, 0);

  SoftPWMSetFadeTime(13, 1000, 1000);
}

void loop()
{
  SoftPWMSet(13, 255);
  delay(1000);
  SoftPWMSet(13, 0);
  delay(1000);
}