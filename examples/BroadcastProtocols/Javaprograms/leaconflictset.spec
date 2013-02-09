### example from the book Concurrent Programming in Java, Second Edition. -- Pages 204-205

vars
notstoringneq0 storingneq0 notretreivingneq0 retreivingneq0 unlockI lockI
Rwhile2 Rbeforewait Rincretreiving Rexitlock Rfinally Rnotifyall Rend Swhile2 Sbeforewait Sincstoring Sexitlock Sfinally Snotifyall Send Rwhile1 Rretreiving Rwait Rafterwait Rdoretreiving Swhile1 Sstoring Swait Safterwait Sdostore


#Retreiving threads
rules

Rwhile1 >= 1 ->
	Rwhile1' = Rwhile1 - 1,
	Rretreiving' = Rretreiving + 1;

Rretreiving >= 1, unlockI >= 1 ->
	Rretreiving' = Rretreiving - 1,
	Rwhile2' = Rwhile2 + 1,
	unlockI' = unlockI - 1,
	lockI' = lockI + 1;

Rwhile2 >= 1, storingneq0 >= 1 ->
	Rwhile2' = Rwhile2 - 1,
	Rbeforewait' = Rbeforewait + 1;

Rwhile2 >= 1, retreivingneq0 >= 1 ->
        Rwhile2' = Rwhile2 - 1,
        Rbeforewait' = Rbeforewait + 1;

Rbeforewait >= 1, lockI >= 1 ->
	Rbeforewait' = Rbeforewait - 1,
	Rwait' = Rwait + 1,
	lockI' = lockI - 1,
	unlockI' = unlockI + 1;

Rafterwait >= 1, unlockI >= 1 ->
	Rafterwait' = Rafterwait - 1,
	Rwhile2' = Rwhile2 + 1,
	unlockI' = unlockI - 1,
	lockI' = lockI + 1;

Rwhile2 >= 1, notstoringneq0 >= 1, notretreivingneq0 >= 1 ->
	Rwhile2' = Rwhile2 - 1,
	Rincretreiving' = Rincretreiving + 1;

Rincretreiving >= 1, notretreivingneq0 >= 1 ->
	Rincretreiving' = Rincretreiving - 1,
	Rexitlock' = Rexitlock + 1,
	notretreivingneq0' = notretreivingneq0 - 1,
	retreivingneq0' = retreivingneq0 + 1;

Rincretreiving >= 1, retreivingneq0 >= 1 ->
        Rincretreiving' = Rincretreiving - 1,
        Rexitlock' = Rexitlock + 1,
        retreivingneq0' = retreivingneq0 - 1,
        notretreivingneq0' = notretreivingneq0 + 1;

Rincretreiving >= 1, retreivingneq0 >= 1 ->
        Rincretreiving' = Rincretreiving - 1,
        Rexitlock' = Rexitlock + 1;

Rexitlock >= 1, lockI >= 1 ->
	Rexitlock' = Rexitlock - 1,
	Rdoretreiving' = Rdoretreiving + 1,
	lockI' = lockI - 1,
	unlockI' = unlockI + 1;

Rdoretreiving >= 1, unlockI >= 1 ->
	Rdoretreiving' = Rdoretreiving - 1,
	Rfinally' = Rfinally + 1,
	unlockI' = unlockI - 1,
	lockI' = lockI + 1;

Rfinally >= 1, notretreivingneq0 >= 1 ->
	Rfinally' = Rfinally - 1,
	Rnotifyall' = Rnotifyall + 1,
	notretreivingneq0' = notretreivingneq0 - 1,
	retreivingneq0' = retreivingneq0 + 1;

Rfinally >= 1, retreivingneq0 >= 1 ->
        Rfinally' = Rfinally - 1,
        Rnotifyall' = Rnotifyall + 1,
        retreivingneq0' = retreivingneq0 - 1,
        notretreivingneq0' = notretreivingneq0 + 1;

Rfinally >= 1, retreivingneq0 >= 1 ->
        Rfinally' = Rfinally - 1,
        Rnotifyall' = Rnotifyall + 1;

Rnotifyall >= 1, notretreivingneq0 >= 1 ->
	Rnotifyall' = Rnotifyall - 1,
	Rend' = Rend + 1,
	Rafterwait' = Rafterwait + Rwait + 0,
	Rwait' = 0,
	Safterwait' = Safterwait + Swait + 0,
	Swait' = 0;

Rnotifyall >= 1, retreivingneq0 >= 1 ->
        Rnotifyall' = Rnotifyall - 1,
        Rend' = Rend + 1;

Rend >= 1, lockI >= 1 ->
	Rend' = Rend - 1,
	Rwhile1' = Rwhile1 + 1,
	lockI' = lockI - 1,
	unlockI' = unlockI + 1;

#storing threads

Swhile1 >= 1 ->
	Swhile1' = Swhile1 - 1,
	Sstoring' = Sstoring + 1;

