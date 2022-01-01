/*#####################*/
/*# Program to drive  #*/
/*# throttle response #*/
/*# for bucc cockpit  #*/
/*# TBAG 2021         #*/
/*#####################*/


#define DEBUG

/*#####################################*/
/*global variables*/

const float maxRPM=4000/60.0;      // maximum RPM for tachometers, in Hz
const float tRate=100.0/50.0;      // throttle change rate, in % per second
const int nAngSteps=360;           // number of angle steps in tachometer array
const float angScale=(float)nAngSteps/360.0;

/*#####################################*/
/*hold data and functions for engines*/

class engine{
  public:
    // methods
    void setup(unsigned char,unsigned char,unsigned char,unsigned char,unsigned char,unsigned char,unsigned char);
    void readInputs();
    void determineState();
    void writeState();

  private:
    // methods
    void setPhase(float,float);

    // throttle controls
    float throtPos1;  // throttle position, in %
    char cockPos;     // fuel cock position. On/off
  
    // throttle internals
    float tachAng;     // phase angle of tachometer
    int8_t onTachPin;    // which phase pin is on
    int8_t lastOnTachPin; // which pin was on last
    float tim;         // time now
  
    // throttle outputs
    float rpm1;        // tachometer RPM, in %
    float temp1;       // exhaust temperature

    // arduino pins
    unsigned char RpPin;     // red positive pin
    unsigned char RnPin;     // red negative pin
    unsigned char GpPin;     // green
    unsigned char GnPin;     // green
    unsigned char BpPin;     // blue
    unsigned char BnPin;     // blue
    unsigned char throt1Pin; // input for throttle
};


/*##############################*/
/*determine tachometer phases*/

void engine::setPhase(float thisTime,float dTime)
{

  onTachPin=1;

  #ifdef DEBUG  // write to display to monitor
  Serial.print(" onPin ");
  Serial.print(onTachPin);
  Serial.print(" tim ");
  Serial.print(tim);
  Serial.print(" dtim ");
  Serial.print(dTime,10);

  Serial.print("\n");
  #endif

  return;
}

/*##############################*/
/*internal setup*/

void engine::setup(unsigned char inRpPin,unsigned char inRnPin,unsigned char inGpPin,\
                   unsigned char inGnPin,unsigned char inBpPin,unsigned char inBnPin,unsigned char inthrot1Pin)
{
  // set bin variables
  RpPin=inRpPin;
  RnPin=inRnPin;
  GpPin=inGpPin;
  GnPin=inGnPin;
  BpPin=inBpPin;
  BnPin=inBnPin;
  throt1Pin=inthrot1Pin;

  // set pin modes
  pinMode(RpPin, OUTPUT);
  pinMode(RnPin, OUTPUT);
  pinMode(GpPin, OUTPUT);
  pinMode(GnPin, OUTPUT);
  pinMode(BpPin, OUTPUT);
  pinMode(BnPin, OUTPUT);
  pinMode(throt1Pin, INPUT);

  // set all pins LOW
  digitalWrite(RpPin,LOW);
  digitalWrite(RnPin,LOW);
  digitalWrite(GpPin,LOW);
  digitalWrite(GnPin,LOW);
  digitalWrite(BpPin,LOW);
  digitalWrite(BnPin,LOW);
  
  // inputs
  throtPos1=0.0; // starts with throttle closed
  cockPos=1;     // hard-coded open for now, as now switch

  // internals
  tachAng=0.0;
  
  // outputs
  rpm1=0.0;     // everything off
  temp1=0.0;
  onTachPin=0;
  lastOnTachPin=0;

  
  return;
}

/*##############################*/
/*read input controls*/

void engine::readInputs()
{
  //throttle
  throtPos1=(float)map(analogRead(throt1Pin), 0, 1024, 0, maxRPM);

  return;
}

/*##############################*/
/*write state*/

void engine::writeState()
{
  // determine phase for RPM
  
  // is this a change?
  // update this so a digitalWrite is only done for a direction change

  //eventually this should be replaced with arrays
  if(onTachPin!=lastOnTachPin){
    if(onTachPin==1){
      digitalWrite(RnPin,LOW);
      digitalWrite(GpPin,LOW);
      digitalWrite(BpPin,LOW);
      digitalWrite(BnPin,HIGH);
      digitalWrite(GnPin,HIGH);
      digitalWrite(RpPin,HIGH);
    }else if(onTachPin==2){
      digitalWrite(GnPin,LOW);
      digitalWrite(RpPin,LOW);
      digitalWrite(BpPin,LOW); 
      digitalWrite(RnPin,HIGH);
      digitalWrite(BnPin,HIGH);
      digitalWrite(GpPin,HIGH);
   }else if(onTachPin==3){
      digitalWrite(BnPin,LOW);
      digitalWrite(RpPin,LOW);
      digitalWrite(GpPin,LOW);
      digitalWrite(RnPin,HIGH);
      digitalWrite(GnPin,HIGH);
      digitalWrite(BpPin,HIGH);
    }else{
      digitalWrite(RpPin,LOW);
      digitalWrite(RnPin,LOW);
      digitalWrite(GpPin,LOW);
      digitalWrite(BnPin,LOW);
      digitalWrite(BpPin,LOW);
      digitalWrite(GnPin,LOW);  
    }
  }
  lastOnTachPin=onTachPin;
  
  return;
}


/*##############################*/
/*global classes to hold data*/
engine eng1;


/*##############################*/
/*set things up*/

void setup()
{
  // setup display, if needed
  #ifdef DEBUG
  Serial.begin(9600);
  #endif

  // set positions and pin numbers
  // pins are inRpPin, inRnPin, inGpPin, inGnPin, inBpPin, inBnPin, inthrot1Pin
  eng1.setup(12,6,11,5,10,4,A5);
}

/*##############################*/
/*main loop*/

void loop() {

  // read controls
  eng1.readInputs();

  eng1.setPhase();

  // write outputs
  eng1.writeState();

}/*main loop*/

/*the end*/
/*##############################*/

