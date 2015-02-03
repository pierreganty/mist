#Manufacture exemple from DEP99

vars 
X1 X2 X3 X4 X5 X6 X7

rules 
#T1
X1>=2 -> 
X1'=X1-2 ,
X5'=X5+2 ;

#T2
X2>=1 -> 
X2'=X2-1 ,
X5'=X5+1 ;

#T3
X3>=1 -> 
X3'=X3-1 ,
X6'=X6+1 ;

#T4
X6>=1,X5>=4 -> 
X6'=X6-1 ,
X5'=X5-4 ,
X7'=X7+1 ;

#T5
X4>=1 -> 
X4'=X4-1 ,
X7'=X7+1 ;

#T6
X7>=2 -> 
X7'=X7-2 ,
X1'=X1+3 ,
X2'=X2+1 ,
X3'=X3+1 ,
X4'=X4+1 ;

init
X1=4,X2=0,X3=2,X4=1,X5=0,X6=0,X7=0

target
X1=1,X2=0,X3=0,X4=0,X5=3,X6=2,X7=1
