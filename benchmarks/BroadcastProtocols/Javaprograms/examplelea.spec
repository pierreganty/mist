#expected result: safe
### example from the book Concurrent Programming in Java, Second Edition. -- Pages 242-243


### attention, il faut normalement remplacer les notifyAll par des notify ###

vars
  unlockTC lockTC unlockC lockC unlockS lockS notvalue value Cwhile1 Cbad Cwhile2 Cwait Cafterwait Cchoice C1 C2 C3 C4 C5 C6 C7 C8 C9 C10 C11 C12 C13 C14 Swhile1 Sbad Swhile2 Swait Safterwait Schoice S1 S2 S3 S4 S5 S6 S7 S8 S9 S10 S11 S12 S13 S14

rules

#process manipulationg the checking account

Cwhile1 >= 1, unlockC >= 1 ->
	Cwhile1' = Cwhile1 - 1,
	C1' = C1 + 1,
	unlockC' = unlockC - 1,
	lockC' = lockC + 1;

C1 >= 1, unlockTC >= 1 ->
	C1' = C1 - 1,
	C2' = C2 + 1,
	unlockTC' = unlockTC - 1,
	lockTC' = lockTC + 1;

C2 >= 1, value >= 1 ->
	C2' = C2 - 1,
	C3' = C3 + 1;

C3 >= 1 ->
	C3' = C3 - 1,
	C4' = C4 + 1,
	value' =value + notvalue + 0,
	notvalue' = 0;

C4 >= 1, lockTC >= 1 ->
	C4' = C4 - 1,
	Cwhile2' = Cwhile2 + 1,
	lockTC' = lockTC - 1,
	unlockTC' = unlockTC + 1;

Cwhile2 >= 1, lockC >= 1 ->
	Cwhile2' = Cwhile2 - 1,
	Cwait' = Cwait + 1,
	lockC' = lockC - 1,
	unlockC' = unlockC + 1;

Cafterwait >= 1 , unlockC >= 1 ->
	Cafterwait' = Cafterwait - 1,
	C14' = C14  + 1,
	unlockC' = unlockC - 1,
	lockC' = lockC + 1;

C14 >= 1 , unlockTC >= 1 ->
	C14' = C14 - 1,
	C2' = C2 + 1,
	unlockTC' = unlockTC - 1,
	lockTC' = lockTC + 1;

C2 >= 1, notvalue >= 1 ->
	C2' = C2 - 1,
	C5' = C5 + 1;

C5 >= 1 ->
	C5' = C5 - 1,
	C6' = C6 + 1,
	value' = value + notvalue + 0,
	notvalue' = 0;

C6 >= 1 , lockTC >= 1 ->
	C6' = C6 - 1,
	Cchoice' = Cchoice + 1,
	lockTC' = lockTC - 1,
	unlockTC' = unlockTC + 1;

Cchoice >= 1 ->
	Cchoice' = Cchoice - 1,
	C13' = C13 + 1;

Cchoice >= 1 ->
	Cchoice' = Cchoice - 1,
	Cbad' = Cbad + 1;

C13 >= 1, unlockTC >= 1 ->
	C13' = C13 - 1,
	C7' = C7 + 1,
	unlockTC' = unlockTC - 1,
	lockTC' = lockTC + 1;

C7 >= 1 ->
	C7' = C7 - 1,
	C8' = C8 + 1,
	notvalue' = notvalue + value + 0,
	value' = 0;

C8 >= 1, lockTC >= 1 ->
	C8' = C8 - 1,
	C12' = C12 + 1,
	lockTC' = lockTC - 1,
	unlockTC' = unlockTC + 1;


### On doit avoir des asynchronous rdv a la place des transferts ###

C12 >= 1 ->
	C12' = C12 - 1,
	C9' = C9 + 1,
	Cafterwait' = Cafterwait + Cwait + 0,
	Cwait' = 0,
	Safterwait' = Safterwait + Swait + 0,
	Swait' = 0;

C9 >= 1, lockC >= 1 ->
	C9' = C9 - 1,
	Cwhile1' = Cwhile1 + 1,
	lockC' = lockC - 1,
	unlockC' = unlockC + 1;

Cbad >= 1, unlockS >= 1 ->
	Cbad' = Cbad - 1,
	C10' = C10 + 1,
	unlockS' = unlockS - 1,
	lockS' = lockS + 1;

C10 >= 1, lockS >= 1 ->
	C10' = C10 - 1,
	C11' = C11 + 1,
	lockS' = lockS - 1,
	unlockS' = unlockS + 1;

C11 >= 1, unlockTC >= 1 ->
	C11' = C11 - 1,
	C7' = C7 + 1,
	unlockTC' = unlockTC - 1,
	lockTC' = lockTC + 1;


#process manipulationg the saving account

Swhile1 >= 1, unlockS >= 1 ->
	Swhile1' = Swhile1 - 1,
	S1' = S1 + 1,
	unlockS' = unlockS - 1,
	lockS' = lockS + 1;

S1 >= 1, unlockTC >= 1 ->
	S1' = S1 - 1,
	S2' = S2 + 1,
	unlockTC' = unlockTC - 1,
	lockTC' = lockTC + 1;

S2 >= 1, value >= 1 ->
	S2' = S2 - 1,
	S3' = S3 + 1;

