#Swimming pool example

vars 
X1 X2 X3 X4 X5 X6 X7

rules 
#R1
X6>=1 -> 
X1'=X1+1 ,
X6'=X6-1 ;

#R2
X1>=1,X7>=1 -> 
X1'=X1-1 ,
X7'=X7-1 ,
X2'=X2+1 ;

#R3
X2>=1 -> 
X2'=X2-1 ,
X3'=X3+1 ,
X6'=X6+1 ;

#R4
X3>=1,X6>=1 -> 
X3'=X3-1  ,
X6'=X6-1  ,
X4'=X4+1 ;

#R5
X4>=1 -> 
X4'=X4-1 ,
X5'=X5+1 ,
X7'=X7+1 ;

#R6
X5>=1 -> 
X5'=X5-1  ,
X6'=X6+1  ;

init
X1=0,X2=0,X3=0,X4=0,X5=0,X6>=1,X7>=1 

target
X2=0,X4=0,X5=0,X6=0,X7=0 
X1=0,X2=0,X4=0,X5=0,X6=0
