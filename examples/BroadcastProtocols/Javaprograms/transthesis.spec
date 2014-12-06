#expected result: safe
### example from the book Concurrent Programming in Java, Second Edition. -- Page 257

vars
unlock lock notlockrefa lockrefa lockrefnull notlockrefnull b1 b2 b5 b6 d1 d2 d5 d6 w1 w2 w5 w6 j1 j2 j5 j6 o1 a1 a2 notifya a3 c1 c2 notifyc c3 ob1 ob2 ob3 ob5 ob6 ob7 ob10 od1 od2 od3 od5 od6 od7 od10 ow1 ow2 ow3 ow5 ow6 ow7 ow10 oj1 oj3 oj4 oo1 oc1 oc2 oc3 onotifyc oc4 oa1 oa2 oa3 onotifya oa4 choiceA choiceO waitb afterwaitb waitd afterwaitd waitw afterwaitw waitj afterwaitj owaitb1 oafterwaitb1 owaitb2 oafterwaitb2  owaitd1 oafterwaitd1 owaitd2 oafterwaitd2 owaitw1 oafterwaitw1 owaitw2 oafterwaitw2 owaitj oafterwaitj



rules
choiceA >= 1, unlock >= 1 ->
	choiceA' = choiceA - 1,
	b1' = b1 + 1,
	unlock' = unlock - 1,
	lock' = lock + 1;

choiceA >= 1, unlock >= 1 ->
        choiceA' = choiceA - 1,
        d1' = d1 + 1,
        unlock' = unlock - 1,
        lock' = lock + 1;

choiceA >= 1, unlock >= 1 ->
        choiceA' = choiceA - 1,
        w1' = w1 + 1,
        unlock' = unlock - 1,
        lock' = lock + 1;

choiceA >= 1, unlock >= 1 ->
        choiceA' = choiceA - 1,
        j1' = j1 + 1,
        unlock' = unlock - 1,
        lock' = lock + 1;

choiceA >= 1, unlock >= 1 ->
        choiceA' = choiceA - 1,
        a1' = a1 + 1,
        unlock' = unlock - 1,
        lock' = lock + 1;

choiceA >= 1, unlock >= 1 ->
        choiceA' = choiceA - 1,
        c1' = c1 + 1,
        unlock' = unlock - 1,
        lock' = lock + 1;

choiceA >= 1, unlock >= 1 ->
        choiceA' = choiceA - 1,
        o1' = o1 + 1,
        unlock' = unlock - 1,
        lock' = lock + 1;


#balance

b1 >= 1 , notlockrefa >= 1 ->
	b1' = b1 - 1 ,
	b2' = b2 + 1;

b1 >= 1 , lockrefa >= 1 ->
	b1' = b1 - 1 ,
	b6' = b6 + 1;

b2 >= 1 , notlockrefnull >= 1 , lock >= 1 ->
	b2' = b2 - 1 ,
	waitb' = waitb + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock + 1;

afterwaitb >= 1 , unlock >= 1 ->
	afterwaitb' = afterwaitb - 1 ,
	b2' = b2 + 1,
	unlock' = unlock - 1 ,
	lock' = lock + 1;

b2 >= 1 , lockrefnull >= 1 ->
	b2' = b2 - 1 ,
	b5' = b5 + 1;

b5 >= 1 ->
	b5' = b5 - 1 ,
	b6' = b6 + 1 ,
	lockrefa' = lockrefa + notlockrefa + 0 ,
	notlockrefa' = 0 ,
	notlockrefnull' = notlockrefnull + lockrefnull + 0 ,
	lockrefnull' = 0;

b6 >= 1 , lock >= 1 ->
	b6' = b6 - 1 ,
	choiceA' = choiceA + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock + 1;

#deposit

d1 >= 1 , notlockrefa >= 1 ->
	d1' = d1 - 1 ,
	d2' = d2 + 1;

