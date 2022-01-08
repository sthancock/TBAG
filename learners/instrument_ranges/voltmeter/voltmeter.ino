/*#########################*/
/*# Program to test       #*/
/*# voltmeter insteuments #*/
/*# TBAG 2022             #*/
/*#########################*/


//#define DEBUG

/*#####################################*/
/*global variables*/

const float maxRPM=5100/60.0;      // maximum RPM for tachometers, in Hz
int8_t inPin;
int8_t outPin;


/*##############################*/
/*set things up*/

void setup()
{
  // setup display, if needed
  #ifdef DEBUG
  Serial.begin(9600);
  #endif

  // set positions and pin numbers
  // pins are input, output
  inPin=A5;
  outPin=3;

  pinMode(inPin, INPUT); // input
  pinMode(outPin, OUTPUT);

  analogWrite(outPin,0); 
}/*setup*/

/*##############################*/
/*main loop*/

void loop() {
  float throtPos=0,temp=0;

  // read controls
  throtPos=(float)map(analogRead(inPin), 0, 1024, 0, maxRPM);

  // set temp
  temp=throtPos*800.0/maxRPM; //-rpm/1000.0;

  // write outputs
  analogWrite(outPin,(int)(temp*255.0/800.0));


    #ifdef DEBUG  // write to display to monitor
  Serial.print("Pos ");
  Serial.print(throtPos); 
  Serial.print(" temp ");
  Serial.print(temp);
  Serial.print(" pwm ");
  Serial.print((int)(temp*255.0/800.0));
  Serial.print("\n");
  #endif

}/*main loop*/

/*the end*/
/*##############################*/
