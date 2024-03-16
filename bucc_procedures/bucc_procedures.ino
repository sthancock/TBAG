/*########################*/
/*# Program to drive     #*/
/*# buccaneer procedures #*/
/*# trainer              #*/
/*# TBAG 2021            #*/
/*########################*/


/*turn debug flags on or off*/
/*note that with DEBUG on the program runs*/
/*too slow for RPM gauges to spin         */

#define DEBUG
//#define BREADBOARD

/*#####################################*/
/*pin assignments*/

#ifndef BREADBOARD  /*then this is the PCB arrangment*/
 /*engines*/
  #define AIRst 44
  #define LPSPIN 45

  /*engine 1*/
  /*in*/
  #define THROT1 A0
  #define E1mast 34
  #define E1cock 37
  #define E1START 38
  /*out*/
  #define E1hpR 25  /*the order of these will need checking*/
  #define E1hpG 27
  #define E1hpB 26
  #define E1lpR 22
  #define E1lpG 23
  #define E1lpB 24
  #define E1jpt 4
  #define E1OILP 40
  /*need another pin here*/

  /*engine 2*/
  /*in*/
  #define THROT2 A1
  #define E2mast 35
  #define E2cock 35
  #define E2START 37
  /*out*/
  #define E2hpR 31
  #define E2hpG 32
  #define E2hpB 33
  #define E2lpR 28
  #define E2lpG 29
  #define E2lpB 30
  #define E2jpt 3
  #define E2OILP 42
#else               /*then this is the Breadboard arrangment*/
  /*engines*/
  #define AIRst 22
  #define LPSPIN 26

  /*engine 1*/
  /*in*/
  #define THROT1 A5
  #define E1mast 25
  #define E1cock 24
  #define E1START 23
  /*out*/
  #define E1hpR 33
  #define E1hpG 32
  #define E1hpB 31
  #define E1lpR 28
  #define E1lpG 29
  #define E1lpB 30
  #define E1jpt 3
  #define E1OILP 27

  /*engine 2*/
  /*in*/
  #define THROT2 A4
  #define E2mast 36
  #define E2cock 35
  #define E2START 34
  /*out*/
  #define E2hpR 42
  #define E2hpG 41
  #define E2hpB 40
  #define E2lpR 43
  #define E2lpG 38
  #define E2lpB 39
  #define E2jpt 4
  #define E2OILP 44
#endif

/*#####################################*/




/*#####################################*/
/*global variables*/

const float maxRPM=5100/60.0;      // maximum RPM for tachometers, in Hz
const float throtGate=0.1*maxRPM;  // min throttle gate position
const float tRate=30.0/60.0;       // engine RPM increase rate with fuel, in % per second
const float asRate=60.0/60.0;      // engine RPM increase rate with air start, in % per second
const float engDecRate=-100.0/60.0; // engine RPM decrease rate, in % per second

const float airStartF=5.0;   // Air start force in N
const float jetForc=10.0;    // fuel burn force in N per unit


/*#####################################*/
/*hold data for a jet stage*/

class jetStage{
  public:
    // methods
    void setup(bool,int8_t,int8_t,int8_t);
    bool determineRPM(bool,bool,bool,bool,float);
    float getRPM();
    void writeState();

  private:
    // methods
    void setPhase(float,float);

    // data

    // stage characteristics
    float massRad;  // turbine mass times radius
    float rpmEquil;  // rpm equilibrium

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

bool jetStage::determineRPM(bool alight,bool airStart,bool engMaster,bool starting,float throtPos)
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
    if(rpm>=0.5*maxRPM)starting=0; // startup has finished
    
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

  return(starting);
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
    void setup(int8_t,int8_t,int8_t,int8_t,int8_t,int8_t,int8_t,int8_t,int8_t,int8_t,int8_t,int8_t,int8_t,int8_t);
    void readInputs();
    void determineState();
    void writeState();