d1 >= 1 , lockrefa >= 1 ->
	d1' = d1 - 1 ,
	d6' = d6 + 1;

d2 >= 1 , notlockrefnull >= 1 , lock >= 1 ->
	d2' = d2 - 1 ,
	waitd' = waitd + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock + 1;

afterwaitd >= 1 , unlock >= 1 ->
	afterwaitd' = afterwaitd - 1 ,
	d2' = d2 + 1,
	unlock' = unlock - 1 ,
	lock' = lock + 1;

d2 >= 1 , lockrefnull >= 1 ->
	d2' = d2 - 1 ,
	d5' = d5 + 1;

d5 >= 1 ->
	d5' = d5 - 1 ,
	d6' = d6 + 1 ,
	lockrefa' = lockrefa + notlockrefa + 0 ,
	notlockrefa' = 0 ,
	notlockrefnull' = notlockrefnull + lockrefnull + 0 ,
	lockrefnull' = 0;

d6 >= 1 , lock >= 1 ->
	d6' = d6 - 1 ,
	choiceA' = choiceA + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock + 1;


#withdraw

w1 >= 1 , notlockrefa >= 1 ->
	w1' = w1 - 1 ,
	w2' = w2 + 1;

w1 >= 1 , lockrefa >= 1 ->
	w1' = w1 - 1 ,
	w6' = w6 + 1;

w2 >= 1 , notlockrefnull >= 1 , lock >= 1 ->
	w2' = w2 - 1 ,
	waitw' = waitw + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock + 1;

afterwaitw >= 1 , unlock >= 1 ->
	afterwaitw' = afterwaitw - 1 ,
	w2' = w2 + 1,
	unlock' = unlock - 1 ,
	lock' = lock + 1;

w2 >= 1 , lockrefnull >= 1 ->
	w2' = w2 - 1 ,
	w5' = w5 + 1;

w5 >= 1 ->
	w5' = w5 - 1 ,
	w6' = w6 + 1 ,
	lockrefa' = lockrefa + notlockrefa + 0 ,
	notlockrefa' = 0 ,
	notlockrefnull' = notlockrefnull + lockrefnull + 0 ,
	lockrefnull' = 0;

w6 >= 1 , lock >= 1 ->
	w6' = w6 - 1 ,
	choiceA' = choiceA + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock + 1;

#join

j1 >= 1 , notlockrefa >= 1 ->
	j1' = j1 - 1 ,
	j2' = j2 + 1;

j1 >= 1 , lockrefa >= 1 ->
	j1' = j1 - 1 ,
	j6' = j6 + 1;

j2 >= 1 , notlockrefnull >= 1 , lock >= 1 ->
	j2' = j2 - 1 ,
	waitj' = waitj + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock + 1;

afterwaitj >= 1 , unlock >= 1 ->
	afterwaitj' = afterwaitj - 1 ,
	j2' = j2 + 1,
	unlock' = unlock - 1 ,
	lock' = lock + 1;

j2 >= 1 , lockrefnull >= 1 ->
	j2' = j2 - 1 ,
	j5' = j5 + 1;

j5 >= 1 ->
	j5' = j5 - 1 ,
	j6' = j6 + 1 ,
	lockrefa' = lockrefa + notlockrefa + 0 ,
	notlockrefa' = 0 ,
	notlockrefnull' = notlockrefnull + lockrefnull + 0 ,
	lockrefnull' = 0;

j6 >= 1 , lock >= 1 ->
	j6' = j6 - 1 ,
	choiceA' = choiceA + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock + 1;

o1 >= 1 , lock >= 1 ->
	o1' = o1 - 1 ,
	choiceA' = choiceA + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock  + 1;

#abort

a1 >= 1 , lockrefa >= 1 ->
	a1' = a1 - 1 ,
	a2' = a2 + 1;

a1 >= 1 , notlockrefa >= 1 ->
	a1' = a1 - 1 ,
	a3' = a3 + 1;


