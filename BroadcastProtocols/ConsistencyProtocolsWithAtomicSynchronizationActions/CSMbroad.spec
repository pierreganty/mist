vars 
Think WaitC UseC Stopped WaitD UseD IdleD BusyD IdleC BusyC Pbusy Noint Int



rules
Think>=1 ->
	  Think'=Think-1,
	  WaitC'=WaitC+1;

WaitC>=1 , IdleC>=1 ->
	  WaitC'=WaitC-1,
	  UseC'=UseC+1,
	  IdleC'=IdleC-1,
	  BusyC'=BusyC+1;

UseC>=1 , BusyC>=1 ->
	  BusyC'=BusyC-1,
	  IdleC'=IdleC+1,
	  UseC'=UseC-1,
	  Think'=Think+1;

UseC>=1 , BusyC>=1 ->
	  BusyC'=BusyC-1,
	  IdleC'=IdleC+1,
	  UseC'=UseC-1,
	  WaitD'=WaitD+1;

WaitD>=1 , IdleD>=1 ->
	  WaitD'=WaitD-1,
	  UseD'=UseD+1,
	  IdleD'=IdleD-1,
	  BusyD'=BusyD+1;

UseD>=1 , BusyD>=1 ->
	  BusyD'=BusyD-1,
	  IdleD'=IdleD+1,
	  UseD'=UseD-1,
	  WaitC'=WaitC+1;

Noint>=1 ->
	  Noint'=Noint-1,
	  Int'=Int+1,
	  Stopped'=UseC+Stopped + 0 ,
	  UseC'=0,
	  IdleC'=Pbusy+IdleC + 0 ,
	  Pbusy'=0;

Int>=1 ->
	  UseC'=Stopped+UseC + 0 ,
	  Stopped'=0,
	  Noint'=Noint+1,
	  Int'=Int-1,
	  Pbusy'=IdleC+Pbusy + 0 ,
	  IdleC'=0;

init
Think >= 1 , IdleC = 1 , IdleD = 1 , Noint = 1 , WaitC = 0 , UseD = 0 , Stopped = 0 , UseC = 0 , WaitD = 0 , Pbusy = 0 , BusyC = 0 , BusyD = 0 , Int = 0

target
UseC >= 2

invariants

Pbusy = 1, IdleC = 1, BusyC = 1
IdleD = 1 BusyD = 1
Noint = 1, Int = 1
