/*########################*/
/*# Program to drive     #*/
/*# buccaneer procedures #*/
/*# trainer              #*/
/*# TBAG 2021            #*/
/*########################*/


//#define DEBUG

/*#####################################*/
/*global variables*/

const float maxRPM=5100/60.0;      // maximum RPM for tachometers, in Hz
const float throtGate=0.1*maxRPM;  // min throttle gate position
const float tRate=30.0/60.0;       // engine RPM increase rate with fuel, in % per second


/*#####################################*/
/*hold data for a jet stage*/

class jetStage{
  public:
    // methods
    void setup(bool,int8_t,int8_t,int8_t);
    void determineRPM(float);
    float getRPM();
    void writeState();

    // stage outputs
    float rpm;   // stage tachometer RPM, in %

  private:
    // methods
    void setPhase(float,float);

    // data

    // stage characteristics
    float massRad;  // turbine mass times radius
    float rpmEquil;  // rpm equilibrium


    // engine internals
    float tachAng;     // phase angle of tachometer
    int8_t aMode;     // A wire phase +ve or gnd
    int8_t bMode;     // B wire phase +ve or gnd
    int8_t cMode;     // C wire phase +ve or gnd
    int8_t lastAmode; // Last A phase mode
    int8_t lastBmode; // Last B phase mode
    int8_t lastCmode; // Last C phase mode
    float tim;        // time now

    // arduino pins
    int8_t aPin;     // RPM gauge red phase output
    int8_t bPin;     // RPM gauge green phase output
    int8_t cPin;     // RPM gauge blue phase output
  
};/*class to hold jet stage, LP or HP*/


/*#####################################*/
/*determine stage RPM*/

void jetStage::determineRPM(float throtPos)
{
  float dRPM=0,dFuel=0;
  float thisTime=0,dTime=0;
  
  // time change since last call?
  thisTime=micros()/1000000.0;
  dTime=thisTime-tim;
  tim=thisTime;

    
  // determine delta fuel
  dFuel=throtPos-rpm;
  //if(dFuel<0.0)dFuel=engDecRate;
  dRPM=dFuel*tRate*dTime;

  // update rpm and temperatures
  rpm+=dRPM;
  if(rpm<0.0)rpm=0.0;

  //set tachometer phase
  setPhase(thisTime,dTime);

  return;
}/*jetStage::determineRPM*/


/*#####################################*/
/*set parameters for HP or LP stage*/

void jetStage::setup(bool isHP,int8_t inAPin,int8_t inBPin,int8_t inCPin)
{

  // setup pins
  aPin=inAPin;
  bPin=inBPin;
  cPin=inCPin;
  
  pinMode(aPin, OUTPUT);     // output
  pinMode(bPin, OUTPUT);
  pinMode(cPin, OUTPUT);
  digitalWrite(aPin,LOW);
  digitalWrite(bPin,LOW);
  digitalWrite(cPin,LOW);
  
  // stage starting point
  tachAng=0.0;
  rpm=0.0;
  aMode=bMode=cMode=0;
  lastAmode=lastBmode=lastCmode=0;

  // stage charsacteristics
  if(isHP){  // set up for HP stage
    massRad=200.0*0.5;
    rpmEquil=0.5;
  }else{     // set up for LP stage
    massRad=400.0*0.75;
    rpmEquil=0.4;
  }/*HP-LP switch*/

  return;  
}/*jetStage::setup*/


/*#####################################*/
/*retrurn private RPM*/

float jetStage::getRPM()
{
  return(rpm);
}/*jetStage*/


/*#####################################*/
/*hold data and functions for engines*/

class engine{
  public:
    // methods
    void setup(int8_t,int8_t,int8_t,int8_t,int8_t);
    void readInputs();
    void determineState();
    void writeState();

  private:
    // methods
    void setJPT();

    // two stages
    jetStage hpStage;
    jetStage lpStage;

    // engine controls
    float throtPos;   // throttle position, in %
  
    // engine internals
    float tim;        // time for LP spin light
  
    // engine outputs
    float temp;       // exhaust temperature

    // arduino pins
    int8_t throtPin; // input for throttle
    int8_t jptPin;   // JPT gauge output
}; /*engine class*/


/*##############################*/
/*determine tachometer phases*/