a2 >= 1 ->
	a2' = a2 - 1 ,
	notifya' = notifya + 1 ,
	notlockrefa' = notlockrefa + lockrefa + 0 ,
	lockrefa' = 0 ,
	lockrefnull' = lockrefnull + notlockrefnull + 0 ,
	notlockrefnull' = 0;


notifya >= 1 ->
	notifya' = notifya - 1 ,
	a3' = a3 + 1 ,
	oafterwaitb1' = oafterwaitb1 + owaitb1 + 0 ,
	owaitb1' = 0 ,
	oafterwaitb2' = oafterwaitb2 + owaitb2 + 0 ,
	owaitb2' = 0 ,
	oafterwaitd1' = oafterwaitd1 + owaitd1 + 0 ,
	owaitd1' = 0 ,
	oafterwaitd2' = oafterwaitd2 + owaitd2 + 0 ,
	owaitd2' = 0 ,
	oafterwaitw1' = oafterwaitw1 + owaitw1 + 0 ,
	owaitw1' = 0 ,
	oafterwaitw2' = oafterwaitw2 + owaitw2 + 0 ,
	owaitw2' = 0 ,
	oafterwaitj' = oafterwaitj + owaitj + 0 ,
	owaitj' = 0 ,
	afterwaitb' = afterwaitb + waitb + 0 ,
	waitb' = 0 ,
	afterwaitd' = afterwaitd + waitd + 0 ,
	waitd' = 0 ,
	afterwaitw' = afterwaitw + waitw + 0 ,
	waitw' = 0 ,
	afterwaitj' = afterwaitj + waitj + 0 ,
	waitj' = 0;




a3 >= 1 , lock >= 1 ->
	a3' = a3 - 1 ,
	choiceA' = choiceA + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock + 1;

c1 >= 1 , lockrefa >= 1 ->
	c1' = c1 - 1 ,
	c2' = c2 + 1;

c1 >= 1 , notlockrefa >= 1 ->
	c1' = c1 - 1 ,
	c3' = c3 + 1;


c2 >= 1 ->
	c2' = c2 - 1 ,
	notifyc' = notifyc + 1 ,
	notlockrefa' = notlockrefa + lockrefa + 0 ,
	lockrefa' = 0 ,
	lockrefnull' = lockrefnull + notlockrefnull + 0 ,
	notlockrefnull' = 0;


notifyc >= 1 ->
	notifyc' = notifyc - 1 ,
	c3' = c3 + 1 ,
	oafterwaitb1' = oafterwaitb1 + owaitb1 + 0 ,
	owaitb1' = 0 ,
	oafterwaitb2' = oafterwaitb2 + owaitb2 + 0 ,
	owaitb2' = 0 ,
	oafterwaitd1' = oafterwaitd1 + owaitd1 + 0 ,
	owaitd1' = 0 ,
	oafterwaitd2' = oafterwaitd2 + owaitd2 + 0 ,
	owaitd2' = 0 ,
	oafterwaitw1' = oafterwaitw1 + owaitw1 + 0 ,
	owaitw1' = 0 ,
	oafterwaitw2' = oafterwaitw2 + owaitw2 + 0 ,
	owaitw2' = 0 ,
	oafterwaitj' = oafterwaitj + owaitj + 0 ,
	owaitj' = 0 ,
	afterwaitb' = afterwaitb + waitb + 0 ,
	waitb' = 0 ,
	afterwaitd' = afterwaitd + waitd + 0 ,
	waitd' = 0 ,
	afterwaitw' = afterwaitw + waitw + 0 ,
	waitw' = 0 ,
	afterwaitj' = afterwaitj + waitj + 0 ,
	waitj' = 0;



c3 >= 1 , lock >= 1 ->
	c3' = c3 - 1 ,
	choiceA' = choiceA + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock + 1;


#process O

