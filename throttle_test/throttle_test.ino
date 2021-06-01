/*#####################*/
/*# Program to drive  #*/
/*# throttle response #*/
/*# for bucc cockpit  #*/
/*# TBAG 2021         #*/
/*#####################*/


//#define DEBUG

/*#####################################*/
/*global variables*/

char sinArr[360];   // pre-calculated array of sine +/-
const float maxRPM=4000/60.0;      // maximum RPM for tachometers, in Hz
const float tRate=100.0/50.0;       // throttle change rate, in % per second
float tim;         // time now


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
    char rDir;         // red tacho directions
    char gDir;         // green tacho directions
    char bDir;         // blue tacho directions
  
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

  if(rpm1>0.001){
    dAng=360.0*rpm1;
    tachAng+=dAng*dTime;

    // to prevent loss of precision when running for long periods
    while(tachAng>=360000.0){   // 1000 rotations at a time to save
      tachAng-=360000.0;        // computational time
    }

    rDir=sinArr[(int)tachAng%360];
    bDir=sinArr[(int)(tachAng+120.0)%360];
    gDir=sinArr[(int)(tachAng+240.0)%360];
  }else rDir=bDir=gDir=0;

  #ifdef DEBUG  // write to display to monitor
  Serial.print("Pos ");
  Serial.print(throtPos1); 
  Serial.print(" RPM ");
  Serial.print(rpm1*60.0,4);
  Serial.print(" time ");
  Serial.print(tim,4);
  Serial.print(" ang ");
  Serial.print(tachAng);
  Serial.print(" amp ");
  Serial.print(sin(tachAng),4);
  Serial.print(" ");
  Serial.print(sin(tachAng+120.0),4);
  Serial.print(" ");
  Serial.print(sin(tachAng+240.0),4);
  Serial.print(" ");
  //Serial.print(" phase ");
  Serial.print((int)rDir);
  Serial.print(" ");
  Serial.print((int)gDir);
  Serial.print(" ");
  Serial.print((int)bDir);
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
  rDir=gDir=bDir=0;

  // setup display, if needed
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
  
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
/*adjust the states*/

void engine::determineState()
{
  float dFuel=0;
  float thisTime=0,dTime=0;
  

  // time change since last call?
  thisTime=micros()/1000000.0;
  dTime=thisTime-tim;

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
  // update time
  tim=thisTime;
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
  if(rDir>0){  // red phase
    digitalWrite(RpPin,HIGH);
    digitalWrite(RnPin,LOW);
  }else if(rDir<0){
    digitalWrite(RpPin,LOW);
    digitalWrite(RnPin,HIGH);
  }else{
    digitalWrite(RpPin,LOW);
    digitalWrite(RnPin,LOW);
  }
  if(gDir>0){  // green phase
    digitalWrite(GpPin,HIGH);
    digitalWrite(GnPin,LOW);
  }else if(gDir<0){
    digitalWrite(GpPin,LOW);
    digitalWrite(GnPin,HIGH);
  }else{
    digitalWrite(GpPin,LOW);
    digitalWrite(GnPin,LOW);
  }
  if(bDir>0){  // blue phase
    digitalWrite(BpPin,HIGH);
    digitalWrite(BnPin,LOW);
  }else if(bDir<0){
    digitalWrite(BpPin,LOW);
    digitalWrite(BnPin,HIGH);
  }else{
    digitalWrite(BpPin,LOW);
    digitalWrite(BnPin,LOW);
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
/*calculate a sine array*/

void calcSin()
{
  int i=0;
  float y=0;

  for(i=0;i<360;i++){
    y=sin((float)i*M_PI/180.0);
    if(y>0.3)      sinArr[i]=1;
    else if(y<-0.3)sinArr[i]=-1;
    else           sinArr[i]=0;
  }

  return;  
}/*calcSin*/


/*##############################*/
/*set things up*/

void setup()
{
  void calcSin();

  // pre-calculate
  calcSin();

  // set positions and pin numbers
  eng1.setup(12,6,11,5,10,4,A5);
  eng2.setup(1,2,3,7,9,13,A6);
}

/*##############################*/
/*main loop*/

void loop() {

  // read controls
  eng1.readInputs();
  eng2.readInputs();

  // determine state
  eng1.determineState();
  eng2.determineState();

  // write outputs
  eng1.writeState();
  eng2.writeState();
  
}/*main loop*/

/*the end*/
/*##############################*/

