#variable definition
vars
  initc whilec whileget waitget competec notifyc endc initp whilep whileput waitput competep notifyp returnp available notavailable lock notlock 



#Rules section
rules

#consumer

initc >= 1 ->
	initc' = initc - 1
	,
	whilec' = whilec + 1;

whilec >= 1 , notlock >= 1 ->
	whilec' = whilec - 1
	,
	whileget' = whileget + 1
	,
	notlock' = notlock - 1
	,
	lock' = lock + 1;

whileget >= 1 , notavailable >= 1 , lock >= 1 ->
	whileget' = whileget - 1
	,
	waitget' = waitget + 1
	,
	lock' = lock - 1
	,
	notlock' = notlock + 1;

competec >= 1 , notlock >= 1 ->
	competec' = competec - 1
	,
	whileget' = whileget + 1
	,
	notlock' = notlock - 1
	,
	lock' = lock + 1;

whileget >= 1 , available >= 1 ->
	whileget' = whileget - 1
	,
	notifyc' = notifyc + 1
	,
	notavailable' = notavailable + 1 
	,
	available' = available - 1;

notifyc >= 1 ->
	notifyc' = notifyc - 1
	,
	endc' = endc + 1
	,
	competec' = competec + waitget + 0
	,
	waitget' = 0
	,
	competep' = competep + waitput + 0
	,
	waitput' = 0;

endc >= 1 , lock >= 1 ->
	endc' = endc - 1
	,
	whilec' = whilec + 1
	,
	lock' = lock - 1
	,
	notlock' = notlock + 1;

# producer

initp >= 1 -> 
	initp' =initp - 1
	,
	whilep' = whilep + 1;

whilep >= 1 , notlock >= 1 -> 
	whilep' = whilep - 1
	,
	whileput' = whileput + 1
	,
	notlock' = notlock - 1
	,
	lock' = lock + 1;

whileput >= 1 , lock >= 1 , available >= 1 ->
	whileput' = whileput - 1
	,
	waitput' = waitput + 1
	,
	lock' = lock - 1
	,
	notlock' = notlock + 1;
	
competep >= 1 , notlock >= 1 ->
	competep' = competep - 1
	,
	whileput' = whileput + 1
	,
	notlock' = notlock - 1
	,
	lock' = lock + 1;
	
whileput >= 1 , notavailable >= 1 ->
	whileput' = whileput - 1
	,
	notifyp' = notifyp + 1
	,
	available' = available + 1 
	,
	notavailable' = notavailable - 1;

notifyp >= 1 ->
	notifyp' = notifyp - 1
	,
	returnp' = returnp + 1
	,
	competec' = competec + waitget + 0
	,
	waitget' = 0
	,
	competep' = competep + waitput + 0
	,
	waitput' = 0;

returnp >= 1 , lock >= 1 ->
	returnp' = returnp - 1
	,
	whilep' = whilep + 1
	,
	lock' = lock - 1
	,
	notlock' = notlock + 1;

#init section
init
  initc >= 1, initp >= 1, whilec = 0, whileget = 0, waitget = 0, competec = 0, notifyc = 0, endc = 0,  whilep = 0, whileput = 0, waitput = 0, competep = 0, notifyp = 0, returnp = 0, available = 0, notavailable = 0, lock = 0, notlock = 0 

#Goals section
target
  waitput >= 2 , waitget >= 2 

invariants

available = 1, notavailable = 1
lock = 1, notlock = 1
initc = 1, whilec = 1, whileget = 1, waitget = 1, competec = 1, notifyc = 1, endc = 1
initp = 1, whilep = 1, whileput = 1, waitput = 1, competep = 1, notifyp = 1, returnp = 1
initc = 1, whilec = 1, waitget = 1, competec = 1, initp = 1, whilep = 1, waitput = 1, competep = 1, lock = 1

whileget = 1, notifyc = 1, endc = 1, whileput = 1, notifyp = 1, returnp = 1, notlock = 1