choiceO >= 1 , unlock >= 1 ->
	choiceO' = choiceO - 1 ,
	ob1' = ob1 + 1 ,
	unlock' = unlock - 1 ,
	lock' = lock + 1;


choiceO >= 1 , unlock >= 1 ->
	choiceO' = choiceO - 1 ,
	od1' = od1 + 1 ,
	unlock' = unlock - 1 ,
	lock' = lock + 1;


choiceO >= 1 , unlock >= 1 ->
	choiceO' = choiceO - 1 ,
	ow1' = ow1 + 1 ,
	unlock' = unlock - 1 ,
	lock' = lock + 1;

choiceO >= 1 , unlock >= 1 ->
	choiceO' = choiceO - 1 ,
	oj1' = oj1 + 1 ,
	unlock' = unlock - 1 ,
	lock' = lock + 1;

choiceO >= 1 , unlock >= 1 ->
	choiceO' = choiceO - 1 ,
	oo1' = oo1 + 1 ,
	unlock' = unlock - 1 ,
	lock' = lock + 1;

choiceO >= 1 , unlock >= 1 ->
	choiceO' = choiceO - 1 ,
	oc1' = oc1 + 1 ,
	unlock' = unlock - 1 ,
	lock' = lock + 1;

choiceO >= 1 , unlock >= 1 ->
	choiceO' = choiceO - 1 ,
	oa1' = oa1 + 1 ,
	unlock' = unlock - 1 ,
	lock' = lock + 1;

ob1 >= 1 , lockrefa >= 1 ->
	ob1' = ob1 - 1,
	ob2' = ob2 + 1;

ob1 >= 1 , lockrefnull >= 1 ->
	ob1' = ob1 - 1,
	ob2' = ob2 + 1;

ob1 >= 1 , notlockrefnull >= 1 , notlockrefa >= 1->
	ob1' = ob1 - 1,
	ob5' = ob5 + 1;

ob2 >= 1 , notlockrefnull >= 1 , lock >= 1 ->
	ob2' = ob2 - 1 ,
	owaitb1' = owaitb1 + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock + 1;

oafterwaitb1 >= 1 , unlock >= 1 ->
	oafterwaitb1' = oafterwaitb1 - 1 ,
	ob1' = ob1 + 1 ,
	unlock' = unlock - 1 ,
	lock' = lock + 1;

ob2 >= 1 , lockrefnull >= 1 ->
	ob2' = ob2 - 1 ,
	ob3' = ob3 + 1;

ob3 >= 1 ->
	ob3' = ob3 - 1 ,
	ob10' = ob10 + 1 ,
	notlockrefa' = notlockrefa + lockrefa + 0 ,
	lockrefa' = 0 ,
	notlockrefnull' = notlockrefnull + lockrefnull + 0 ,
	lockrefnull' = 0;

ob5 >= 1 ->
	ob5' = ob5 - 1 ,
	ob6' = ob6 + 1;

ob5 >= 1 ->
	ob5' = ob5 - 1 ,
	ob10' = ob10 + 1;

ob6 >= 1 , notlockrefnull >= 1 , lock >= 1 ->
	ob6' = ob6 - 1 ,
	owaitb2' = owaitb2 + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock + 1;

oafterwaitb2 >= 1 , unlock >= 1 ->
	oafterwaitb2' = oafterwaitb2 - 1 ,
	ob6' = ob6 + 1 ,
	unlock' = unlock - 1 ,
	lock' = lock + 1;

ob6 >= 1 , lockrefnull >= 1 ->
	ob6' = ob6 - 1 ,
	ob7' = ob7 + 1;

ob7 >= 1 ->
	ob7' = ob7 - 1 ,
	ob10' = ob10 + 1 ,
	notlockrefa' = notlockrefa + lockrefa + 0 ,
	lockrefa' = 0 ,
	notlockrefnull' = notlockrefnull + lockrefnull + 0 ,
	lockrefnull' = 0;

