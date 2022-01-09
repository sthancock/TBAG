

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
      self.coefF=0.4
    else:  # LP
      self.m=200.0
      self.r=0.5
      self.coefF=0.4

    self.isHP=isHP
    self.rpm=0.0

    return

  ####################

  def determineRPM(self,airStart,throtGate,throtPos,otherRPM,dTim):
    '''Determine RPM of a stage'''

    airForce=1.0   # force from airstart
    jetForce=2.0   # force per unit throttle position

    if(self.isHP==0):
      forceLoss=(1.0-otherRPM)*100.0
    else:
      forceLoss=self.rpm*jetForce

    # at min throttle, HP is in equilibrium at 50% RPM
    # at max throttle, HP is in equilibrium at 100% RPM

    f=airStart*airForce+throtGate*jetForce*(throtPos/2.0+0.5)-(forceLoss)
    a=f/(self.m*self.r*0.5)

    self.rpm=self.rpm+a*dTim



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

    # is air start on?
    if(self.hpStage.rpm<0.35):
      self.airStart=1
    else:
      self.airStart=0

    if(self.hpStage.rpm>=0.12):
      self.throtGate=1
    else:
      self.throtGate=0

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
  while(tim<300.0):
    if(tim>=150):
      eng.throtPos=1.0


    eng.determineState(dTim)

    eng.writeStatus(tim)

    tim=tim+dTim


