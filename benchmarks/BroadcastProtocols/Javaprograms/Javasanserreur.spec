#expected result: safe
#variable definition
vars
  c2while1 c2getwa c2while2 c2wait1 c2afterwait1 c2return c2putack c2while3 c2wait2 c2afterwait2 c2end p2while1 p2putwa p2while2 p2wait1 p2afterwait1 p2return p2getack p2while3 p2wait2 p2afterwait2 p2end cwhile1 get cwhile2 cwait cafterwait creturn pwhile1 put pwhile2 pwait pafterwait pend unlock lock available notavailable askforack notaskforack needack notneedack isack notisack


#Rules section
rules

#consumer2

c2while1 >= 1 ->
	c2while1' = c2while1 - 1
	,
	c2getwa' = c2getwa + 1;

c2getwa >= 1 , unlock >= 1 ->
	c2getwa' = c2getwa - 1
	,
	c2while2' = c2while2 + 1
	,
	unlock' = unlock - 1
	,
	lock' = lock + 1;

c2while2 >= 1 , isack >= 1 , lock >= 1 ->
	c2while2' = c2while2 - 1
	,
	c2wait1' = c2wait1 + 1
	,
	lock' = lock - 1
	,
	unlock' = unlock + 1;

c2while2 >= 1 , notaskforack >= 1 , lock >= 1 ->
        c2while2' = c2while2 - 1
        ,
        c2wait1' = c2wait1 + 1
        ,
        lock' = lock - 1
        ,
        unlock' = unlock + 1;

c2afterwait1 >= 1 , unlock >= 1 ->
	c2afterwait1' = c2afterwait1 - 1
	,
	c2while2' = c2while2 + 1
	,
	unlock' = unlock - 1
	,
	lock' = lock + 1;

c2while2 >= 1 , notisack >= 1 , askforack >= 1 ->
	c2while2' = c2while2 - 1
	,
	c2return' = c2return + 1
        ,
	askforack' = askforack - 1
	,
	notaskforack' = notaskforack + 1
	,
	needack' = needack + notneedack + 0
	,
	notneedack' = 0
	,
	c2afterwait1' = c2afterwait1 + c2wait1 + 0
	,
	c2wait1' = 0
	,
        c2afterwait2' = c2afterwait2 + c2wait2 + 0
	,
	c2wait2' = 0
	,
	p2afterwait1' = p2afterwait1 + p2wait1 + 0
	,
	p2wait1' = 0
        ,
        p2afterwait2' = p2afterwait2 + p2wait2 + 0
	,
	p2wait2' = 0
	,
	cafterwait' = cafterwait + cwait + 0
	,
	cwait' = 0
	,
	pafterwait' = pafterwait+ pwait + 0
	,
	pwait' = 0;

c2return >= 1 , lock >= 1 ->
	c2return' = c2return - 1
	,
	c2putack' = c2putack + 1
	,
	lock' = lock - 1
	,
	unlock' = unlock +1;

c2putack >= 1 , unlock >= 1 ->
	c2putack' = c2putack - 1
	,
	c2while3' = c2while3 + 1
	,
	unlock' = unlock - 1
	,
	lock' = lock + 1;

c2while3 >= 1 , available >= 1 , lock >= 1 ->
	c2while3' = c2while3 - 1
	,
	c2wait2' = c2wait2 + 1
	,
	lock' = lock - 1
	,
	unlock' = unlock + 1;

c2afterwait2 >= 1 , unlock >= 1 ->
	c2afterwait2' = c2afterwait2 - 1
	,
        c2while3' = c2while3 + 1
	,
	unlock' = unlock - 1
	,
	lock' = lock + 1;

c2while3 >= 1 , notavailable >= 1 ->
	c2while3' = c2while3 - 1
	,
	c2end' = c2end + 1
	,
	notavailable' = notavailable - 1
	,
	available' = available + 1
	,
	isack' = isack + notisack + 0
	,
        notisack' = 0
	,
        c2afterwait1' = c2afterwait1 + c2wait1 + 0
	,
	c2wait1' = 0
        ,
        c2afterwait2' = c2afterwait2 + c2wait2 + 0
	,
	c2wait2' = 0
        ,
        p2afterwait1' = p2afterwait1 + p2wait1 + 0
	,
	p2wait1' = 0
        ,
        p2afterwait2' = p2afterwait2 + p2wait2 + 0
	,
	p2wait2' = 0
        ,
        cafterwait' = cafterwait + cwait + 0
	,
	cwait' = 0
        ,
        pafterwait' = pafterwait+ pwait + 0
	,
	pwait' = 0;


c2end >= 1 , lock >= 1 ->
	c2end' = c2end - 1
	,
	c2while1'= c2while1 + 1
	,
	lock' = lock - 1
	,
	unlock' = unlock + 1;

# producer2

p2while1 >= 1 ->
	p2while1' = p2while1 - 1
	,
	p2putwa' = p2putwa + 1;