  private:
    // methods
    void setJPT();
    void setLPspinLight(float);

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
    bool lpSpinLight; // LP spin light on
    float tim;        // time for LP spin light
  
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
    int8_t lpLightPin;/*LP spin light pin*/
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

void engine::setup(int8_t inHpAPin,int8_t inHpBPin,int8_t inHpCPin,int8_t inthrotPin,\
                   int8_t inJPTpin,int8_t engMasPinIn,int8_t cockPinIn,\
                   int8_t startPinIn,int8_t aiaPinIn,int8_t lpLightIn,int8_t oilPlightIn,\
                   int8_t inLpAPin,int8_t inLpBPin,int8_t inLpCPin)
{
  // set bin variables
  throtPin=inthrotPin;
  jptPin=inJPTpin;
  cockPin=cockPinIn;
  engMasPin=engMasPinIn;
  startPin=startPinIn;
  aiaPin=aiaPinIn;
  oilPlight=oilPlightIn;
  lpLightPin=lpLightIn;

  // set pin modes
  pinMode(jptPin, OUTPUT);
  pinMode(throtPin, INPUT); // input
  pinMode(engMasPin,INPUT);   
  pinMode(cockPin,INPUT);   
  pinMode(startPin,INPUT);   
  pinMode(aiaPin,INPUT);   
  pinMode(oilPlight,OUTPUT);   
  pinMode(lpLightPin,OUTPUT);   

  // set all output pins LOW
  analogWrite(jptPin,0);
  digitalWrite(oilPlight,HIGH);
  digitalWrite(lpLightPin,LOW);

  // inputs
  throtPos=0.0;  // starts with throttle closed
  cockPos=0;     // all switches off for now
  airStart=0;    // all switches off for now
  engMaster=0;   // all switches off for now
  engStart=0;    // all switches off for now

  // setup two stages
  hpStage.setup(1,inHpAPin,inHpBPin,inHpCPin);
  lpStage.setup(0,inLpAPin,inLpBPin,inLpCPin);

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
  airStart=1;  //digitalRead(aiaPin);  current PCB has error that prevents this. Lock on
  engMaster=digitalRead(engMasPin);
  engStart=digitalRead(startPin);

  return;
}/*engine::readInputs*/


/*##############################*/
/*determine JPT*/

void engine::setJPT()
{
  float dTemp=0;

  // determine temperature change
  if(alight){
    dTemp=throtPos/maxRPM; //-rpm/1000.0;
  }else{
    dTemp=-1.0*temp/500.0; //-rpm/1000.0;
  }  
  // hack for now
  if(alight)temp=500.0;
  else temp=0.0;
  //temp+=dTemp;
  if(temp<0.0)temp=0.0;
  else if(temp>800)temp=800.0;

  return;
}/*engine::setJPT*/


/*##############################*/
/*is LP spin light on?*/

void engine::setLPspinLight(float lpRPM)
{
  static float tLastChange;
 
  /*is it spinning slowly?*/
  if((lpRPM>0.00001)&&(lpRPM<0.3*maxRPM)){

    if(tLastChange<0.0)tLastChange=tim;

    // time change since last call?
    tim=micros()/1000000.0;
    if((tim-tLastChange)>=0.5){
      if(lpSpinLight)lpSpinLight=0;
      else           lpSpinLight=1;

      tLastChange=tim;
    }
  }else{
    lpSpinLight=0;
    tLastChange=-1.0;
  }
  
  return;
}/*engine::setLPspinLight*/


/*##############################*/
/*adjust the states*/

void engine::determineState()
{
  bool blank=0;

  // is engine alight?
  if(alight||starting||engStart){
    if(cockPos&&(throtPos>=throtGate)&&(hpStage.getRPM()>=0.12*maxRPM))alight=1;    
    else                                                               alight=0;
  }else alight=0;

  // update RPMs
  starting=hpStage.determineRPM(alight,airStart,engMaster,engStart||starting,throtPos);
  blank=lpStage.determineRPM(alight,airStart,engMaster,engStart||starting,throtPos);

  // set temperatures
  setJPT();

  // is oil pressure warning on?
  if(hpStage.getRPM()>0.35*maxRPM)oilPress=0;
  else                            oilPress=1;

  // is LP spin light on?
  if(starting||engStart)setLPspinLight(hpStage.getRPM());
  else                  lpSpinLight=0;

  #ifdef DEBUG
  Serial.print("Throt ");
  Serial.print(throtPos);
  Serial.print(" HPrpm ");
  Serial.print(hpStage.getRPM());
  Serial.print(" LPrpm ");
  Serial.print(lpStage.getRPM());
  Serial.print(" JPT ");
  Serial.print(temp);
  Serial.print(" engSwitch ");
  Serial.print(engMaster);
  Serial.print(" cockPos ");
  Serial.print(cockPos);
  Serial.print(" engStart ");
  Serial.print(engStart);
  Serial.print(" airStart ");
  Serial.print(airStart);
  Serial.print("\n");
  #endif

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

  // oil pressure
  if(oilPress&&engMaster)digitalWrite(oilPlight,HIGH);
  else                   digitalWrite(oilPlight,LOW);

  // LP turbine light
  if(lpSpinLight)digitalWrite(lpLightPin,HIGH);
  else           digitalWrite(lpLightPin,LOW);

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
  // pins are hpRPMaPin, hpRPMbPin, hpRPMcPin,throtPin,JPTpin,engMasPin,cockPin,startPin,airstartPin,lpLightPin,oilPlightPin
  eng1.setup(E1hpR,E1hpG,E1hpB,THROT1,E1jpt,E1mast,E1cock,E1START,AIRst,LPSPIN,E1OILP,E1lpR,E1lpG,E1lpB);
  eng2.setup(E2hpR,E2hpG,E2hpB,THROT2,E2jpt,E2mast,E2cock,E2START,AIRst,LPSPIN,E2OILP,E2lpR,E2lpG,E2lpB);

  return;
}/*setup*/


/*##############################*/
/*main loop*/

void loop() {

  // read controls
  //eng1.readInputs();
  eng2.readInputs();

  // determine state
  //eng1.determineState();
  eng2.determineState();

  // write outputs
  //eng1.writeState();
  eng2.writeState();

  return;
}/*main loop*/

/*the end*/
/*##############################*/
