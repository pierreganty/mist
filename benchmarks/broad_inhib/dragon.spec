

vars
  invalid dirty exclusive shared shared_dirty
rules
         
           invalid >= 1 , 
           dirty =0 , 
           shared = 0 , 
           exclusive =0  , 
           shared_dirty=0
         -> 
	      invalid'=invalid-1,
              exclusive'=exclusive+1 
	 ; 
          
           invalid >= 1 , shared_dirty >= 1  
         -> 
	      invalid'=invalid-1,
              dirty'=0,
              shared_dirty'=shared_dirty+dirty,
              shared'=shared+exclusive+1,
              exclusive'=0 
	 ; 
          invalid >= 1 , exclusive >= 1  
         -> 
	      invalid'=invalid-1,
              dirty'=0,
              shared_dirty'=shared_dirty+dirty,
              shared'=shared+exclusive+1,
              exclusive'=0 
	 ; 
	 
           invalid >= 1 , shared >= 1  
         -> 
	      invalid'=invalid-1,
              dirty'=0,
              shared_dirty'=shared_dirty+dirty,
              shared'=shared+exclusive+1,
              exclusive'=0 
	 ; 
          invalid >= 1 , dirty >= 1  
         -> 
	      invalid'=invalid-1,
              dirty'=0,
              shared_dirty'=shared_dirty+dirty,
              shared'=shared+exclusive+1,
              exclusive'=0 
	 ; 
	 
	 
	 
	 
           dirty >=1 
         ->; 
          
           exclusive >= 1
         -> 
	     dirty'=dirty+1,
             exclusive'=exclusive-1 
	 ; 
          
            shared_dirty = 1 , shared =0   
          -> 
	       shared_dirty'=shared_dirty-1, 
               dirty'=dirty+1
	 ; 
          
            shared_dirty = 0 , shared =1   
          -> 
	       shared'=shared-1, 
               dirty'=dirty+1
	 ; 
              shared_dirty >= 2 
         ->  
	       shared'=shared+shared_dirty-1, 
               shared_dirty'=1
	 ; 
       
            shared >= 2 
         ->  
	       shared'=shared+shared_dirty-1, 
               shared_dirty'=1
	 ; 
             shared_dirty>=1,shared >= 1
         ->  
	       shared'=shared+shared_dirty-1, 
               shared_dirty'=1
	 ; 
         
           invalid >= 1 , 
           dirty =0 , 
           shared = 0 , 
           exclusive =0  , 
           shared_dirty=0
         -> 
	      invalid'=invalid-1,
              dirty'=dirty+1 
	 ; 
          
           invalid >= 1 , dirty >= 1  
         -> 
	      invalid'=invalid-1,
              shared'=shared+exclusive+shared_dirty+dirty,
              exclusive'=0,
              dirty'=0,
              shared_dirty'=1
	 ; 
              invalid >= 1 , shared >= 1  
         -> 
	      invalid'=invalid-1,
              shared'=shared+exclusive+shared_dirty+dirty,
              exclusive'=0,
              dirty'=0,
              shared_dirty'=1
	 ; 
               invalid >= 1 , exclusive >= 1  
         -> 
	      invalid'=invalid-1,
              shared'=shared+exclusive+shared_dirty+dirty,
              exclusive'=0,
              dirty'=0,
              shared_dirty'=1
	 ; 
              invalid >= 1 , shared_dirty >= 1  
         -> 
	      invalid'=invalid-1,
              shared'=shared+exclusive+shared_dirty+dirty,
              exclusive'=0,
              dirty'=0,
              shared_dirty'=1
	 ; 
       
           dirty >=1 
         -> invalid'=invalid+1,
              dirty'=dirty-1 ; 
          
          shared >= 1 
         -> invalid'=invalid+1,
              shared'=shared-1 ; 
          
          shared_dirty >= 1 
         -> invalid'=invalid+1,
              shared_dirty'=shared_dirty-1 ; 
          
          exclusive >=1
        -> invalid'=invalid+1,
             exclusive'=exclusive-1 ; 
init
invalid >=1 , exclusive = 0 , shared = 0 , shared_dirty=0 , dirty = 0

target
dirty >= 2
exclusive >= 2
shared >= 1 , dirty >= 1
exclusive >= 1 , dirty >= 1 
shared_dirty >= 1 , dirty >= 1 
shared >= 1 , exclusive >=  1
shared_dirty >= 1 , exclusive >=  1
