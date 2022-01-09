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
const float asRate=60.0/60.0;      // engine RPM increase rate with air start, in % per second
const float engDecRate=-100.0/60.0; // engine RPM decrease rate, in % per second


/*#####################################*/
/*hold data for a jet stage*/

class jetStage{
  public:
    // methods
    void setup(bool,int8_t,int8_t,int8_t);
    void determineRPM(bool,bool,bool,bool,float);
    float getRPM();
    void writeState();

  private:
    // methods
    void setPhase(float,float);

    // data

    // stage characteristics
    float mass;  // turbine mass
    float radH;  // half the turbine radius
    float coefF; // coefficient of friction

    // stage outputs
    float rpm;   // stage tachometer RPM, in %

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

void jetStage::determineRPM(bool alight,bool airStart,bool engMaster,bool starting,float throtPos)
{
  float dRPM=0,dFuel=0;
  float thisTime=0,dTime=0;
  
  // time change since last call?
  thisTime=micros()/1000000.0;
  dTime=thisTime-tim;
  tim=thisTime;

  // is engine alight or not?
  if(!alight){ // needs air start to spin
    if(engMaster&&starting&&airStart){  // start procedure
      starting=1;
      dRPM=asRate*dTime;
    }else dRPM=engDecRate*dTime;
  }else{  // engine is running
    starting=0; // startup has finished
    
    // determine delta fuel
    dFuel=throtPos-rpm;
    //if(dFuel<0.0)dFuel=engDecRate;
    dRPM=dFuel*tRate*dTime+asRate*dTime*(float)airStart;
  }

  
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

    
  }else{     // set up for LP stage

    
  }

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
    void setup(int8_t,int8_t,int8_t,int8_t,int8_t,int8_t,int8_t,int8_t,int8_t,int8_t,int8_t);
    void readInputs();
    void determineState();
    void writeState();

  private:
    // methods

    // two stages
    jetStage hpStage;
    jetStage lpStage;

    // engine controls
    float throtPos;   // throttle position, in %
    bool engMaster;   // engine master switch
    bool cockPos;     // fuel cock position. On/off
    bool engStart;    // engine start switch
    bool airStart;    // air start position. On/off
  
    // engine internals
    bool alight;      // is fuel alight, off/on
    bool starting;    // engine startup procedure running
    bool oilPress;    // oil pressure low light
  
    // engine outputs
    float temp;       // exhaust temperature

    // arduino pins
    int8_t throtPin; // input for throttle
    int8_t jptPin;   // JPT gauge output
    int8_t engMasPin;// engine master switch pin
    int8_t cockPin;  // fuel cock pin        
    int8_t startPin; // engine start switch pin
    int8_t aiaPin;   // air start pin
    int8_t oilPlight;// oil pressure light
    int8_t lpLight;  // LP spin light
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

    // Decide which pins are ground and which are live
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
                   int8_t inJPTpin,int8_t engMasPinIn,int8_t cockPinIn,\
                   int8_t startPinIn,int8_t aiaPinIn,int8_t lpLightIn,int8_t oilPlightIn)
{
  // set bin variables
  throtPin=inthrotPin;
  jptPin=inJPTpin;
  cockPin=cockPinIn;
  engMasPin=engMasPinIn;
  startPin=startPinIn;
  aiaPin=aiaPinIn;
  oilPlight=oilPlightIn;
  lpLight=lpLightIn;

  // set pin modes
  pinMode(jptPin, OUTPUT);
  pinMode(throtPin, INPUT); // input
  pinMode(engMasPin,INPUT);   
  pinMode(cockPin,INPUT);   
  pinMode(startPin,INPUT);   
  pinMode(aiaPin,INPUT);   
  pinMode(oilPlight,OUTPUT);   
  pinMode(lpLight,OUTPUT);   

  // set all output pins LOW
  analogWrite(jptPin,0);
  digitalWrite(oilPlight,HIGH);
  digitalWrite(lpLight,LOW);

  // inputs
  throtPos=0.0;  // starts with throttle closed
  cockPos=0;     // all switches off for now
  airStart=0;    // all switches off for now
  engMaster=0;   // all switches off for now
  engStart=0;    // all switches off for now

  // setup two stages
  hpStage.setup(1,inAPin,inBPin,inCPin);
  //lpStage.setup();

  // internals
  alight=0;
  starting=0;
  oilPress=1;
  
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

  //engine switches
  cockPos=digitalRead(cockPin);
  airStart=digitalRead(aiaPin);
  engMaster=digitalRead(engMasPin);
  engStart=digitalRead(startPin);

  return;
}/*engine::readInputs*/


/*##############################*/
/*adjust the states*/

void engine::determineState()
{
  float dTemp=0;

  // is engine alight?
  if(alight||starting||engStart){
    if(cockPos&&(throtPos>=throtGate)&&(hpStage.getRPM()>=0.12*maxRPM))alight=1;    
    else                                                               alight=0;
  }else alight=0;

  // update RPMs
  hpStage.determineRPM(alight,airStart,engMaster,engStart||starting,throtPos);

  // determine temperature change
  if(alight){
    dTemp=throtPos/maxRPM; //-rpm/1000.0;
  }else{
    dTemp=-1.0*temp/500.0; //-rpm/1000.0;
  }  

  // set temperatures
  if(alight)temp=500.0;
  else temp=0.0;
  //temp+=dTemp;
  if(temp<0.0)temp=0.0;
  else if(temp>800)temp=800.0;

  // is oil pressure warning on?
  if(hpStage.getRPM()>0.35*maxRPM)oilPress=0;
  else                            oilPress=1;

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

  // JPT gauge, write voltage
  analogWrite(jptPin,(int)(temp*255.0/800.0));

  // oil pressure
  if(oilPress)digitalWrite(oilPlight,HIGH);
  else        digitalWrite(oilPlight,LOW);

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
  // pins are redPin, greenPin, bluePin,throtPin, inJPTpin,engMasPin,cockPin,startPin,aiaPin,lpLightPin,oilPlightPin
  eng1.setup(4,5,6,A5,3,25,24,23,22,26,27);
  eng2.setup(7,8,9,A6,2,30,31,32,33,26,35);
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
