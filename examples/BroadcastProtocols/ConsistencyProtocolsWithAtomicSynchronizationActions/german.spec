#variable definition
vars
  Idle ServeS ServeE GrantS GrantE ex notex Null WaitS WaitE Shared Exclusive



#Rules section
rules

#reqS

Idle >= 1 , Null >= 1 ->
	Idle' = Idle - 1
	,
	ServeS' = ServeS + 1
	,
	Null' = Null - 1
	,
	WaitS' = WaitS + 1;

#inv

ServeS >= 1 , ex >= 1 , Exclusive >= 1 ->
	ServeS' = ServeS - 1
	,
	GrantS' = GrantS + 1
	,
	ex' = ex - 1
	,
	notex' = notex + 1
	,
	Exclusive' = Exclusive - 1
	,
	Null' = Null + 1;

ServeE >= 1 ->
	ServeE' = ServeE - 1
	,
	GrantE' = GrantE + 1
	,
	Null' = Null + Shared + Exclusive + 0
	,
	Shared' = 0
	,
	Exclusive' = 0;

#nonex

ServeS >= 1 , notex >= 1 ->
	ServeS' = ServeS - 1
	,
	GrantS' = GrantS + 1;

#grantS

GrantS >= 1 , WaitS >= 1 ->
	GrantS' = GrantS - 1
	,
	Idle' = Idle + 1
	,
	WaitS' = WaitS - 1
	,
	Shared' = Shared + 1;

#reqE

Idle >= 1 , Null >= 1 ->
	Idle' = Idle - 1
	,
	ServeE' = ServeE + 1
	,
	Null' = Null - 1
	,
	WaitE' = WaitE + 1;

Idle >= 1 , Shared >= 1 ->
	Idle' = Idle - 1
	,
	ServeE' = ServeE + 1
	,
	Shared' = Shared - 1
	,
	WaitE' = WaitE + 1;

#grantE

GrantE >= 1 , WaitE >= 1 ->
	GrantE' = GrantE - 1
	,
	Idle' = Idle + 1
	,
	WaitE' = WaitE - 1
	,
	Exclusive' = Exclusive + 1
	,
	ex' = ex + notex + 0
	,
	notex' = 0;

#init section
init
  Null >= 1 , Idle = 1, ServeS = 0, ServeE = 0, GrantS = 0, GrantE = 0, ex = 0, notex = 1, WaitS = 0, WaitE = 0, Shared = 0, Exclusive = 0


#Goals section
target
  Exclusive >= 2 

invariants

Idle = 1, ServeS = 1, ServeE = 1, GrantS = 1, GrantE = 1
ex = 1, notex = 1