p2putwa >= 1 , unlock >= 1 ->
	p2putwa' = p2putwa - 1
	,
	p2while2' = p2while2 + 1
	,
	unlock' = unlock - 1
	,
	lock' =lock + 1;

p2while2 >= 1 , lock >= 1 , available >= 1 ->
	p2while2' = p2while2 - 1
	,
	p2wait1' = p2wait1 + 1
	,
	lock' = lock - 1
	,
	unlock' = unlock + 1;

p2while2 >= 1 , lock >= 1 , needack >= 1 ->
        p2while2' = p2while2 - 1
        ,
        p2wait1' = p2wait1 + 1
        ,
        lock' = lock - 1
        ,
        unlock' = unlock + 1;

p2afterwait1 >= 1 , unlock >= 1 ->
	p2afterwait1' = p2afterwait1 - 1
	,
	p2while2' = p2while2 + 1
	,
	unlock' =unlock - 1
	,
	lock' = lock + 1;

p2while2 >= 1 , notavailable >= 1 , notneedack >= 1 ->
	p2while2' = p2while2 - 1
	,
	p2end' = p2end + 1
	,
	askforack' = askforack + notaskforack + 0
	,
	notaskforack' = 0
	,
        c2afterwait1' = c2afterwait1 + c2wait1 + 0
	,
	c2wait1' = 0
        ,
        c2afterwait2' = c2afterwait2 + c2wait2 + 0
	,
	c2wait2' = 0
        ,
        p2afterwait1' = p2afterwait1 + p2wait1 + 0
	,
	p2wait1' = 0
        ,
        p2afterwait2' = p2afterwait2 + p2wait2 + 0
	,
	p2wait2' = 0
        ,
        cafterwait' = cafterwait + cwait + 0
	,
	cwait' = 0
        ,
        pafterwait' = pafterwait+ pwait + 0
	,
	pwait' = 0;

p2end >= 1 , lock >= 1 ->
	p2end' = p2end - 1
	,
	p2getack' = p2getack + 1
	,
	lock' = lock - 1
	,
	unlock' = unlock + 1;

p2getack >= 1 , unlock >= 1 ->
	p2getack' = p2getack - 1
	,
	p2while3' = p2while3 + 1
	,
	unlock' = unlock - 1
	,
	lock' = lock + 1;

p2while3 >= 1 , lock >= 1 , notisack >= 1 ->
	p2while3' = p2while3 - 1
	,
	p2wait2' = p2wait2 + 1
	,
	lock' = lock - 1
	,
	unlock' = unlock + 1;

p2afterwait2 >= 1 , unlock >= 1 ->
	p2afterwait2' = p2afterwait2 - 1
	,
	p2while3' = p2while3 + 1
	,
	unlock' = unlock - 1
	,
	lock' = lock + 1;

p2while3 >= 1 , isack >= 1 ->
	p2while3' = p2while3 - 1
	,
	p2return' = p2return + 1
        ,
	isack' = isack - 1
	,
	notisack' = notisack + 1
	,
	notavailable' = notavailable + available + 0
	,
	available' = 0
	,
	notneedack' = notneedack + needack + 0
	,
	needack' = 0
	,
        c2afterwait1' = c2afterwait1 + c2wait1 + 0
        ,
	c2wait1' = 0
	,
        c2afterwait2' = c2afterwait2 + c2wait2 + 0
        ,
	c2wait2' = 0
	,
        p2afterwait1' = p2afterwait1 + p2wait1 + 0
        ,
	p2wait1' = 0
	,
        p2afterwait2' = p2afterwait2 + p2wait2 + 0
        ,
	p2wait2' = 0
	,
        cafterwait' = cafterwait + cwait + 0
        ,
	cwait' = 0
	,
        pafterwait' = pafterwait+ pwait + 0
	,
	pwait' = 0;


p2return >= 1 , lock >= 1 ->
	p2return' = p2return - 1
	,
	p2while1' = p2while1 + 1
	,
	lock' = lock - 1
	,
	unlock' = unlock + 1;

#consumer

cwhile1 >= 1 ->
	cwhile1' = cwhile1 - 1
	,
	get' = get + 1;

get >= 1 , unlock >= 1 ->
	get' = get - 1
	,
	cwhile2' = cwhile2 + 1
	,
	unlock' = unlock - 1
	,
	lock' = lock + 1;

cwhile2 >= 1 , lock >= 1 , notavailable >= 1 ->
	cwhile2' = cwhile2 - 1
	,
	cwait' = cwait + 1
	,
	lock' =lock - 1
	,
	unlock' = unlock + 1;

cwhile2 >= 1 , lock >= 1 , askforack >= 1 ->
        cwhile2' = cwhile2 - 1
        ,
        cwait' = cwait + 1
        ,
        lock' =lock - 1
        ,
        unlock' = unlock + 1;

