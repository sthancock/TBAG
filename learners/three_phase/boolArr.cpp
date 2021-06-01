#include <iostream>
#include "math.h"
#include "stdlib.h"

int main()
{
  int i=0;
  float y=0;
  bool sinArr[360];

  for(i=0;i<360;i++){
    y=sin((float)i*M_PI/180.0);

    if(y>0.3)sinArr[i]=true;
    else     sinArr[i]=false;
  }


  for(i=0;i<360;i++){
    if(sinArr[i])fprintf(stdout,"%d\n",i);
  }



  return(0);
}