ob10 >= 1 , lock >= 1 ->
	ob10' = ob10 - 1 ,
	choiceO' = choiceO + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock + 1;


od1 >= 1 , lockrefa >= 1 ->
	od1' = od1 - 1,
	od2' = od2 + 1;

od1 >= 1 , lockrefnull >= 1 ->
	od1' = od1 - 1,
	od2' = od2 + 1;

od1 >= 1 , notlockrefnull >= 1 , notlockrefa >= 1->
	od1' = od1 - 1,
	od5' = od5 + 1;

od2 >= 1 , notlockrefnull >= 1 , lock >= 1 ->
	od2' = od2 - 1 ,
	owaitd1' = owaitd1 + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock + 1;

oafterwaitb1 >= 1 , unlock >= 1 ->
	oafterwaitd1' = oafterwaitd1 - 1 ,
	od1' = od1 + 1 ,
	unlock' = unlock - 1 ,
	lock' = lock + 1;

od2 >= 1 , lockrefnull >= 1 ->
	od2' = od2 - 1 ,
	od3' = od3 + 1;

od3 >= 1 ->
	od3' = od3 - 1 ,
	od10' = od10 + 1 ,
	notlockrefa' = notlockrefa + lockrefa + 0 ,
	lockrefa' = 0 ,
	notlockrefnull' = notlockrefnull + lockrefnull + 0 ,
	lockrefnull' = 0;

od5 >= 1 ->
	od5' = od5 - 1 ,
	od6' = od6 + 1;

od5 >= 1 ->
	od5' = od5 - 1 ,
	od10' = od10 + 1;

od6 >= 1 , notlockrefnull >= 1 , lock >= 1 ->
	od6' = od6 - 1 ,
	owaitd2' = owaitd2 + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock + 1;

oafterwaitd2 >= 1 , unlock >= 1 ->
	oafterwaitd2' = oafterwaitd2 - 1 ,
	od6' = od6 + 1 ,
	unlock' = unlock - 1 ,
	lock' = lock + 1;

od6 >= 1 , lockrefnull >= 1 ->
	od6' = od6 - 1 ,
	od7' = od7 + 1;

od7 >= 1 ->
	od7' = od7 - 1 ,
	od10' = od10 + 1 ,
	notlockrefa' = notlockrefa + lockrefa + 0 ,
	lockrefa' = 0 ,
	notlockrefnull' = notlockrefnull + lockrefnull + 0 ,
	lockrefnull' = 0;


od10 >= 1 , lock >= 1 ->
	od10' = od10 - 1 ,
	choiceO' = choiceO + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock + 1;


ow1 >= 1 , lockrefa >= 1 ->
	ow1' = ow1 - 1,
	ow2' = ow2 + 1;

ow1 >= 1 , lockrefnull >= 1 ->
	ow1' = ow1 - 1,
	ow2' = ow2 + 1;

ow1 >= 1 , notlockrefnull >= 1 , notlockrefa >= 1->
	ow1' = ow1 - 1,
	ow5' = ow5 + 1;

ow2 >= 1 , notlockrefnull >= 1 , lock >= 1 ->
	ow2' = ow2 - 1 ,
	owaitw1' = owaitw1 + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock + 1;

oafterwaitb1 >= 1 , unlock >= 1 ->
	oafterwaitw1' = oafterwaitw1 - 1 ,
	ow1' = ow1 + 1 ,
	unlock' = unlock - 1 ,
	lock' = lock + 1;

ow2 >= 1 , lockrefnull >= 1 ->
	ow2' = ow2 - 1 ,
	ow3' = ow3 + 1;

ow3 >= 1 ->
	ow3' = ow3 - 1 ,
	ow10' = ow10 + 1 ,
	notlockrefa' = notlockrefa + lockrefa + 0 ,
	lockrefa' = 0 ,
	notlockrefnull' = notlockrefnull + lockrefnull + 0 ,
	lockrefnull' = 0;