void jetStage::setPhase(float thisTime,float dTime)
{
  float dAng=0;  // angle change rate
  float rAng=0,gAng=0,bAng=0;
  uint32_t angFrac=0,offset=0;

  if(rpm>0.001){
    dAng=360.0*maxRPM*rpm/100.0;

    tachAng+=dAng*dTime;

    // to prevent loss of precision when running for long periods
    while(tachAng>360000.0){   // 1000 rotations at a time to save
      tachAng-=360000.0;       // computational time
    }
    while(tachAng<0.0){   // keep positive
      tachAng+=360.0;
    }

    angFrac=(uint32_t)tachAng%360;

    // Decide which pins are ground (0) and which are live (1)
    if((angFrac>(0+offset))&&(angFrac<(180-offset)))cMode=1;
    else                                            cMode=0;
    if((angFrac>(120+offset))&&(angFrac<(300-offset)))bMode=1;
    else                                              bMode=0;
    if(((angFrac>(240+offset))||(angFrac<(60-offset))))aMode=1;
    else                                               aMode=0;
  }else aMode=bMode=cMode=0;

  return;
}/*engine::setPhase*/


/*##############################*/
/*internal setup*/

void engine::setup(int8_t inAPin,int8_t inBPin,int8_t inCPin,int8_t inthrotPin,\
                   int8_t inJPTpin)
{
  // set bin variables
  throtPin=inthrotPin;
  jptPin=inJPTpin;

  // set pin modes
  pinMode(jptPin, OUTPUT);
  pinMode(throtPin, INPUT); // input
 

  // set all output pins LOW
  analogWrite(jptPin,0);


  // inputs
  throtPos=0.0;  // starts with throttle closed

  // setup two stages
  hpStage.setup(1,inAPin,inBPin,inCPin);
  
  // outputs
  temp=0.0;   // everything off

  return;
}/*engine::setup*/


/*##############################*/
/*read input controls*/

void engine::readInputs()
{
  //throttle
  throtPos=(float)map(analogRead(throtPin), 0, 1024, 0, maxRPM);

  return;
}/*engine::readInputs*/


/*##############################*/
/*determine JPT*/

void engine::setJPT()
{
  float dTemp=0;

  // determine temperature change
  dTemp=throtPos/maxRPM; //-rpm/1000.0;
  if(temp<0.0)temp=0.0;
  else if(temp>800)temp=800.0;

  return;
}/*engine::setJPT*/


/*##############################*/
/*adjust the states*/

void engine::determineState()
{
  bool blank=0;

  // update RPMs
  hpStage.determineRPM(throtPos);

  #ifdef DEBUG
  Serial.print("Throtpos ");
  Serial.print(throtPos);
  Serial.print(" RPM ");
  Serial.print(hpStage.rpm);
  Serial.print("\n");
  #endif

  // set temperatures
  setJPT();

  return;
}/*engine::determineState*/


/*##############################*/
/*write stage state*/

void jetStage::writeState()
{
  
  // RPM gauge, write 3 phases
  if(aMode!=lastAmode){
    if(aMode==1)digitalWrite(aPin,HIGH);
    else        digitalWrite(aPin,LOW);
    lastAmode=aMode;
  }  
  if(bMode!=lastBmode){
    if(bMode==1)digitalWrite(bPin,HIGH);
    else        digitalWrite(bPin,LOW);
    lastBmode=bMode;
  }  
  if(cMode!=lastCmode){
    if(cMode==1)digitalWrite(cPin,HIGH);
    else        digitalWrite(cPin,LOW);
    lastCmode=cMode;
  }

  return;
}/*jetStage::writeState*/


/*##############################*/
/*write engine state*/

void engine::writeState()
{

  // turbine stage outputs
  hpStage.writeState();
  lpStage.writeState();

  // JPT gauge, write voltage
  analogWrite(jptPin,(int)(temp*255.0/800.0));

  return;
}/*engine::writeState*/


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
  // pins are hpRPMaPin, hpRPMbPin, hpRPMcPin,throtPin,JPTpin
  eng1.setup(4,5,6,A5,3);

  return;
}/*setup*/


/*##############################*/
/*main loop*/

void loop() {

  // read controls
  eng1.readInputs();

  // determine state
  eng1.determineState();

  // write outputs
  eng1.writeState();

  return;
}/*main loop*/

/*the end*/
/*##############################*/
