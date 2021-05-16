
void setPhase(unsigned char ph);
void advancePhase(char dir);

const unsigned char AHPin = 12;
const unsigned char ALPin = 6;
const unsigned char BHPin = 11;
const unsigned char BLPin = 5;
const unsigned char CHPin = 10;
const unsigned char CLPin = 4;
const unsigned char delayPin = A5;

char currentPhase = 0;
char motorDirection = 1;
unsigned long previousDelay = 0;

// these seem to be binary numbers
const unsigned char phaseStates[6]{
  B011000,  // 24?
  B010010,  // 18?
  B000110,  // 6?
  B100100,  // 36?
  B100001,  // 33?
  B001001   // 9?
};

// turns power off and on to pins?

void setPhase(unsigned char ph){
  unsigned char phase = phaseStates[ph];
  digitalWrite(AHPin, (phase & B100000) >> 5);
  digitalWrite(ALPin, (phase & B010000) >> 4);
  digitalWrite(BHPin, (phase & B001000) >> 3);
  digitalWrite(BLPin, (phase & B000100) >> 2);
  digitalWrite(CHPin, (phase & B000010) >> 1);
  digitalWrite(CLPin, phase & B000001);
}

// not sure what this one does?

void advancePhase(char dir){
  currentPhase += dir;
  if(currentPhase > 5){
    currentPhase = 0;
  }else if(currentPhase < 0){
    currentPhase = 5;
  }
}

void setup(){
  pinMode(AHPin, OUTPUT);
  pinMode(ALPin, OUTPUT);
  pinMode(BHPin, OUTPUT);
  pinMode(BLPin, OUTPUT);
  pinMode(CHPin, OUTPUT);
  pinMode(CLPin, OUTPUT);
  pinMode(delayPin, INPUT);

  Serial.begin(9600);
}

void loop(){

  // read the frequency, in microseconds
  long delay = map(analogRead(delayPin), 0, 1024, 0, 100000);

  //print control position to screen
  Serial.print(delay);
  Serial.print(" tim ");
  Serial.print(micros());
  Serial.print(" phase ");
  Serial.print((float)currentPhase);
  Serial.print("\n");

  //skip rest if the timing is not more than the delay long
  if(micros() - previousDelay <= delay) {
    return;
  }

  // advance timer
  previousDelay += delay;

  //advance the phase?
  advancePhase(motorDirection);

  // set the phase?
  setPhase(currentPhase);
}
