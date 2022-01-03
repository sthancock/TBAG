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
/*hold data and functions for engines*/

class engine{
  public:
    // methods
    void setup(int8_t,int8_t,int8_t,int8_t,int8_t,int8_t,int8_t,int8_t,int8_t);
    void readInputs();
    void determineState();
    void writeState();

  private:
    // methods
    void setPhase(float,float);

    // engine controls
    float throtPos;   // throttle position, in %
    bool engMaster;   // engine master switch
    bool cockPos;     // fuel cock position. On/off
    bool engStart;    // engine start switch
    bool airStart;    // air start position. On/off
  
    // engine internals
    float tachAng;     // phase angle of tachometer
    int8_t rMode;     // Red phase +ve or -ve
    int8_t gMode;     // Green phase +ve or -ve
    int8_t bMode;     // Blue phase +ve or -ve
    int8_t lastRmode; // Last red phase mode
    int8_t lastGmode; // Last red phase mode
    int8_t lastBmode; // Last red phase mode
    float tim;        // time now
    bool alight;      // is fuel alight, off/on
    bool starting;    // engine startup procedure running
  
    // throttle outputs
    float rpm;        // tachometer RPM, in %
    float temp;        // exhaust temperature

    // arduino pins
    int8_t rPin;     // RPM gauge red phase output
    int8_t gPin;     // RPM gauge green phase output
    int8_t bPin;     // RPM gauge blue phase output
    int8_t throtPin; // input for throttle
    int8_t jptPin;   // JPT gauge output
    int8_t engMasPin;// engine master switch pin
    int8_t cockPin;  // fuel cock pin        
    int8_t startPin; // engine start switch pin
    int8_t airPin;   // air start pin
}; /*engine class*/


/*##############################*/
/*determine tachometer phases*/

void engine::setPhase(float thisTime,float dTime)
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
    if((angFrac>(0+offset))&&(angFrac<(180-offset)))bMode=1;
    else                                            bMode=0;
    if((angFrac>(120+offset))&&(angFrac<(300-offset)))gMode=1;
    else                                              gMode=0;
    if(((angFrac>(240+offset))||(angFrac<(60-offset))))rMode=1;
    else                                               rMode=0;
  }else rMode=gMode=bMode=0;

  #ifdef DEBUG  // write to display to monitor
  Serial.print("Pos ");
  Serial.print(throtPos); 
  Serial.print(" RPM ");
  Serial.print(rpm*maxRPM,4);
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
  Serial.print(" alight ");
  Serial.print(alight);
  Serial.print("\n");
  #endif

  return;
}/*engine::setPhase*/


/*##############################*/
/*internal setup*/

void engine::setup(int8_t inRPin,int8_t inGPin,int8_t inBPin,int8_t inthrotPin,\
                   int8_t inJPTpin,int8_t engMasPinIn,int8_t cockPinIn,\
                   int8_t startPinIn,int8_t airPinIn)
{
  // set bin variables
  rPin=inRPin;
  gPin=inGPin;
  bPin=inBPin;
  throtPin=inthrotPin;
  jptPin=inJPTpin;
  cockPin=cockPinIn;
  engMasPin=engMasPinIn;
  startPin=startPinIn;
  airPin=airPinIn;

  // set pin modes
  pinMode(rPin, OUTPUT);     // output
  pinMode(gPin, OUTPUT);
  pinMode(bPin, OUTPUT);
  pinMode(jptPin, OUTPUT);
  pinMode(throtPin, INPUT); // input
  pinMode(engMasPin,INPUT);   
  pinMode(cockPin,INPUT);   
  pinMode(startPin,INPUT);   
  pinMode(airPin,INPUT);   

  // set all output pins LOW
  digitalWrite(rPin,LOW);
  digitalWrite(gPin,LOW);
  digitalWrite(bPin,LOW);
  analogWrite(jptPin,0);
  
  // inputs
  throtPos=0.0;  // starts with throttle closed
  cockPos=0;     // all switches off for now
  airStart=0;    // all switches off for now
  engMaster=0;   // all switches off for now
  engStart=0;    // all switches off for now

  // internals
  tachAng=0.0;
  alight=0;
  starting=0;
  
  // outputs
  rpm=0.0;     // everything off
  temp=0.0;
  rMode=gMode=bMode=0;
  lastRmode=lastGmode=lastBmode=0;

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
  airStart=digitalRead(airPin);
  engMaster=digitalRead(engMasPin);
  engStart=digitalRead(startPin);

  return;
}/*engine::readInputs*/


/*##############################*/
/*adjust the states*/

void engine::determineState()
{
  float dFuel=0,dRPM=0,dTemp=0;
  float thisTime=0,dTime=0;
  
  // time change since last call?
  thisTime=micros()/1000000.0;
  dTime=thisTime-tim;
  tim=thisTime;

  // is engine alight?
  if(alight||starting||engStart){
    if(cockPos&&(throtPos>=throtGate)&&(rpm>=0.12*maxRPM))alight=1;    
    else                                                  alight=0;
  }else alight=0;

  // is engine alight or not?
  if(!alight){ // needs air start to spin
    if(engMaster&&(engStart||starting)&&airStart){  // start procedure
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

  // determine temperature change
  if(alight){
    dTemp=throtPos/maxRPM; //-rpm/1000.0;
  }else{
    dTemp=-1.0*temp/500.0; //-rpm/1000.0;
  }
  
  // update rpm and temperatures
  rpm+=dRPM;
  if(rpm<0.0)rpm=0.0;

  if(alight)temp=500.0;
  else temp=0.0;
  //temp+=dTemp;
  if(temp<0.0)temp=0.0;
  else if(temp>800)temp=800.0;

  //set tachometer phase
  setPhase(thisTime,dTime);

  return;
}/*engine::determineState*/


/*##############################*/
/*write state*/

void engine::writeState()
{
  // RPM gauge, write 3 phases
  if(rMode!=lastRmode){
    if(rMode==1)digitalWrite(rPin,HIGH);
    else        digitalWrite(rPin,LOW);
    lastRmode=rMode;
  }  
  if(gMode!=lastGmode){
    if(gMode==1)digitalWrite(gPin,HIGH);
    else        digitalWrite(gPin,LOW);
    lastGmode=gMode;
  }  
  if(bMode!=lastBmode){
    if(bMode==1)digitalWrite(bPin,HIGH);
    else        digitalWrite(bPin,LOW);
    lastBmode=bMode;
  }

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
  // pins are redPin, greenPin, bluePin,throtPin, inJPTpin,engMasPin,cockPin,startPin,airPin
  eng1.setup(4,5,6,A5,3,25,24,23,22);
  eng2.setup(7,8,9,A6,2,26,27,28,29);
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