Sstoring >= 1, unlockI >= 1 ->	
	Sstoring' = Sstoring - 1,
	Swhile2' = Swhile2 + 1,
 	unlockI' = unlockI - 1,
	lockI' = lockI + 1;

Swhile2 >= 1, retreivingneq0 >= 1 ->
	Swhile2' = Swhile2 - 1,
	Sbeforewait' = Sbeforewait + 1;

Sbeforewait >= 1, lockI >= 1 ->
	Sbeforewait' = Sbeforewait - 1,
	Swait' = Swait + 1,
	lockI' = lockI - 1,
	unlockI' = unlockI + 1;

Safterwait >= 1, unlockI >= 1 ->
	Safterwait' = Safterwait - 1,
	Swhile2' = Swhile2 + 1,
	unlockI' = unlockI - 1,
	lockI' = lockI + 1;

Swhile2 >= 1, notretreivingneq0 >= 1 -> 
	Swhile2' = Swhile2 - 1,
	Sincstoring' = Sincstoring + 1;

Sincstoring >= 1, notstoringneq0 >= 1 ->
	Sincstoring' = Sincstoring - 1,
	Sexitlock' = Sexitlock + 1,
	notstoringneq0' =notstoringneq0 - 1,
	storingneq0' = storingneq0 + 1;

Sincstoring >= 1, storingneq0 >= 1 ->
        Sincstoring' = Sincstoring - 1,
        Sexitlock' = Sexitlock + 1,
        storingneq0' =storingneq0 - 1,
        notstoringneq0' = notstoringneq0 + 1;

Sincstoring >= 1, storingneq0 >= 1 ->
        Sincstoring' = Sincstoring - 1,
        Sexitlock' = Sexitlock + 1;

Sexitlock >= 1, lockI >= 1 ->
	Sexitlock' = Sexitlock - 1,
	Sdostore' = Sdostore + 1,
	lockI' = lockI - 1,
	unlockI' = unlockI + 1;

Sdostore >= 1, unlockI >= 1 ->
	Sdostore' = Sdostore  - 1,
	Sfinally' = Sfinally + 1,
	unlockI' = unlockI - 1,
	lockI' = lockI + 1;

Sfinally >= 1, storingneq0 >= 1 ->
	Sfinally' = Sfinally - 1,
	Snotifyall' = Snotifyall + 1;

Sfinally >= 1, storingneq0 >= 1 ->
        Sfinally' = Sfinally - 1,
        Snotifyall' = Snotifyall + 1,
	storingneq0' = storingneq0 - 1,
	notstoringneq0' = notstoringneq0 + 1;

Sfinally >= 1, notstoringneq0 >= 1 ->
        Sfinally' = Sfinally - 1,
        Snotifyall' = Snotifyall + 1,
        notstoringneq0' = notstoringneq0 - 1,
        storingneq0' = storingneq0 + 1;

Snotifyall >= 1, notstoringneq0 >= 1 ->
	Snotifyall' = Snotifyall - 1,
	Send' = Send + 1,
	Safterwait' = Safterwait + Swait + 0,
	Swait' = 0,
	Rafterwait' = Rafterwait + Rwait + 0,
	Rwait' = 0;

Snotifyall >= 1, storingneq0 >= 1 ->
        Snotifyall' = Snotifyall - 1,
        Send' = Send + 1;

Send >= 1, lockI >= 1 ->
	Send' =Send  - 1,
	Swhile1' = Swhile1 + 1,
	lockI' = lockI - 1,
	unlockI' = unlockI + 1;

init
notstoringneq0 = 1 , storingneq0 = 0 , notretreivingneq0 = 1 , retreivingneq0 = 0 , unlockI = 1 , lockI = 0 ,
Rwhile1 >= 1 , Rretreiving = 0 , Rwhile2 = 0 , Rbeforewait = 0 , Rwait = 0 , Rafterwait = 0 , Rincretreiving = 0 , Rexitlock = 0 , Rdoretreiving = 0 , Rfinally = 0 , Rnotifyall = 0 , Rend = 0 , Swhile1 >= 1 , Sstoring = 0 , Swhile2 = 0 , Sbeforewait = 0 , Swait = 0 , Safterwait = 0 , Sincstoring = 0 , Sexitlock = 0 , Sdostore = 0 , Sfinally = 0 , Snotifyall = 0 , Send = 0

target
Rdoretreiving >= 1 , Sdostore >= 1
#Rdoretreiving >= 2 

invariants
notstoringneq0 = 1, storingneq0 = 1
notretreivingneq0 = 1, retreivingneq0 = 1
unlockI = 1, lockI = 1
unlockI = 1, Rwhile2 = 1, Rbeforewait = 1, Rincretreiving = 1, Rexitlock = 1, Rfinally = 1, Rnotifyall = 1, Rend = 1, Swhile2 = 1, Sbeforewait = 1, Sincstoring = 1, Sexitlock = 1, Sfinally = 1, Snotifyall = 1, Send = 1
