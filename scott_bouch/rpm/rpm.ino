/*
From Scott Boucher
https://forum.arduino.cc/t/3-phase-hysteresis-motor-control-why-so-slow-jet-engine-speed-indicator/566210
*/


#define DEBUG

int outA = 4;                      // Declare output pin A
int outB = 5;                      // Declare output pin B
int outC = 6;                      // Declare output pin C
int arrayLookupA = 1;               // Output A array lookup point
int arrayLookupB = 3;               // Output B array lookup point
int arrayLookupC = 5;               // Output C array lookup point
int arrayIndex = 0;                 // For stepping through the array
int statusArray[6] = {1,1,1,0,0,0}; // Sequence used for output switching


void setup() {
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
  
  pinMode(outA, OUTPUT);
  pinMode(outB, OUTPUT);
  pinMode(outC, OUTPUT);
}

void loop() {


  ++arrayIndex;
  if (arrayIndex == 6){
    arrayIndex = 0;
  }

  arrayLookupA = arrayIndex;

  if (arrayIndex + 2 < 6)arrayLookupB = arrayIndex + 2;
  else                   arrayLookupB = arrayIndex - 4;

  if (arrayIndex + 4 < 6)arrayLookupC = arrayIndex + 4;
  else                   arrayLookupC = arrayIndex - 2;

   digitalWrite(outA, statusArray[arrayLookupA]);
   digitalWrite(outB, statusArray[arrayLookupB]);
   digitalWrite(outC, statusArray[arrayLookupC]);

  #ifdef DEBUG
   Serial.print("Microseconds ");
   Serial.print(arrayIndex);
   Serial.print(" ");
   Serial.print(statusArray[arrayLookupA]);
   Serial.print(" ");
   Serial.print(statusArray[arrayLookupB]);
   Serial.print(" ");
   Serial.print(statusArray[arrayLookupC]);
   Serial.print("\n");
   #endif
}
