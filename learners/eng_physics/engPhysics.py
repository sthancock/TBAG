

'''
Script for testing engine physics
before implementing on Arduino
procedure trainer

S Hancock                  2022
'''

#########################################

class jetStage():
  '''Class to store a jet engine stage'''

  def __init__(self,isHP=1):
    '''Class initialiser'''

    if(isHP):
      self.mass=200.0
      self.R=0.5
      self.coefF=0.4
    else:  # LP
      self.mass=200.0
      self.R=0.5
      self.coefF=0.4

    self.rpm=0.0

    return



#########################################

class eng():
  '''A class to hold the engine bits'''

  ####################

  def __init__(self):
    '''Class initialiser'''

    self.hpStage=jetStage()
    self.lpStage=jetStage(isHP=0)

    return

  ####################



#########################################

if __name__=="__main__":
  '''Main block'''

  tim=0.0
  dTim=0.01


  # loop over time
  while(tim<300.0){


    tim=tim+dTim


