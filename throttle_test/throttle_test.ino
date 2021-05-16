/*#####################*/
/*# Program to drive  #*/
/*# throttle response #*/
/*# for bucc cockpit  #*/
/*# TBAG 2021         #*/
/*#####################*/

//#define DEBUG

// set all the pins
const unsigned char RpPin=12;     // red positive pin
const unsigned char RnPin=6;      // red negative pin
const unsigned char GpPin=11;     // green
const unsigned char GnPin=5;      // green
const unsigned char BpPin=10;     // blue
const unsigned char BnPin=4;      // blue
const unsigned char throt1Pin=A5; // input for throttle 1

class procedures{
  public:
    // methods
    void setup();
    void readInputs();
    void determineState();
    void writeState();

  private:
    // methods
    void setPhase(float);
    // fixed constants
    const float maxRPM=4000/60.0;      // maximum RPM for tachometers, in Hz

    // throttle controls
    float throtPos1;  // throttle position, in %
    char cockPos;     // fuel cock position. On/off
  
    // throttle internals
    const float tRate=100.0/50.0;       // throttle change rate, in % per second
    float tim;         // time now
    char rDir;         // red tacho directions
    char gDir;         // green tacho directions
    char bDir;         // blue tacho directions
  
    // throttle outputs
    float rpm1;        // tachometer RPM, in %
    float temp1;       // exhaust temperature
  
}proc;

/*##############################*/
/*determine tachometer phases*/

void procedures::setPhase(float thisTime)
{
  float l=0,fracL=0;
  const float third=1.0/3.0;

  if(rpm1>0.001){
    l=1.0/rpm1;   /*wavelength in seconds*/
    fracL=(tim/l)-(float)(unsigned long int)(tim/l);     /* fraction of a wavelength*/
    
    if(fracL<0.5)rDir=1;   // red
    else         rDir=-1;
    if((fracL>=third)&&(fracL<(third+0.5)))gDir=1;  // green
    else                                   gDir=-1;
    if((fracL>(2.0*third))||(fracL<(2.0*third-0.5)))bDir=1; // blue
    else                                            bDir=-1;
  }else rDir=bDir=gDir=0;

  #ifdef DEBUG
  Serial.print("Pos ");
  Serial.print(throtPos1); 
  Serial.print(" RPM ");
  Serial.print(rpm1*60.0,4);
  Serial.print(" time ");
  Serial.print(tim,4);
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

void procedures::setup()
{
  // inputs
  throtPos1=0.0; // starts with throttle closed
  cockPos=1;     // hard-coded open for now, as now switch

  // internals
  
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

void procedures::readInputs()
{
  //throttle
  throtPos1=(float)map(analogRead(throt1Pin), 0, 1024, 0, maxRPM);

  return;
}

/*##############################*/
/*adjust the states*/

void procedures::determineState()
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
  setPhase(thisTime);

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

void procedures::writeState()
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
  if(bDir>0){  // green phase
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
/*set things up*/

void setup() {
  // allocate pins
  pinMode(RpPin, OUTPUT);
  pinMode(RnPin, OUTPUT);
  pinMode(GpPin, OUTPUT);
  pinMode(GnPin, OUTPUT);
  pinMode(BpPin, OUTPUT);
  pinMode(BnPin, OUTPUT);
  pinMode(throt1Pin, INPUT);

  // set positions etc.
  proc.setup();


  // set all pins LOW
  digitalWrite(RpPin,LOW);
  digitalWrite(RnPin,LOW);
  digitalWrite(GpPin,LOW);
  digitalWrite(GnPin,LOW);
  digitalWrite(BpPin,LOW);
  digitalWrite(BnPin,LOW);
}

/*##############################*/
/*main loop*/

void loop() {

  // read controls
  proc.readInputs();

  // determine state
  proc.determineState();

  // write outputs
  proc.writeState();
  
}/*main loop*/

/*the end*/
/*##############################*/

