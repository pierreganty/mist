#variable definition
vars
  whileinc incx xi notifyincx endincx incy yi notifyincy endincy endinc whiledec decx while1 wait1 afterwait1 xd enddecx1 enddecx2 decy while2 wait2 afterwait2 yd enddecy1 enddecy2 enddec xpos notxpos ypos notypos lock unlock



#Rules section
rules

#inc

whileinc >= 1 ->
	whileinc' = whileinc - 1
	,
	incx' = incx + 1;

incx >= 1 , unlock >= 1 ->
	incx' = incx - 1
	,
	xi' = xi + 1
	,
	unlock' = unlock - 1
	,
	lock' = lock + 1;

xi >= 1 ->
	xi' = xi - 1
	,
	notifyincx' = notifyincx + 1
	,
	xpos' = xpos + notxpos + 0
	,
	notxpos' = 0;

notifyincx >= 1 ->
	notifyincx' = notifyincx - 1
	,
	endincx' = endincx + 1
	,
	afterwait1' = afterwait1 + wait1 + 0
	,
	wait1' = 0
	,
	afterwait2' = afterwait2 + wait2 + 0
	,
	wait2' = 0;

endincx >= 1 , lock >= 1 ->
	endincx' = endincx - 1
	,
	incy' = incy + 1
	,
	lock' = lock - 1
	,
	unlock' = unlock + 1;

incy >= 1 , unlock >= 1 ->
	incy' = incy - 1
	,
	yi' = yi + 1
	,
	unlock' = unlock - 1
	,
	lock' = lock + 1;

yi >= 1 ->
	yi' = yi - 1
	,
	notifyincy' = notifyincy + 1
	,
	ypos' = ypos + notypos + 0
	,
	notypos' = 0;

notifyincy >= 1 ->
	notifyincy' = notifyincy - 1
	,
	endincy' = endincy + 1
	,
	afterwait1' = afterwait1 + wait1 + 0
        ,
        wait1' = 0
        ,
        afterwait2' = afterwait2 + wait2 + 0
        ,
        wait2' = 0;

endincy >= 1 , lock >= 1 ->
	endincy' = endincy - 1
	,
	endinc' = endinc + 1
	, 
	lock' = lock - 1
	,
	unlock' = unlock + 1;

endinc >= 1 ->
	endinc' = endinc - 1
	,
	whileinc' = whileinc + 1;
#dec

whiledec >= 1 ->
	whiledec' = whiledec - 1
	,
	decx' = decx + 1;

decx >= 1 , unlock >= 1 ->
	decx' = decx - 1
	,
	while1' = while1 + 1
	,
	unlock' = unlock - 1
	,
	lock' = lock + 1;

while1 >= 1 , lock >= 1 , notxpos >= 1 ->
	while1' = while1 - 1
	,
	wait1' = wait1 + 1
	,
	lock' = lock - 1
	,
	unlock' = unlock + 1;

afterwait1 >= 1 , unlock >= 1 ->
	afterwait1' = afterwait1 - 1
	,
	while1' = while1 + 1
	,
	unlock' = unlock - 1
	,
	lock' = lock + 1;

while1 >= 1 , xpos >= 1 ->
	while1' = while1 - 1
	,
	xd' = xd + 1;

xd >=1 ->
	xd' = xd - 1
	,
	enddecx1' = enddecx1 + 1
	,
	xpos' = xpos + notxpos + 0
	,
	notxpos' = 0;

xd >=1 ->
        xd' = xd - 1
        ,
        enddecx2' = enddecx2 + 1
        ,
        notxpos' = xpos + notxpos + 0
        ,
        xpos' = 0;

enddecx1 >= 1 , lock >= 1 ->
	enddecx1' = enddecx1 - 1
	,	
	decy' = decy + 1
	,
	lock' = lock - 1
	,
	unlock' = unlock + 1;

enddecx2 >= 1 , lock >= 1 ->
        enddecx2' = enddecx2 - 1
        ,
        decy' = decy + 1
        ,
        lock' = lock - 1
        ,
        unlock' = unlock + 1;

decy >= 1 , unlock >= 1 ->
	decy' = decy - 1
	,
	while2' = while2 + 1
	,
	unlock' = unlock - 1
	,
	lock' = lock + 1;

while2 >= 1 , notypos >= 1 , lock >= 1 ->
	while2' = while2 - 1
	,
	wait2' = wait2 + 1
	,
	lock' = lock - 1
	,
	unlock' = unlock + 1;

afterwait2 >= 1 , unlock >= 1 ->
	afterwait2' = afterwait2 - 1
	,
	while2' = while2 + 1
	,
	unlock' = unlock - 1
	,
	lock' = lock + 1;

while2 >= 1 , ypos >= 1 -> 
	while2' = while2 - 1
	,
	yd' = yd + 1;

yd >= 1 ->
	yd' = yd - 1
	,
	enddecy1' = enddecy1 + 1
	,
	ypos' = ypos + notypos + 0
	,
	notypos' = 0; 

yd >= 1 ->
        yd' = yd - 1
        ,
        enddecy2' = enddecy2 + 1
        ,
        notypos' = ypos + notypos + 0
        ,
        ypos' = 0; 

enddecy1 >= 1 , lock >= 1 ->
	enddecy1' =enddecy1 - 1
	,
	enddec' = enddec + 1
	,
	lock' = lock - 1
	,
	unlock' = unlock + 1;

enddecy2 >= 1 , lock >= 1 ->
        enddecy2' =enddecy2 - 1
        ,
        enddec' = enddec + 1
        ,
        lock' = lock - 1
        ,
        unlock' = unlock + 1;

enddec >= 1 ->
	enddec' = enddec - 1
	,
	whiledec' = whiledec + 1;


#init section
init
  whileinc >= 1, whiledec >= 1, unlock = 1 , notxpos = 1 , notypos = 1, incx = 0, xi = 0, notifyincx = 0, endincx = 0, incy = 0, yi = 0, notifyincy = 0, endincy = 0, endinc = 0, decx = 0, while1 = 0, wait1 = 0, afterwait1 = 0, xd = 0, enddecx1 = 0, enddecx2 = 0, decy = 0, while2 = 0, wait2 = 0, afterwait2 = 0, yd = 0, enddecy1 = 0, enddecy2 = 0, enddec = 0, xpos = 0, ypos =0, lock =0

#Goals section
target
  decy >= 1 , incy >= 1 

invariants
xpos = 1, notxpos = 1
ypos = 1, notypos = 1
lock = 1, unlock = 1
xi = 1, notifyincx = 1, endincx = 1,  yi = 1, notifyincy = 1, endincy = 1, while1 = 1, xd = 1, enddecx1 =1, enddecx2 = 1, while2 = 1, yd = 1, enddecy1 = 1, enddecy2 = 1, unlock = 1