var
unlockT lockT unlockP lockP notslotTeq0 slotTeq0 notslotTeq1 slotTeq1 notslotTeq2  slotTeq2 notslotPeq0 slotPeq0 notslotPeq1 slotPeq1 notslotPeq2  slotPeq2 notptrTeq0 ptrTeq0 notptrTeq1 ptrTeq1 notptrPeq0 ptrPeq0 notptrPeq1 ptrPeq1  
put Pwhile P1 Pwait Pafterwait Pdecslot Pincptr Passign Pnotify Pslotinc Pbeforenotify Pafternotify Pend take Twhile T1 Twait Tafterwait Tdecslot Tincptr Tassign Tnotify Tslotinc Tbeforenotify Tafternotify Tend

init

unlockT = 1 && lockT = 0 && unlockP = 1 && lockP = 0 && notslotTeq0 = 0 && slotTeq0 = 1 && notslotTeq1 = 1 && slotTeq1 = 0 && notslotTeq2  = 1 && slotTeq2 = 0 && notslotPeq0 = 1 && slotPeq0 = 0 && notslotPeq1 = 1 && slotPeq1 = 0 && notslotPeq2 = 0 && slotPeq2 = 1 &&  notptrTeq0 = 0 && ptrTeq0 = 1 && notptrTeq1 = 1 && ptrTeq1 = 0 && notptrPeq0 = 0 && ptrPeq0 = 1 && notptrPeq1 = 1 && ptrPeq1 = 0 && 
put = 2 && Pwhile = 0 && P1 = 0 && Pwait = 0 && Pafterwait = 0 && Pdecslot = 0 && Pincptr = 0 && Passign = 0 && Pnotify = 0 && Pslotinc = 0 && Pbeforenotify = 0 && Pend = 0 && take = 2 && Twhile = 0 && T1 = 0 && Twait = 0 && Tafterwait = 0 && Tdecslot = 0 && Tincptr = 0 && Tassign = 0 && Tnotify = 0 && Tslotinc = 0 && Tbeforenotify = 0 && Tend = 0 && Pafternotify = 0 && Tafternotify = 0

goals

#Passign >= 1 && ptrPeq0 >= 1 && Tassign >= 1 &&  ptrTeq0 >= 1
#Passign >= 1 && ptrPeq1 >= 1 && Tassign >= 1 &&  ptrTeq1 >= 1
#notslotTeq0 >= 1 && notslotTeq1 >= 1 && notslotTeq2 >= 1 

notslotPeq0 >= 1 && notslotPeq1 >= 1 && notslotPeq2 >= 1
#putter 
rules

put >= 1&& unlockP >= 1 ->
	put' = put - 1&&
	Pwhile' = Pwhile + 1&&
	unlockP' = unlockP - 1&&
	lockP' = lockP + 1;

Pwhile >= 1&& slotPeq0 >= 1 ->
        Pwhile' = Pwhile - 1&&
        P1' = P1 + 1;

Pwhile >= 1&& notslotPeq1 >= 1&& notslotPeq2 >= 1 ->
        Pwhile' = Pwhile - 1&&
        P1' = P1 + 1;

P1 >= 1&& lockP >= 1 ->
	P1' = P1 - 1&&
	Pwait' = Pwait + 1&&
	lockP' = lockP - 1&&
	unlockP' = unlockP + 1;

Pafterwait >= 1&& unlockP >= 1 ->
	Pafterwait' = Pafterwait - 1&&
	Pwhile' = Pwhile + 1&&
	unlockP' = unlockP - 1&&
	lockP' = lockP + 1;

Pwhile >= 1&& notslotPeq0 >= 1 ->
	Pwhile' = Pwhile - 1&&
	Pdecslot' = Pdecslot + 1;

Pwhile >= 1&& notslotPeq1 >= 1&& notslotPeq2 >= 1&& notslotPeq0 >= 1 ->
        Pwhile' = Pwhile - 1&&
        Pdecslot' = Pdecslot + 1;


Pdecslot >= 1&& slotPeq0 >= 1 ->
	Pdecslot' = Pdecslot - 1&&
	Pincptr' = Pincptr + 1&&
	slotPeq0' = slotPeq0 - 1&&
	notslotPeq0' = notslotPeq0 + 1;

