#expected result: safe
#variable definition
vars
  i1 i2 lock unlock invalid modified shared owned exclusive


#Rules section
rules

invalid >= 1 , unlock >= 0 ->
	invalid' = invalid - 1
	,
	shared' = shared + exclusive + 1
	,
	exclusive' = 0
	,
	owned' = owned + modified + 0
	,
	modified' = 0;

exclusive >= 1 , unlock >= 1 ->
	exclusive' = exclusive - 1
	,
	modified' = modified + 1;

shared >= 1 , unlock >= 1->
	shared' = shared - 1
	,
	i1' = i1 + 1
	,
	unlock' = unlock - 1
	,
	lock' = lock + 1;

owned >= 1 , unlock >= 1 ->
	owned' = owned - 1
	,
	i1' = i1 + 1
        ,
        unlock' = unlock - 1
        ,
        lock' = lock + 1;

i1 >= 1 , lock >= 1 ->
	i1' = i1 - 1
	,
	invalid' = invalid + owned + modified + exclusive + shared + 0
	,
	shared' = 0
	,
	owned' = 0
	,
	exclusive' = 1
	,
	modified' = 0
	,
	lock' = lock - 1
	,
	unlock' = unlock + 1;

invalid >= 1 , unlock >= 1 ->
	invalid' = invalid - 1
	,
	i2' = i2 + 1
	,
	unlock' = unlock - 1
	,
	lock' = lock + 1;

i2 >= 1 , lock >=1 ->
	i2' = i2 -1
	,
	invalid' = invalid + owned + modified + exclusive + shared + 0
        ,
        shared' = 0
        ,
        owned' = 0
        ,
        exclusive' = 1
        ,
        modified' = 0
	,
	lock' = lock - 1
	,
	unlock' = unlock + 1;

modified >= 1 , unlock >= 1->
	invalid' = invalid + 1
	,
	modified' = modified - 1;

shared >= 1 , unlock >= 1 ->
	shared' = shared - 1
	,
	invalid' = invalid + 1;

exclusive >= 1 , unlock >= 1 ->
	exclusive' = exclusive - 1
	,
	invalid' = invalid + 1;

owned >= 1 , unlock >= 1 ->
	owned' = owned - 1
	,
	invalid' = invalid + 1;



#init section
init
  invalid >= 1, unlock = 1, i1 = 0, i2 = 0, lock = 0, modified = 0, shared = 0,
owned = 0, exclusive = 0

#Goals section
target
  exclusive >= 2

invariants
lock = 1, unlock = 1