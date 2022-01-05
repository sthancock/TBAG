
'''
Script to calculate the
resistance needed to 
pair with the TGT
'''



Vop=18*10**-3   # operating voltage
resTGT=33.5     # TGT gauge resistance

resDamp=51      # damper resistance
Vin=5           # voltage in

x=(Vin/Vop-1)*resTGT

print(x)




