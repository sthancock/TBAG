/*#####################*/
/*# Program to drive  #*/
/*# throttle response #*/
/*# for bucc cockpit  #*/
/*# TBAG 2021         #*/
/*#####################*/


// set all the pins
const unsigned char RpPin = 12;  // red positive pin
const unsigned char RnPin = 6;   // red negative pin
const unsigned char GpPin = 11;  // green
const unsigned char GnPin = 5;   // green
const unsigned char BpPin = 10;  // blue
const unsigned char BnPin = 4;   // blue
const unsigned char throt1Pin = A5; // input for throttle 1

class procedures{
  const float maxRPM=4000;      // maximum RPM for tachometers
  const float minThrotV=1024.0; // maximum throttle input voltage
  
}proc;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