Pdecslot >= 1&& slotPeq1 >= 1 ->
        Pdecslot' = Pdecslot - 1&&
        Pincptr' = Pincptr + 1&&
        slotPeq1' = slotPeq1 - 1&&
        notslotPeq1' = notslotPeq1 + 1&&
	notslotPeq0' = 0&&
	slotPeq0' = slotPeq0 + notslotPeq0 + 0;

Pdecslot >= 1&& slotPeq2 >= 1 ->
        Pdecslot' = Pdecslot - 1&&
        Pincptr' = Pincptr + 1&&
        slotPeq2' = slotPeq2 - 1&&
        notslotPeq2' = notslotPeq2 + 1&&
        notslotPeq1' = 0&&
        slotPeq1' = slotPeq1 + notslotPeq1 + 0;

Pdecslot >= 1&& notslotPeq0 >= 1&& notslotPeq1 >= 1&& notslotPeq2 >= 1 ->
        Pdecslot' = Pdecslot - 1&&
        Pincptr' = Pincptr + 1;

Pdecslot >= 1&& notslotPeq0 >= 1&& notslotPeq1 >= 1&& notslotPeq2 >= 1 ->
        Pdecslot' = Pdecslot - 1&&
        Pincptr' = Pincptr + 1&&
	notslotPeq2' = notslotPeq2 - 1&&
	slotPeq2' = slotPeq2 + 1;


Pincptr >= 1&& ptrPeq0 >= 1 ->
	Pincptr' =Pincptr - 1&&
	Passign' = Passign + 1&&
	ptrPeq0' = ptrPeq0 - 1&&
	notptrPeq0' = notptrPeq0 + 1&&
	ptrPeq1' = ptrPeq1 + notptrPeq1 + 0&&
	notptrPeq1' = 0;

Pincptr >= 1&& ptrPeq1 >= 1 ->
        Pincptr' =Pincptr - 1&&
        Passign' = Passign + 1&&
        ptrPeq1' = ptrPeq1 - 1&&
        notptrPeq1' = notptrPeq1 + 1&&
        ptrPeq0' = ptrPeq0 + notptrPeq0 + 0&&
        notptrPeq0' = 0;

Pincptr >= 1&& notptrPeq1 >= 1&& notptrPeq0 >= 1 ->
        Pincptr' =Pincptr - 1&&
        Passign' = Passign + 1&&
	ptrPeq0' = ptrPeq0 + 1&&
	notptrPeq0' = notptrPeq0 - 1;

Pincptr >= 1&& notptrPeq1 >= 1&& notptrPeq0 >= 1 ->
        Pincptr' =Pincptr - 1&&
        Passign' = Passign + 1&&
        ptrPeq1' = ptrPeq1 + 1&&
        notptrPeq1' = notptrPeq1 - 1;


Passign >= 1&& lockP >= 1 ->
	Passign' = Passign - 1&&
	Pnotify' = Pnotify + 1&&
	lockP' = lockP - 1&&
	unlockP' = unlockP + 1;

Pnotify >= 1&& unlockT >= 1 ->
	Pnotify' = Pnotify - 1&&
	Pslotinc' = Pslotinc + 1&&
	unlockT' = unlockT - 1&&
	lockP' = lockT + 1;

Pslotinc >= 1&& slotTeq0 >= 1 ->
	Pslotinc' = Pslotinc - 1&&
	Pbeforenotify' = Pbeforenotify + 1&&
	slotTeq0' = slotTeq0 - 1&&
	notslotTeq0' = notslotTeq0 + 1&&
	slotTeq1' = slotTeq1 + notslotTeq1 + 0&&
	notslotTeq1' = 0;


Pslotinc >= 1&& slotTeq1 >= 1 ->
        Pslotinc' = Pslotinc - 1&&
        Pbeforenotify' = Pbeforenotify + 1&&
        slotTeq1' = slotTeq1 - 1&&
        notslotTeq1' = notslotTeq1 + 1&&
        slotTeq2' = slotTeq2 + notslotTeq2 + 0&&
        notslotTeq2' = 0;

Pslotinc >= 1&& slotTeq2 >= 1 ->
        Pslotinc' = Pslotinc - 1&&
        Pbeforenotify' = Pbeforenotify + 1&&
        slotTeq2' = slotTeq2 - 1&&
        notslotTeq2' = notslotTeq2 + 1;

