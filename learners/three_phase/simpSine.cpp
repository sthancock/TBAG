#include <iostream>
#include "math.h"
#include "stdlib.h"


char sinArr[360];   // pre-calculated array of sine +/-

/*##############################*/
/*main*/

int main()
{
  float tachAng=0,step=0;
  char rDir=0,bDir=0,gDir=0;
  void calcSin();

  /*set arrqy*/
  calcSin();

  /*call it*/
  tachAng=0.0;
  step=2.3;
  while(tachAng<7000.0){

    rDir=sinArr[(int)tachAng%360];
    bDir=sinArr[(int)(tachAng+120.0)%360];
    gDir=sinArr[(int)(tachAng+240.0)%360];

    fprintf(stdout,"%f %d %d %d\n",tachAng,rDir,bDir,gDir);

    tachAng+=step;
  }



  return(0);
}/*main*/


/*##############################*/
/*calculate a sine array*/

void calcSin()
{
  int i=0;
  float y=0;

  for(i=0;i<360;i++){
    y=sin((float)i*M_PI/180.0);
    if(y>0.5)      sinArr[i]=1;
    else if(y<-0.5)sinArr[i]=-1;
    else           sinArr[i]=0;
  }

  return;  
}/*calcSin*/
