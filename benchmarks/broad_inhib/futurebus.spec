

vars
     invalid
     sharedU
     exclusiveU
     exclusiveM
     pendingR
     pendingW
     pendingEMR
     pendingEMW
     pendingSU	

rules
invalid >= 1, pendingW=0, pendingR>=1 ->
 
	      invalid'=invalid-1,
	      sharedU'=0,
	      exclusiveU'=0,
	      exclusiveM'=0,    
              pendingR'=pendingR+1,
	      pendingEMR'=pendingEMR+exclusiveM,
	      pendingSU'=sharedU+exclusiveU ;
	      
invalid >= 1, pendingW=0, pendingSU>=1 ->
 
	      invalid'=invalid-1,
	      sharedU'=0,
	      exclusiveU'=0,
	      exclusiveM'=0,    
              pendingR'=pendingR+1,
	      pendingEMR'=pendingEMR+exclusiveM,
	      pendingSU'=sharedU+exclusiveU ;
	      
invalid >= 1, pendingW=0, pendingEMR>=1 ->
 
	      invalid'=invalid-1,
	      sharedU'=0,
	      exclusiveU'=0,
	      exclusiveM'=0,    
              pendingR'=pendingR+1,
	      pendingEMR'=pendingEMR+exclusiveM,
	      pendingSU'=sharedU+exclusiveU ;
	      
invalid >= 1, pendingW=0, sharedU>=1 ->
 
	      invalid'=invalid-1,
	      sharedU'=0,
	      exclusiveU'=0,
	      exclusiveM'=0,    
              pendingR'=pendingR+1,
	      pendingEMR'=pendingEMR+exclusiveM,
	      pendingSU'=sharedU+exclusiveU ;

invalid >= 1, pendingW=0, exclusiveU>=1 ->
 
	      invalid'=invalid-1,
	      sharedU'=0,
	      exclusiveU'=0,
	      exclusiveM'=0,    
              pendingR'=pendingR+1,
	      pendingEMR'=pendingEMR+exclusiveM,
	      pendingSU'=sharedU+exclusiveU ;
 
invalid >= 1, pendingW=0, exclusiveM>=1 ->
 
	      invalid'=invalid-1,
	      sharedU'=0,
	      exclusiveU'=0,
	      exclusiveM'=0,    
              pendingR'=pendingR+1,
	      pendingEMR'=pendingEMR+exclusiveM,
	      pendingSU'=sharedU+exclusiveU ;
 
          
           pendingR >= 1 , pendingEMR >=1
         -> 
	      pendingR'=0,	      
	      sharedU'=sharedU+pendingSU+1,
	      pendingEMR'=pendingEMR-1
             ; 
         
          
	   invalid >= 1 , pendingW=0     , pendingEMW =0 ,
           pendingR=0   , pendingEMR =0  , pendingSU=0 , 
	   sharedU=0    , exclusiveU=0   , exclusiveM=0 
         -> 
	   exclusiveU'=exclusiveU+1,
	   invalid'=invalid-1
	 ; 
         
          invalid >= 1 
         -> 
               invalid'=invalid+exclusiveU+sharedU+pendingW+pendingSU+pendingR+pendingEMW+pendingEMR-1,
	       pendingW'   = 1,
	       pendingEMW' = exclusiveM, 
               sharedU' =0, 
	       exclusiveU' = 0, 
	       exclusiveM' =0,
	       pendingR' = 0,    
	       pendingEMR' = 0,
	       pendingSU' = 0   	 	
         ;
	 
         
           pendingW >= 1 , pendingEMW >=1
         -> 
	      pendingW'=0,
	      exclusiveM'=exclusiveM+pendingW,
	      invalid'=invalid+1,
	      pendingEMW'=pendingEMW-1
          ;
	 
	 
           pendingW >= 1 , pendingEMW =0
         -> 
	  exclusiveM'=exclusiveM+pendingW,
	  pendingW'=0
	 ;
	 
	 
init
 
               invalid   >= 1 , 
               sharedU    = 0 , 
	       exclusiveU = 0 , 
	       exclusiveM = 0 ,
	       pendingR   = 0 ,
	       pendingW   = 0 ,
	       pendingEMR = 0 ,
	       pendingEMW = 0 ,
	       pendingSU  = 0

target   	
		invalid    >= 0    , 
               sharedU    >= 1    , 
	       exclusiveU >= 0    , 
	       exclusiveM >= 1    ,
	       pendingR   >= 0    ,
	       pendingW   >= 0    ,
	       pendingEMR >= 0    ,
	       pendingEMW >= 0    ,
	       pendingSU  >= 0 
	       
	       invalid    >= 0    , 
               sharedU    >= 1    , 
	       exclusiveU >= 1    , 
	       exclusiveM >= 0    ,
	       pendingR   >= 0    ,
	       pendingW   >= 0    ,
	       pendingEMR >= 0    ,
	       pendingEMW >= 0    ,
	       pendingSU  >= 0 
	       
	       invalid    >= 0    , 
               sharedU    >= 0    , 
	       exclusiveU >= 1    , 
	       exclusiveM >= 1    ,
	       pendingR   >= 0    ,
	       pendingW   >= 0    ,
	       pendingEMR >= 0    ,
	       pendingEMW >= 0    ,
	       pendingSU  >= 0 
	       
	       invalid    >= 0    , 
               sharedU    >= 0    , 
	       exclusiveU >= 0    , 
	       exclusiveM >= 2    ,
	       pendingR   >= 0    ,
	       pendingW   >= 0    ,
	       pendingEMR >= 0    ,
	       pendingEMW >= 0    ,
	       pendingSU  >= 0 
	       
	       invalid    >= 0    , 
               sharedU    >= 0    , 
	       exclusiveU >= 2    , 
	       exclusiveM >= 0    ,
	       pendingR   >= 0    ,
	       pendingW   >= 0    ,
	       pendingEMR >= 0    ,
	       pendingEMW >= 0    ,
	       pendingSU  >= 0 
	       
	       invalid    >= 0    , 
               sharedU    >= 0    , 
	       exclusiveU >= 0    , 
	       exclusiveM >= 1    ,
	       pendingR   >= 0    ,
	       pendingW   >= 1    ,
	       pendingEMR >= 0    ,
	       pendingEMW >= 0    ,
	       pendingSU  >= 0 
	       
	       invalid    >= 0    , 
               sharedU    >= 1    , 
	       exclusiveU >= 0    , 
	       exclusiveM >= 0    ,
	       pendingR   >= 0    ,
	       pendingW   >= 1    ,
	       pendingEMR >= 0    ,
	       pendingEMW >= 0    ,
	       pendingSU  >= 0


