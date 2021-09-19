/*#####################*/
/*# Program to drive  #*/
/*# throttle response #*/
/*# for bucc cockpit  #*/
/*# TBAG 2021         #*/
/*#####################*/


//#define DEBUG

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
    int8_t rMode;     // Red phase +ve or -ve
    int8_t gMode;     // Green phase +ve or -ve
    int8_t bMode;     // Blue phase +ve or -ve
    int8_t lastRmode; // Last red phase mode
    int8_t lastGmode; // Last red phase mode
    int8_t lastBmode; // Last red phase mode
    float tim;        // time now
  
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
  float dAng=0;  // angle change rate
  float rAng=0,gAng=0,bAng=0;
  uint32_t angFrac=0,offset=10;

  if(rpm1>0.001){
    dAng=360.0*maxRPM*rpm1/100.0;

    tachAng+=dAng*dTime;

    // to prevent loss of precision when running for long periods
    while(tachAng>360000.0){   // 1000 rotations at a time to save
      tachAng-=360000.0;        // computational time
    }
    while(tachAng<0.0){   // keep positive
      tachAng+=360.0;
    }

    angFrac=(uint32_t)tachAng%360;

    // Decide which pins are ground and which are live
    if((angFrac>(0+offset))&&(angFrac<(180-offset)))rMode=1;
    else                                            rMode=0;
    if((angFrac>(120+offset))&&(angFrac<(300-offset)))gMode=1;
    else                                              gMode=0;
    if((angFrac>(240+offset))||(angFrac<(60-offset)))bMode=1;
    else                                             bMode=0;
  }else rMode=gMode=bMode=0;

  #ifdef DEBUG  // write to display to monitor
  Serial.print("Pos ");
  Serial.print(throtPos1); 
  Serial.print(" RPM ");
  Serial.print(rpm1*maxRPM,4);
  Serial.print(" time ");
  Serial.print(tim,4);
  Serial.print(" ang ");
  Serial.print(tachAng);
  Serial.print(" angFrac ");
  Serial.print(angFrac);
  Serial.print(" rMode ");
  Serial.print(rMode);
  Serial.print(" gMode ");
  Serial.print(gMode);
  Serial.print(" bMode ");
  Serial.print(bMode);
  Serial.print(" tim ");
  Serial.print(tim);
  Serial.print(" dtim ");
  Serial.print(dTime,10);

  Serial.print("\n");
  #endif

  return;
}//engine::setPhase


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
  rMode=gMode=bMode=0;
  lastRmode=lastGmode=lastBmode=0;

  return;
}//engine::setup

/*##############################*/
/*read input controls*/

void engine::readInputs()
{
  //throttle
  throtPos1=(float)map(analogRead(throt1Pin), 0, 1024, 0, maxRPM);

  return;
}

/*##############################*/
/*adjust the states*/

void engine::determineState()
{
  float dFuel=0;
  float thisTime=0,dTime=0;
  

  // time change since last call?
  thisTime=micros()/1000000.0;
  dTime=thisTime-tim;
  tim=thisTime;
  

  // determine delta fuel
  if(cockPos)dFuel=throtPos1-rpm1;
  else       dFuel=-1.0*rpm1;

  // update rpm and temperatures
  rpm1+=dFuel*tRate*dTime;
  if(rpm1>50.0)temp1=50.0;
  else         temp1=0.0;

  //set tachometer phase
  setPhase(thisTime,dTime);

  #ifdef DEBUG
  /*Serial.print("ThrotPos: ");
  Serial.print(throtPos1);
  Serial.print(" RPM: ");
  Serial.print(rpm1);
  Serial.print(" dtime ");
  Serial.print(dTime);
  Serial.print(" dFuel ");
  Serial.print(dFuel);
  Serial.print("\n");*/
  #endif

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
  if(rMode!=lastRmode){
    if(rMode==1){
      digitalWrite(RnPin,LOW);
      digitalWrite(RpPin,HIGH);
    }else{
      digitalWrite(RpPin,LOW);
      digitalWrite(RnPin,HIGH);
    }
    lastRmode=rMode;
  }  
  if(gMode!=lastGmode){
    if(gMode==1){
      digitalWrite(GnPin,LOW);
      digitalWrite(GpPin,HIGH);
    }else{
      digitalWrite(GpPin,LOW);
      digitalWrite(GnPin,HIGH);
    }
    lastGmode=gMode;
  }  
  if(bMode!=lastBmode){
    if(bMode==1){
      digitalWrite(BnPin,LOW);
      digitalWrite(BpPin,HIGH);
    }else{
      digitalWrite(BpPin,LOW);
      digitalWrite(BnPin,HIGH);
    }
    lastBmode=bMode;
  }  

  #ifdef DEBUG
  //Serial.print("ThrotPos: ");
  //Serial.print(throtPos1);
  //Serial.print(" RPM: ");
  //Serial.print(rpm1);
  //Serial.print("\n");
  #endif

  return;
}


/*##############################*/
/*global classes to hold data*/
engine eng1;
engine eng2;


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
  //eng2.setup(1,2,3,7,9,13,A6);
}

/*##############################*/
/*main loop*/

void loop() {

  // read controls
  eng1.readInputs();
  //eng2.readInputs();

  // determine state
  eng1.determineState();
  //eng2.determineState();

  // write outputs
  eng1.writeState();
  //eng2.writeState();

}/*main loop*/

/*the end*/
/*##############################*/