ow5 >= 1 ->
	ow5' = ow5 - 1 ,
	ow6' = ow6 + 1;

ow5 >= 1 ->
	ow5' = ow5 - 1 ,
	ow10' = ow10 + 1;

ow6 >= 1 , notlockrefnull >= 1 , lock >= 1 ->
	ow6' = ow6 - 1 ,
	owaitw2' = owaitw2 + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock + 1;

oafterwaitd2 >= 1 , unlock >= 1 ->
	oafterwaitw2' = oafterwaitw2 - 1 ,
	ow6' = ow6 + 1 ,
	unlock' = unlock - 1 ,
	lock' = lock + 1;

ow6 >= 1 , lockrefnull >= 1 ->
	ow6' = ow6 - 1 ,
	ow7' = ow7 + 1;

ow7 >= 1 ->
	ow7' = ow7 - 1 ,
	ow10' = ow10 + 1 ,
	notlockrefa' = notlockrefa + lockrefa + 0 ,
	lockrefa' = 0 ,
	notlockrefnull' = notlockrefnull + lockrefnull + 0 ,
	lockrefnull' = 0;


ow10 >= 1 , lock >= 1 ->
	ow10' = ow10 - 1 ,
	choiceO' = choiceO + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock + 1;

oj1 >= 1 , notlockrefnull >= 1 , lock >= 1 ->
	oj1' = oj1 - 1 ,
	owaitj' = owaitj + 1,
	lock' = lock - 1 ,
	unlock' = unlock + 1;


oafterwaitj >= 1 , unlock >= 1 ->
	oafterwaitj' = oafterwaitj - 1 ,
	oj1' = oj1 + 1 ,
	unlock' = unlock - 1 ,
	lock' = lock + 1;


oj1 >= 1 , lockrefnull >= 1 ->
	oj1' = oj1 - 1 ,
	oj3' = oj3 + 1;


oj3 >= 1 ->
	oj3' = oj3 - 1 ,
	oj4' = oj4 + 1 ,
	notlockrefa' = notlockrefa + lockrefa + 0 ,
	lockrefa' = 0 ,
	notlockrefnull' = notlockrefnull + lockrefnull + 0 ,
	lockrefnull' = 0;

oj4 >= 1 , lock >= 1 ->
	oj4' = oj4 - 1 ,
	choiceO'  = choiceO + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock + 1;

oo1 >= 1 , lock >= 1 ->
	oo1' = oo1 - 1 ,
	choiceO' = choiceO + 1 ,
	lock' = lock - 1 ,
	unlock' = unlock + 1;

oc1 >= 1 , notlockrefa >= 1 , notlockrefnull >= 1 ->
	oc1' = oc1 - 1 ,
	oc2' = oc2 + 1;

oc1 >= 1 , lockrefa >= 1 ->
	oc1' = oc1 - 1 ,
	oc4' = oc4 + 1;

oc1 >= 1 , lockrefnull >= 1 ->
	oc1' = oc1 - 1 ,
	oc4' = oc4 + 1;

oc2 >= 1 ->
	oc2' = oc2 - 1 ,
	oc3' = oc3 + 1;

oc2 >= 1 ->
	oc2' = oc2 - 1 ,
	oc4' = oc4 + 1;

oc3 >= 1 ->
	oc3' = oc3 - 1 ,
	onotifyc' = onotifyc + 1 ,
	lockrefnull' = lockrefnull + notlockrefnull + 0 ,
	notlockrefnull' = 0 ,
	notlockrefa' = notlockrefa + lockrefa + 0 ,
	lockrefa' = 0;