S3 >= 1 ->
	S3' = S3 - 1,
	S4' = S4 + 1,
	value' =value + notvalue + 0,
	notvalue' = 0;

S4 >= 1, lockTC >= 1 ->
	S4' = S4 - 1,
	Swhile2' = Swhile2 + 1,
	lockTC' = lockTC - 1,
	unlockTC' = unlockTC + 1;

Swhile2 >= 1, lockS >= 1 ->
	Swhile2' = Swhile2 - 1,
	Swait' = Swait + 1,
	lockS' = lockS - 1,
	unlockS' = unlockS + 1;

Safterwait >= 1 , unlockS >= 1 ->
	Safterwait' = Safterwait - 1,
	S14' = S14  + 1,
	unlockS' = unlockS - 1,
	lockS' = lockS + 1;

S14 >= 1 , unlockTC >= 1 ->
	S14' = S14 - 1,
	S2' = S2 + 1,
	unlockTC' = unlockTC - 1,
	lockTC' = lockTC + 1;

S2 >= 1, notvalue >= 1 ->
	S2' = S2 - 1,
	S5' = S5 + 1;

S5 >= 1 ->
	S5' = S5 - 1,
	S6' = S6 + 1,
	value' = value + notvalue + 0,
	notvalue' = 0;

S6 >= 1 , lockTC >= 1 ->
	S6' = S6 - 1,
	lockTC' = lockTC - 1,
	unlockTC' = unlockTC + 1,
	Schoice' = Schoice + 1;

Schoice >= 1 ->
	Schoice' = Schoice - 1,
	S13' = S13 + 1;

S13 >= 1, unlockTC >= 1 ->
	S13' = S13 - 1,
	S7' = S7 + 1,
	unlockTC' = unlockTC - 1,
	lockTC' = lockTC + 1;

S7 >= 1 ->
	S7' = S7 - 1,
	S8' = S8 + 1,
	notvalue' = notvalue + value + 0,
	value' = 0;

S8 >= 1, lockTC >= 1 ->
	S8' = S8 - 1,
	S12' = S12 + 1,
	lockTC' = lockTC - 1,
	unlockTC' = unlockTC + 1;


### On doit avoir des asynchronous rdv a la place de transferts. ###

S12 >= 1 ->
	S12' = S12 - 1,
	S9' = S9 + 1,
	Cafterwait' = Cafterwait + Cwait + 0,
	Cwait' = 0,
	Safterwait' = Safterwait + Swait + 0,
	Swait' = 0;

S9 >= 1, lockS >= 1 ->
	S9' = S9 - 1,
	Swhile1' = Swhile1 + 1,
	lockS' = lockS - 1,
	unlockS' = unlockS + 1;

Schoice >= 1 ->
	Schoice' = Schoice - 1,
	Sbad' = Sbad + 1;

Sbad >= 1, unlockC >= 1 ->
	Sbad' = Sbad - 1,
	S10' = S10 + 1,
	unlockC' = unlockC - 1,
	lockC' = lockC + 1;

S10 >= 1, lockC >= 1 ->
	S10' = S10 - 1,
	S11' = S11 + 1,
	lockC' = lockC - 1,
	unlockC' = unlockC + 1;

S11 >= 1, unlockTC >= 1 ->
	S11' = S11 - 1,
	S7' = S7 + 1,
	unlockTC' = unlockTC - 1,
	lockTC' = lockTC + 1;


init

  unlockTC = 1 , lockTC = 0 , unlockC = 1, lockC = 0, unlockS = 1, lockS = 0, notvalue = 1, value = 0, Cwhile1 >=1, Cwhile2 = 0, Cwait = 0, Cafterwait = 0, C1 = 0, C2 = 0, C3 = 0, C4 = 0, C5 = 0, C6 = 0, C7 = 0, C8 = 0, C9 = 0, C10 = 0, C11 = 0, C12 = 0, Swhile1 >= 1, Swhile2 = 0, Swait = 0, Safterwait = 0, S1 = 0, S2 = 0, S3 = 0, S4 = 0, S5 = 0, S6 = 0, S7 = 0, S8 = 0, S9 = 0, S10 = 0, S11 = 0, S12 = 0, C13 = 0, Cbad = 0, S13 = 0, Sbad = 0 , Cchoice = 0 , Schoice = 0 , C14 = 0 , S14 = 0

target
Cbad >= 1, Sbad >= 1

invariants
unlockTC = 1, lockTC = 1
unlockC = 1, lockC = 1
unlockS = 1, lockS = 1
notvalue = 1, value = 1
unlockTC = 1, C2 = 1, C3 = 1, C4 = 1, C5 = 1, C6 = 1, S2 = 1, S3 = 1, S4 = 1, S5 = 1, S6 = 1
unlockC = 1, Cbad = 1, Cwhile2 = 1, Cchoice = 1, C1 = 1, C2 = 1, C3 = 1, C4 = 1, C5 = 1, C6 = 1, C7 = 1, C8 = 1, C9 = 1, C12 = 1, C13 = 1, C14 = 1, S10 = 1
unlockS  = 1, C10 = 1, Sbad = 1, Swhile2 = 1, Schoice = 1, S1 = 1, S2 = 1, S3 = 1, S4 = 1, S5 = 1, S6 = 1, S7 = 1,  S8 = 1, S9 = 1, S12 = 1, S13 = 1, S14 = 1
