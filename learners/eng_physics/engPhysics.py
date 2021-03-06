

'''
Script for testing engine physics
before implementing on Arduino
procedure trainer

S Hancock                  2022
'''

#########################################

class jetStage():
  '''Class to store a jet engine stage'''

  ####################

  def __init__(self,isHP=1):
    '''Class initialiser'''

    # is this HP or LP stage?
    if(isHP):
      self.m=200.0
      self.r=0.5
      self.equil=0.5
      self.coefF=1
    else:  # LP
      self.m=400.0
      self.r=0.75
      self.equil=0.4
      self.coefF=1.1

    self.rpm=0.0

    return

  ####################

  def determineRPM(self,airStart,throtGate,throtPos,otherRPM,dTim):
    '''Determine RPM of a stage'''

    airForce=5.0   # force from airstart
    jetForce=10.0   # force per unit throttle position

    forceLoss=self.rpm*jetForce

    # at min throttle, HP is in equilibrium at 50% RPM
    # at max throttle, HP is in equilibrium at 100% RPM

    f=airStart*airForce+throtGate*jetForce*(throtPos/2.0+self.equil)-(forceLoss+self.coefF)
    a=f/(self.m*self.r*0.5)

    self.rpm=self.rpm+a*dTim
    if(self.rpm<0.0):
      self.rpm=0.0



#########################################

class engine():
  '''A class to hold the engine bits'''

  ####################

  def __init__(self):
    '''Class initialiser'''

    self.hpStage=jetStage()
    self.lpStage=jetStage(isHP=0)
    self.temp=0.0
    self.throtGate=0
    self.throtPos=0.0

    return

  ####################

  def determineState(self,dTim):
    '''Determine state of the engine'''

    # determine HP rpm
    self.hpStage.determineRPM(self.airStart,self.throtGate,self.throtPos,self.lpStage.rpm,dTim)

    # determine LP rpm
    self.lpStage.determineRPM(0,self.throtGate,self.throtPos,self.hpStage.rpm,dTim)

    # determine temperature
    self.temp=0.0


  ####################

  def writeStatus(self,tim):
    '''Write engine status'''

    print(tim,self.hpStage.rpm,self.lpStage.rpm,self.temp)

    return


#########################################

if __name__=="__main__":
  '''Main block'''

  # set time to 0 and step size
  tim=0.0
  dTim=0.01

  # create class
  eng=engine()


  # loop over time
  while(tim<700.0):

    if((tim<500)&(eng.hpStage.rpm>=0.12)):
      eng.throtGate=1

    if((tim<500)&(eng.hpStage.rpm<0.35)):
      eng.airStart=1
    else:
      eng.airStart=0

    if(tim>500):
      eng.throtPos=0.0
      eng.throtGate=0
    if(tim>300):
      eng.throtPos=0.0
    elif(tim>=150):
      eng.throtPos=1.0

    eng.determineState(dTim)

    eng.writeStatus(tim)

    tim=tim+dTim


