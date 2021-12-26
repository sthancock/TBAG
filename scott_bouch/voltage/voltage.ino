/* June 2016 - Scott Bouchard, UK - www.scottbouch.com
 *  
 * Aircraft cockpit panel instrument control from potentiometer input for test purposes.
 * 
 * For: Arduino Uno. Simple moving coil, two terminal, cockpit instruments.
 *   
 * Gague Circuit: series resistor sets Maximum FSD, Minimum (0) FSD is set by rangeMin 
 * variable values further on in the code.
 */

int inputVal = 0;
int val = 0;
int scaledVal = 0;
int rangeMin = 0;
int outputPin = 0;
const int inPin = A0;
int testValue = 2;
int testMaxHit = 0;
int testMaxTimeout = 0;
int testBit = 1;
int upDone = 0;

void setup() {
        Serial.begin(9600);
}

// This custom function operates all gagues and runs as a subroutine. Do not change.
void gaugeFunction(int val,int FoutputPin,int FrangeMin){

        scaledVal = map(val,0,1023,FrangeMin,255);
        analogWrite(FoutputPin,scaledVal);
        delay(10);
}

void loop(){

// Power-on gauge self test

  while(testValue > 1){                  // While loop runs while condition is true, the
                                         // rest of program won't run until it's finished.

        testValue = testValue + testBit; // Add bit(s) per loop.
        
        if (testValue == 255){           // When max is reached...
          testMaxHit = 1;
        }
          else{
          testMaxHit = 0;
          }
        
        
        if (testMaxHit){
          delay (500);
        }

        
        if (testMaxHit || upDone){       // ... latch upDone.
          upDone = 1;
        }


        if(upDone){                      // When FSD is reached...
          testBit = -1;                  // ... ramp Down bits per loop.
        }
        else{                            // Before FSD is reached...
          testBit = 1;                   // ... ramp Up bits per loop.
        }

        
        delay(40);                       // Sets speed of ramp up and down
        
        analogWrite(3, testValue);       // Write test values to all outputs
        analogWrite(5, testValue);
        analogWrite(6, testValue);
        analogWrite(9, testValue);
        analogWrite(10, testValue);
        analogWrite(11, testValue);
        }


        inputVal = analogRead(inPin);


// Configure outputPin and rangeMin to suit the gauge connected.
// The following blocks can be copied and configured for more gauges.


// 1. Weston - Liquid Oxygen Litres Gauge - 0 to F litres - Ref: 6D/2154.
       outputPin = 3;
       rangeMin = 50;
       gaugeFunction(inputVal,outputPin,rangeMin);


// 2. Smiths - Fuel lbs Gauge - 0 to 2300 lbs Ref: 3AG 6A/4931.
      outputPin = 5;
      rangeMin = 13;
      gaugeFunction(inputVal,outputPin,rangeMin);

}