cwhile2 >= 1 , lock >= 1 , isack >= 1 ->
        cwhile2' = cwhile2 - 1
        ,
        cwait' = cwait + 1
        ,
        lock' =lock - 1
        ,
        unlock' = unlock + 1;

cafterwait >= 1 , unlock >= 1 ->
	cafterwait' = cafterwait - 1
	,
	cwhile2' = cwhile2 + 1
	,
	unlock' = unlock - 1
	,
	lock' = lock + 1;

cwhile2 >= 1 , available >= 1 , notaskforack >= 1 , notisack >= 1 ->
	cwhile2' = cwhile2 - 1
	,
	creturn' = creturn + 1
	,
	available' = available - 1
	,
	notavailable' = notavailable + 1
        ,
        c2afterwait1' = c2afterwait1 + c2wait1 + 0
	,
	c2wait1' = 0
        ,
        c2afterwait2' = c2afterwait2 + c2wait2 + 0
	,
	c2wait2' = 0
        ,
        p2afterwait1' = p2afterwait1 + p2wait1 + 0
	,
	p2wait1' = 0
        ,
        p2afterwait2' = p2afterwait2 + p2wait2 + 0
	,
	p2wait2' = 0
        ,
        cafterwait' = cafterwait + cwait + 0
	,
	cwait' = 0
        ,
        pafterwait' = pafterwait+ pwait + 0
	,
	pwait' = 0;


creturn >= 1 , lock >= 1 ->
	creturn' = creturn - 1
	,
	cwhile1' = cwhile1 + 1
	,
	lock' = lock - 1
	,
	unlock' = unlock + 1;

#procdurer

pwhile1 >= 1 ->
	pwhile1' = pwhile1 - 1
	,
	put' = put + 1;

put >= 1 , unlock >= 1 ->
	put' = put - 1
	,
	pwhile2' = pwhile2 + 1
	,
	unlock' = unlock - 1
	,
	lock' = lock + 1;

pwhile2 >= 1 , lock >= 1 , available >= 1 ->
	pwhile2' = pwhile2 - 1
	,
	pwait' = pwait + 1
	,
	lock' = lock - 1
	,
	unlock' = unlock + 1;

pafterwait >= 1 , unlock >= 1 ->
	pafterwait' = pafterwait - 1
	,
	pwhile2' = pwhile2 + 1
	,
	unlock' = unlock - 1
	,
	lock' = lock + 1;

pwhile2 >= 1 , notavailable >= 1 ->
	pwhile2' = pwhile2 - 1
	,
	pend' = pend + 1
	,
	notavailable' = notavailable - 1
	,
	available' = available + 1
	,
        c2afterwait1' = c2afterwait1 + c2wait1 + 0
	,
	c2wait1' = 0
        ,
        c2afterwait2' = c2afterwait2 + c2wait2 + 0
	,
	c2wait2' = 0
        ,
        p2afterwait1' = p2afterwait1 + p2wait1 + 0
	,
	p2wait1' = 0
        ,
        p2afterwait2' = p2afterwait2 + p2wait2 + 0
	,
	p2wait2' = 0
        ,
        cafterwait' = cafterwait + cwait + 0
	,
	cwait' = 0
        ,
        pafterwait' = pafterwait+ pwait + 0
	,
	pwait' = 0;

pend >= 1 , lock >= 1 ->
	pend' = pend - 1
	,
	pwhile1' = pwhile1 + 1
	,
	lock' = lock - 1
	,
	unlock' = unlock + 1;


#init section
init
  c2while1 >= 1 , p2while1 >= 1 , cwhile1 >= 1 , pwhile1 >= 1 , unlock = 1 , notisack = 1 , notaskforack = 1 , notneedack = 1 , notavailable = 1, c2getwa = 0, c2while2 = 0, c2wait1 = 0, c2afterwait1 = 0, c2return = 0, c2putack = 0, c2while3 = 0, c2wait2 = 0, c2afterwait2 = 0, c2end = 0, p2putwa = 0, p2while2 = 0, p2wait1 = 0, p2afterwait1 = 0, p2return = 0, p2getack = 0, p2while3 = 0, p2wait2= 0, p2afterwait2 = 0, p2end = 0,  get = 0, cwhile2 = 0, cwait = 0, cafterwait = 0, creturn = 0, put = 0, pwhile2 = 0, pwait = 0, pafterwait = 0, pend = 0, lock = 0, available = 0, askforack = 0, needack  = 0, isack = 0

#Goals section
target
  notavailable >= 1 , isack >= 1

invariants

unlock = 1, lock =1
available = 1, notavailable = 1
askforack = 1, notaskforack = 1
needack = 1, notneedack = 1
isack = 1, notisack = 1
c2while2 = 1, c2return = 1, c2while3 = 1, c2end = 1, p2while2 = 1, p2return = 1, p2while3 = 1, p2end = 1, cwhile2 = 1, creturn = 1, pwhile2 = 1, pend = 1, unlock = 1