Pslotinc >= 1&& notslotTeq0 >= 1&& notslotTeq1 >= 1&& notslotTeq2 >= 1 ->
        Pslotinc' = Pslotinc - 1&&
        Pbeforenotify' = Pbeforenotify + 1;
	
Pslotinc >= 1&& notslotTeq0 >= 1&& notslotTeq1 >= 1&& notslotTeq2 >= 1 ->
        Pslotinc' = Pslotinc - 1&&
        Pbeforenotify' = Pbeforenotify + 1&&
        slotTeq0' = slotTeq0 + 1&&
        notslotTeq0' = notslotTeq0 - 1;
	
#attention&& c'est un notify normalement!

Pbeforenotify >= 1 ->
	Pbeforenotify' = Pbeforenotify - 1&&
	Pafternotify' = Pafternotify + 1&&
	Pafterwait' = Pafterwait + Pwait + 0&&
	Pwait' = 0&&
	Tafterwait' = Tafterwait + Twait + 0&&
	Twait' = 0;

Pafternotify >= 1&& lockT >= 1 ->
	Pafternotify' = Pafternotify - 1&&
	Pend' = Pend + 1&&
	lockT' = lockT - 1&&
	unlockT' = unlockT + 1;

Pend >= 1 ->
	Pend' = Pend - 1&&
	put' = put + 1;

#taker 

take >= 1&& unlockT >= 1 ->
	take' = take - 1&&
	Twhile' = Twhile + 1&&
	unlockT' = unlockT - 1&&
	lockT' = lockT + 1;

Twhile >= 1&& slotTeq0 >= 1 ->
        Twhile' = Twhile - 1&&
        T1' = T1 + 1;

Twhile >= 1&& notslotTeq1 >= 1&& notslotTeq2 >= 1 ->
        Twhile' = Twhile - 1&&
        T1' = T1 + 1;

T1 >= 1&& lockT >= 1 ->
	T1' = T1 - 1&&
	Twait' = Twait + 1&&
	lockT' = lockT - 1&&
	unlockT' = unlockT + 1;

Tafterwait >= 1&& unlockT >= 1 ->
	Tafterwait' = Tafterwait - 1&&
	Twhile' = Twhile + 1&&
	unlockT' = unlockT - 1&&
	lockT' = lockT + 1;

Twhile >= 1&& notslotTeq0 >= 1 ->
	Twhile' = Twhile - 1&&
	Tdecslot' = Tdecslot + 1;

Twhile >= 1&& notslotTeq1 >= 1&& notslotTeq2 >= 1&& notslotTeq0 >= 1 ->
        Twhile' = Twhile - 1&&
        Tdecslot' = Tdecslot + 1;


Tdecslot >= 1&& slotTeq0 >= 1 ->
	Tdecslot' = Tdecslot - 1&&
	Tincptr' = Tincptr + 1&&
	slotTeq0' = slotTeq0 - 1&&
	notslotTeq0' = notslotTeq0 + 1;

Tdecslot >= 1&& slotTeq1 >= 1 ->
        Tdecslot' = Tdecslot - 1&&
        Tincptr' = Tincptr + 1&&
        slotTeq1' = slotTeq1 - 1&&
        notslotTeq1' = notslotTeq1 + 1&&
	notslotTeq0' = 0&&
	slotTeq0' = slotTeq0 + notslotTeq0 + 0;

Tdecslot >= 1&& slotTeq2 >= 1 ->
        Tdecslot' = Tdecslot - 1&&
        Tincptr' = Tincptr + 1&&
        slotTeq2' = slotTeq2 - 1&&
        notslotTeq2' = notslotTeq2 + 1&&
        notslotTeq1' = 0&&
        slotTeq1' = slotTeq1 + notslotTeq1 + 0;

Tdecslot >= 1&& notslotTeq0 >= 1&& notslotTeq1 >= 1&& notslotTeq2 >= 1 ->
        Tdecslot' = Tdecslot - 1&&
        Tincptr' = Tincptr + 1;

Tdecslot >= 1&& notslotTeq0 >= 1&& notslotTeq1 >= 1&& notslotTeq2 >= 1 ->
        Tdecslot' = Tdecslot - 1&&
        Tincptr' = Tincptr + 1&&
	notslotTeq2' = notslotTeq2 - 1&&
	slotTeq2' = slotTeq2 + 1;


