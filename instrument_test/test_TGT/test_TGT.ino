/*#######################*/
/*# Program to test a   #*/
/*# buccaneer TGT gauge #*/
/*# TBAG 2024           #*/
/*#######################*/



/*#####################################*/
/*pin assignments*/

#define VARPIN A1  /* potentiometer in*/
#define TGTPIN 8   /*PWM out*/


/*#####################################*/
/*variables*/

float varPos;   /*potentiometer position. 0-1*/


/*#####################################*/

void setup() {
  // put your setup code here, to run once:
  pinMode(VARPIN, INPUT);
  pinMode(TGTPIN,OUTPUT);
}


/*#####################################*/
/*the main loop*/

void loop() {

  /*read potentiometer position*/
  varPos=(float)map(analogRead(VARPIN),0,1024,0,1.0);

  /*set PWM*/
  analogWrite(TGTPIN,(int)(varPos));

  return;
}

/*#####################################*/                                             