onotifyc >= 1 ->
	onotifyc' = onotifyc - 1 ,
	oc4' = oc4 + 1 ,
	oafterwaitb1' = oafterwaitb1 + owaitb1 + 0 ,
	owaitb1' = 0 ,
	oafterwaitb2' = oafterwaitb2 + owaitb2 + 0 ,
	owaitb2' = 0 ,
	oafterwaitd1' = oafterwaitd1 + owaitd1 + 0 ,
	owaitd1' = 0 ,
	oafterwaitd2' = oafterwaitd2 + owaitd2 + 0 ,
	owaitd2' = 0 ,
	oafterwaitw1' = oafterwaitw1 + owaitw1 + 0 ,
	owaitw1' = 0 ,
	oafterwaitw2' = oafterwaitw2 + owaitw2 + 0 ,
	owaitw2' = 0 ,
	oafterwaitj' = oafterwaitj + owaitj + 0 ,
	owaitj' = 0 ,
	afterwaitb' = afterwaitb + waitb + 0 ,
	waitb' = 0 ,
	afterwaitd' = afterwaitd + waitd + 0 ,
	waitd' = 0 ,
	afterwaitw' = afterwaitw + waitw + 0 ,
	waitw' = 0 ,
	afterwaitj' = afterwaitj + waitj + 0 ,
	waitj' = 0;




oc4 >= 1 , lock>= 1 ->
	oc4' = oc4 - 1 ,
	choiceO' = choiceO + 1,
	lock' = lock - 1 ,
	unlock' = unlock + 1;


oa1 >= 1 , notlockrefa >= 1 , notlockrefnull >= 1 ->
	oa1' = oa1 - 1 ,
	oa2' = oa2 + 1;

oa1 >= 1 , lockrefa >= 1 ->
	oa1' = oa1 - 1 ,
	oa4' = oa4 + 1;

oa1 >= 1 , lockrefnull >= 1 ->
	oa1' = oa1 - 1 ,
	oa4' = oa4 + 1;

oa2 >= 1 ->
	oa2' = oa2 - 1 ,
	oa3' = oa3 + 1;

oa2 >= 1 ->
	oa2' = oa2 - 1 ,
	oa4' = oa4 + 1;

oa3 >= 1 ->
	oa3' = oa3 - 1 ,
	onotifya' = onotifya + 1 ,
	lockrefnull' = lockrefnull + notlockrefnull + 0 ,
	notlockrefnull' = 0 ,
	notlockrefa' = notlockrefa + lockrefa + 0 ,
	lockrefa' = 0;


onotifya >= 1 ->
	onotifya' = onotifya - 1 ,
	oa4' = oa4 + 1 ,
	oafterwaitb1' = oafterwaitb1 + owaitb1 + 0 ,
	owaitb1' = 0 ,
	oafterwaitb2' = oafterwaitb2 + owaitb2 + 0 ,
	owaitb2' = 0 ,
	oafterwaitd1' = oafterwaitd1 + owaitd1 + 0 ,
	owaitd1' = 0 ,
	oafterwaitd2' = oafterwaitd2 + owaitd2 + 0 ,
	owaitd2' = 0 ,
	oafterwaitw1' = oafterwaitw1 + owaitw1 + 0 ,
	owaitw1' = 0 ,
	oafterwaitw2' = oafterwaitw2 + owaitw2 + 0 ,
	owaitw2' = 0 ,
	oafterwaitj' = oafterwaitj + owaitj + 0 ,
	owaitj' = 0 ,
	afterwaitb' = afterwaitb + waitb + 0 ,
	waitb' = 0 ,
	afterwaitd' = afterwaitd + waitd + 0 ,
	waitd' = 0 ,
	afterwaitw' = afterwaitw + waitw + 0 ,
	waitw' = 0 ,
	afterwaitj' = afterwaitj + waitj + 0 ,
	waitj' = 0;


oa4 >= 1 , lock>= 1 ->
	oa4' = oa4 - 1 ,
	choiceO' = choiceO + 1,
	lock' = lock - 1 ,
	unlock' = unlock + 1;