Tincptr >= 1&& ptrTeq0 >= 1 ->
	Tincptr' =Tincptr - 1&&
	Tassign' = Tassign + 1&&
	ptrTeq0' = ptrTeq0 - 1&&
	notptrTeq0' = notptrTeq0 + 1&&
	ptrTeq1' = ptrTeq1 + notptrTeq1 + 0&&
	notptrTeq1' = 0;

Tincptr >= 1&& ptrTeq1 >= 1 ->
        Tincptr' =Tincptr - 1&&
        Tassign' = Tassign + 1&&
        ptrTeq1' = ptrTeq1 - 1&&
        notptrTeq1' = notptrTeq1 + 1&&
        ptrTeq0' = ptrTeq0 + notptrTeq0 + 0&&
        notptrTeq0' = 0;

Tincptr >= 1&& notptrTeq1 >= 1&& notptrTeq0 >= 1 ->
        Tincptr' =Tincptr - 1&&
        Tassign' = Tassign + 1&&
	ptrTeq0' = ptrTeq0 + 1&&
	notptrTeq0' = notptrTeq0 - 1;

Tincptr >= 1&& notptrTeq1 >= 1&& notptrTeq0 >= 1 ->
        Tincptr' =Tincptr - 1&&
        Tassign' = Tassign + 1&&
        ptrTeq1' = ptrTeq1 + 1&&
        notptrTeq1' = notptrTeq1 - 1;


Tassign >= 1&& lockT >= 1 ->
	Tassign' = Tassign - 1&&
	Tnotify' = Tnotify + 1&&
	lockT' = lockT - 1&&
	unlockT' = unlockT + 1;

Tnotify >= 1&& unlockP >= 1 ->
	Tnotify' = Tnotify - 1&&
	Tslotinc' = Tslotinc + 1&&
	unlockP' = unlockP - 1&&
	lockP' = lockP + 1;

Tslotinc >= 1&& slotPeq0 >= 1 ->
	Tslotinc' = Tslotinc - 1&&
	Tbeforenotify' = Tbeforenotify + 1&&
	slotPeq0' = slotPeq0 - 1&&
	notslotPeq0' = notslotPeq0 + 1&&
	slotPeq1' = slotPeq1 + notslotPeq1 + 0&&
	notslotPeq1' = 0;


Tslotinc >= 1&& slotPeq1 >= 1 ->
        Tslotinc' = Tslotinc - 1&&
        Tbeforenotify' = Tbeforenotify + 1&&
        slotPeq1' = slotPeq1 - 1&&
        notslotPeq1' = notslotPeq1 + 1&&
        slotPeq2' = slotPeq2 + notslotPeq2 + 0&&
        notslotPeq2' = 0;

Tslotinc >= 1&& slotPeq2 >= 1 ->
        Tslotinc' = Tslotinc - 1&&
        Tbeforenotify' = Tbeforenotify + 1&&
        slotPeq2' = slotPeq2 - 1&&
        notslotPeq2' = notslotPeq2 + 1;

Tslotinc >= 1&& notslotPeq0 >= 1&& notslotPeq1 >= 1&& notslotPeq2 >= 1 ->
        Tslotinc' = Tslotinc - 1&&
        Tbeforenotify' = Tbeforenotify + 1;
	
Tslotinc >= 1&& notslotPeq0 >= 1&& notslotPeq1 >= 1&& notslotPeq2 >= 1 ->
        Tslotinc' = Tslotinc - 1&&
        Tbeforenotify' = Tbeforenotify + 1&&
        slotPeq0' = slotPeq0 + 1&&
        notslotPeq0' = notslotPeq0 - 1;
	
#attention&& c'est un notify normalement!

Tbeforenotify >= 1 ->
	Tbeforenotify' = Tbeforenotify - 1&&
	Tafternotify' = Tafternotify + 1&&
	Pafterwait' = Pafterwait + Pwait + 0&&
	Pwait' = 0&&
	Tafterwait' = Tafterwait + Twait + 0&&
	Twait' = 0;

Tafternotify >= 1&& lockP >= 1 ->
	Tafternotify' = Tafternotify - 1&&
	Tend' = Tend + 1&&
	lockP' = lockP - 1&&
	unlockP' = unlockP + 1;

Tend >= 1 ->
	Tend' = Tend - 1&&
	take' = take + 1;
		
