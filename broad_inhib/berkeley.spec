
vars  
invalid unowned nonexclusive exclusive
rules
          
           invalid >= 1  
         ->  
	       invalid'=invalid-1,
               unowned'=unowned+1,
               nonexclusive'=nonexclusive+exclusive,
               exclusive'=0 
	 ; 

          
           unowned >=1 
         ->  
	       nonexclusive'=0,
               unowned'=0,
               invalid'=unowned+invalid+nonexclusive-1,
               exclusive'=exclusive+1
         ; 

            nonexclusive >=1 
         ->  
	       nonexclusive'=0,
               unowned'=0,
               invalid'=unowned+invalid+nonexclusive-1,
               exclusive'=exclusive+1
         ; 
         
           invalid >= 1 
         -> 
	      invalid'=invalid+unowned+exclusive+nonexclusive-1,
              unowned'=0,
              nonexclusive'=0,
              exclusive'=1 
         ;  
          
           exclusive >=1
         -> invalid'=invalid+1,exclusive'=exclusive-1 ; 
          
           nonexclusive >= 1
         -> invalid'=invalid+1,nonexclusive'=nonexclusive-1 ; 
          
           unowned >=1
         -> invalid'=invalid+1,unowned'=unowned-1 ; 
init
 invalid >=1 ,  nonexclusive=0 , unowned = 0 , exclusive = 0
target
exclusive >= 2
 nonexclusive >= 1 , exclusive >= 1 
 unowned >= 1 , exclusive >= 1 
