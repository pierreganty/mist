vars
 I Sa Ea Ma Sb Eb Mb

rules

#read_a

I >= 1 ->
	I' = I - 1,
	Sa' = Sa + Ea + Ma + 1,
	Ea' = 0,
	Ma' = 0;

Sb >= 1 ->
	Sb' = Sb - 1,
	Sa' =  Ea + Ma + 1,
        Ea' = 0,
        Ma' = 0;

#read_b

I >= 1 ->
	I' = I - 1,
	Sb' = Sb + Eb + Mb + 1,
	Eb' = 0,
	Mb' = 0;

Sa >= 1 ->
	Sa' = Sa - 1,
	Sb' = Sb + Eb + Mb + 1,
        Eb' = 0,
        Mb' = 0;

#write_a

Sa >= 1 ->
	I' = I + Sa + Ea + Ma + 0,
	Sa' = 0,
	Ea' = 1,
	Ma' = 0;

Sb >= 1 ->
	Sb' = Sb - 1,
	I' = I + Sa + Ea + Ma + 0,
        Sa' = 0,
        Ea' = 1,
        Ma' = 0;

#write_b

Sb >= 1 ->
	I' = I + Sb + Eb + Mb + 0,
	Sb' = 0,
	Eb' = 1,
	Mb' = 0;

Sa >= 1 ->
	Sa' = Sa - 1,
	I' = I + Sb + Eb + Mb + 0,
        Sb' = 0,
        Eb' = 1,
        Mb' = 0;

#write(a)
Ea >= 1 -> 
	Ea' = Ea - 1,
	Ma' = Ma + 1;

#write(b)
Eb >= 1 ->
	Eb' = Eb - 1,
	Mb' = Mb + 1;

init
I >= 1, Sa = 0, Ea = 0, Ma = 0, Sb = 0, Eb = 0, Mb = 0

target
Sa >= 1,Ma >= 1

#no invariants