init
unlock = 1 , lock = 0 , notlockrefa = 1 , lockrefa = 0 , notlockrefnull = 0 , lockrefnull = 1 , b1 =0 , b2 = 0 , b5 = 0 , b6= 0 , d1 = 0 , d2 = 0 , d5 =0 , d6 = 0 , w1 = 0 , w2 = 0 , w5 = 0 , w6 = 0 , j1 = 0 , j2 = 0 , j5 = 0 , j6 = 0 , o1 = 0 , a1 = 0 , a2 = 0 , notifya = 0 , a3 =0 , c1 = 0 , c2 = 0 , notifyc = 0 , c3 = 0 , ob1 = 0 , ob2 = 0 , ob3 = 0 , ob5 = 0 , ob6 = 0 , ob7 = 0 , ob10 = 0 , od1 = 0 , od2 = 0 , od3 = 0 , od5 = 0 , od6 = 0 , od7 = 0 , od10 = 0 , ow1 = 0 , ow2 = 0 , ow3 = 0 , ow5 = 0 , ow6 = 0 , ow7 = 0 , ow10 = 0 , oj1 = 0 , oj3 = 0 , oj4 = 0 , oo1 = 0 , oc1 = 0 , oc2 = 0 , oc3 = 0 , onotifyc = 0 , oc4 = 0 , oa1 = 0 , oa2 = 0 , oa3 = 0 , onotifya = 0 , oa4 = 0 , choiceA = 1 , choiceO >= 0 , waitb = 0 , afterwaitb = 0 , waitd = 0 , afterwaitd = 0 , waitw = 0 , afterwaitw = 0 , waitj = 0 , afterwaitj = 0 , owaitb1 = 0 , oafterwaitb1 = 0 , owaitb2 = 0 , oafterwaitb2 = 0 , owaitd1 = 0 , oafterwaitd1 = 0 , owaitd2 = 0 , oafterwaitd2 = 0 , owaitw1 = 0 , oafterwaitw1 = 0 , owaitw2 = 0 , oafterwaitw2 = 0 , owaitj = 0 , oafterwaitj =0

target
notlockrefnull >= 1 , notlockrefa >= 1 , b5 >= 1
notlockrefnull >= 1 , notlockrefa >= 1 , w5 >= 1
notlockrefnull >= 1 , notlockrefa >= 1 , d5 >= 1
notlockrefnull >= 1 , notlockrefa >= 1 , j5 >= 1
notlockrefa >= 1 , b6 >= 1
notlockrefa >= 1 , w6 >= 1
notlockrefa >= 1 , d6 >= 1

invariants

unlock = 1, lock = 1
notlockrefa = 1, lockrefa = 1
lockrefnull = 1, notlockrefnull = 1
unlock = 1, b1 = 1, b2 = 1, b5 = 1, b6 = 1, d1 = 1, d2 = 1, d5 = 1, d6 = 1, w1 = 1, w2 = 1, w5 = 1, w6 = 1, j1 = 1, j2 = 1, j5 = 1, j6 = 1, o1 = 1, a1 = 1, a2 = 1, notifya = 1, a3 = 1, c1 = 1, c2 = 1, notifyc = 1, c3 = 1, ob1 = 1, ob2 = 1, ob3 = 1, ob5 = 1, ob6 = 1, ob7 = 1, ob10 = 1, od1 = 1, od2 = 1, od3 = 1, od5 = 1, od6 = 1, od7 = 1, od10 = 1, ow1 = 1, ow2 = 1, ow3 = 1, ow5 = 1, ow6 = 1, ow7 = 1, ow10 = 1, oj1 = 1, oj3 = 1, oj4 = 1, oo1 = 1, oc1 = 1, oc2 = 1, oc3 = 1, onotifyc = 1, oc4 = 1, oa1 = 1, oa2 = 1, oa3 = 1, onotifya = 1, oa4 = 1
