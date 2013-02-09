### example from the book Java threads. -- Pages 213 ###

vars
unlock lock flageqi notflageqi flageqmei notflageqmei flageqj notflageqj flageqmej notflageqmej busycounteq0 notbusycounteq0 waitersi notwaitersi  waitersj notwaitersj
Iwhile Igetbusyflag Ichoice1 Iif1 Ichoice2 Igoto1 Ichoice3 Iwhile2 Iwait Iafterwait Iafterwhile Ibeforeendget Iendget Iwork Iif2 Iif3 Ichoice4 Inotifyall Iassign Ichoice5 Iendfree Ierror
Jwhile Jgetbusyflag Jchoice1 Jif1 Jchoice2 Jgoto1 Jchoice3 Jwhile2 Jwait Jafterwait Jafterwhile Jbeforeendget Jendget Jwork Jif2 Jif3 Jchoice4 Jnotifyall Jassign Jchoice5 Jendfree Jerror
Owhile Ogetbusyflag Ochoice1 Oif1 Ochoice2 Ogoto1 Ochoice3 Owhile2 Owait Oafterwait Oafterwhile Obeforeendget Oendget Owork Oif2 Oif3 Ochoice4 Onotifyall Oassign Ochoice5 Oendfree
Oerror

rules 

#process I

Iwhile >= 1 ->
	Iwhile' = Iwhile - 1,
	Igetbusyflag' = Igetbusyflag + 1;

Igetbusyflag >= 1 , unlock >= 1 ->
	Igetbusyflag' = Igetbusyflag - 1,
	Ichoice1' = Ichoice1 + 1,
	unlock' = unlock - 1,
	lock' = lock + 1;

Ichoice1 >= 1 , flageqi >= 1 ->
	Ichoice1' = Ichoice1 - 1,
	Iif1' = Iif1 + 1,
	flageqmei' = flageqmei + notflageqmei + 0,
	notflageqmei' = 0;

Ichoice1 >= 1 , notflageqi >= 1 ->
        Ichoice1' = Ichoice1 - 1,
        Iif1' = Iif1 + 1,
        notflageqmei' = flageqmei + notflageqmei + 0,
        flageqmei' = 0;

Iif1 >= 1 , flageqmei >= 1 ->
	Iif1' = Iif1 - 1,
	Ichoice2' = Ichoice2 + 1;

Iif1 >= 1 , notflageqmei >= 1 ->
	Iif1' = Iif1 - 1,
	Ichoice3' = Ichoice3 + 1;

Ichoice2 >= 1 , busycounteq0 >= 1 ->
	Ichoice2' = Ichoice2 - 1,
	Igoto1' = Igoto1 + 1,
	notbusycounteq0' = notbusycounteq0 + 1,
	busycounteq0' = busycounteq0 - 1; 

Ichoice2 >= 1 , notbusycounteq0 >= 1 ->
	Ichoice2' = Ichoice2 - 1,
	Igoto1' = Igoto1 + 1;


Ichoice2 >= 1 , notbusycounteq0 >= 1 ->
        Ichoice2' = Ichoice2 - 1,
        Igoto1' = Igoto1 + 1,
	notbusycounteq0' = notbusycounteq0 - 1,
	busycounteq0' = busycounteq0 + 1;

Igoto1 >= 1 ->
	Igoto1' = Igoto1 - 1,
	Iendget' = Iendget + 1;

Ichoice3 >= 1 , waitersi >= 1 ->
	Ichoice3' = Ichoice3 - 1,
	Iwhile2' = Iwhile2 + 1;

Ichoice3 >= 1 , waitersj >= 1 ->
        Ichoice3' = Ichoice3 - 1,
        Iwhile2' = Iwhile2 + 1;

Ichoice3 >= 1 , notwaitersi >= 1 , notwaitersj >= 1 ->
        Ichoice3' = Ichoice3 - 1,
        Iwhile2' = Iwhile2 + 1;


Ichoice3 >= 1 , notwaitersi >= 1 , notwaitersj >= 1 ->
        Ichoice3' = Ichoice3 - 1,
        Iwhile2' = Iwhile2 + 1,
	notwaitersi' = notwaitersi - 1,
	waitersi' = waitersi + 1;

Iwhile2 >= 1 , notwaitersi >= 1 , lock >= 1 ->
	Iwhile2' = Iwhile2 - 1,
	Iwait' = Iwait + 1,
	lock' = lock - 1,
	unlock' = unlock + 1;

Iwhile2 >= 1 , waitersi >= 1 ->
	Iwhile2' = Iwhile2 - 1,
	Iafterwhile' = Iafterwhile + 1;

Iafterwait >= 1 , unlock >= 1 ->
	Iafterwait' = Iafterwait - 1,
	Iwhile2' = Iwhile2 + 1,
	unlock' = unlock - 1,
	lock' = lock + 1;
	
Iafterwhile >= 1 ->
	Iafterwhile' = Iafterwhile - 1,
	Ibeforeendget' = Ibeforeendget + 1,
	flageqmei' =  flageqmei + notflageqmei + 0,
	notflageqmei' = 0,
	notflageqmej' =  flageqmej + notflageqmej + 0,
	flageqmej' = 0,
	flageqi' = flageqi + notflageqi + 0,
	notflageqi' = 0,
	notflageqj' = flageqj + notflageqj + 0,
	notflageqj' = 0;

Ibeforeendget >= 1 ->
	Ibeforeendget' = Ibeforeendget - 1,
	Iendget' = Iendget + 1,
	busycounteq0' = busycounteq0 + notbusycounteq0 + 0,
	notbusycounteq0' =  0;

Iendget >= 1 , lock >= 1 ->
	Iendget' = Iendget - 1,
	Iwork' = Iwork + 1,
	lock' = lock - 1,
	unlock' = unlock + 1;

Iwork >= 1 , unlock >= 1 ->
	Iwork' = Iwork - 1,
	Iif2' = Iif2 + 1,
	unlock' = unlock - 1,
	lock' = lock + 1;

Iif2 >= 1 , notflageqmei >= 1, lock >= 1 ->
	Iif2' = Iif2 - 1,
	Ierror' = Ierror + 1,
	lock' = lock - 1,
	unlock' = unlock + 1;

Iif2 >= 1 , flageqmei >= 1 ->
	Iif2' = Iif2 - 1,
	Iif3' = Iif3 + 1;

Iif3 >= 1 , busycounteq0 >= 1 ->
	Iif3' = Iif3 - 1,
	Ichoice4' = Ichoice4 + 1;

Iif3 >= 1 , notbusycounteq0 >= 1 ->
	Iif3' = Iif3 - 1,
	Ichoice5' = Ichoice5 + 1;

Ichoice4 >= 1 ->
	Ichoice4' = Ichoice4 - 1,
	Inotifyall' = Inotifyall + 1,
	notwaitersi' = notwaitersi + waitersi + 0,
	waitersi' = 0,
	notwaitersj' = notwaitersj + waitersj + 0,
        waitersj' = 0;

Ichoice4 >= 1 ->
        Ichoice4' = Ichoice4 - 1,
        Inotifyall' = Inotifyall + 1,
        waitersi' = notwaitersi + waitersi + 0,
        notwaitersi' = 0,
        notwaitersj' = notwaitersj + waitersj + 0,
        waitersj' = 0;	

Ichoice4 >= 1 ->
        Ichoice4' = Ichoice4 - 1,
        Inotifyall' = Inotifyall + 1,
        notwaitersi' = notwaitersi + waitersi + 0,
        waitersi' = 0,
        waitersj' = notwaitersj + waitersj + 0,
        notwaitersj' = 0;

Inotifyall >= 1 ->
	Inotifyall' = Inotifyall -1,
	Iassign' = Iassign + 1,
	Iafterwait' = Iafterwait + Iwait + 0,
	Iwait' = 0,
	Jafterwait' =Jafterwait + Jwait + 0,
	Jwait' = 0,
	Oafterwait' = Oafterwait + Owait + 0,
	Owait' = 0;

Iassign >= 1 ->
	Iassign' = Iassign - 1,
	Iendfree' = Iendfree + 1,
	notflageqi' = notflageqi + flageqi + 0,
	flageqi' = 0,
	notflageqj' = notflageqj + flageqj + 0,
        flageqj' = 0,
	notflageqmei' = notflageqmei + flageqmei + 0,
        flageqmei' = 0,
	notflageqmej' = notflageqmej + flageqmej + 0,
        flageqmej' = 0;

Ichoice5 >= 1 , busycounteq0 >= 1 ->
	Ichoice5' = Ichoice5 - 1,
	Iendfree' = Iendfree + 1,
	busycounteq0' = busycounteq0 - 1,
	notbusycounteq0' = notbusycounteq0 + 1;

Ichoice5 >= 1 , notbusycounteq0 >= 1 ->
        Ichoice5' = Ichoice5 - 1,
        Iendfree' = Iendfree + 1;

Ichoice5 >= 1 , notbusycounteq0 >= 1 ->
        Ichoice5' = Ichoice5 - 1,
        Iendfree' = Iendfree + 1 ,
	busycounteq0' = busycounteq0 + 1,
        notbusycounteq0' = notbusycounteq0 - 1;

Iendfree >= 1, lock >= 1 -> 
	Iendfree' = Iendfree  -1,
	Iwhile' = Iwhile + 1,
	lock' = lock - 1,
	unlock' = unlock + 1;



#process J

Jwhile >= 1 ->
	Jwhile' = Jwhile - 1,
	Jgetbusyflag' = Jgetbusyflag + 1;

Jgetbusyflag >= 1 , unlock >= 1 ->
	Jgetbusyflag' = Jgetbusyflag - 1,
	Jchoice1' = Jchoice1 + 1,
	unlock' = unlock - 1,
	lock' = lock + 1;

Jchoice1 >= 1 , flageqj >= 1 ->
	Jchoice1' = Jchoice1 - 1,
	Jif1' = Jif1 + 1,
	flageqmej' = flageqmej + notflageqmej + 0,
	notflageqmej' = 0;

Jchoice1 >= 1 , notflageqj >= 1 ->
        Jchoice1' = Jchoice1 - 1,
        Jif1' = Jif1 + 1,
        notflageqmej' = flageqmej + notflageqmej + 0,
        flageqmej' = 0;

Jif1 >= 1 , flageqmej >= 1 ->
	Jif1' = Jif1 - 1,
	Jchoice2' = Jchoice2 + 1;

Jif1 >= 1 , notflageqmej >= 1 ->
	Jif1' = Jif1 - 1,
	Jchoice3' = Jchoice3 + 1;

Jchoice2 >= 1 , busycounteq0 >= 1 ->
	Jchoice2' = Jchoice2 - 1,
	Jgoto1' = Jgoto1 + 1,
	notbusycounteq0' = notbusycounteq0 + 1,
	busycounteq0' = busycounteq0 - 1; 

Jchoice2 >= 1 , notbusycounteq0 >= 1 ->
	Jchoice2' = Jchoice2 - 1,
	Jgoto1' = Jgoto1 + 1;


Jchoice2 >= 1 , notbusycounteq0 >= 1 ->
        Jchoice2' = Jchoice2 - 1,
        Jgoto1' = Jgoto1 + 1,
	notbusycounteq0' = notbusycounteq0 - 1,
	busycounteq0' = busycounteq0 + 1;

Jgoto1 >= 1 ->
	Jgoto1' = Jgoto1 - 1,
	Jendget' = Jendget + 1;

Jchoice3 >= 1 , waitersi >= 1 ->
	Jchoice3' = Jchoice3 - 1,
	Jwhile2' = Jwhile2 + 1;

Jchoice3 >= 1 , waitersj >= 1 ->
        Jchoice3' = Jchoice3 - 1,
        Jwhile2' = Jwhile2 + 1;

Jchoice3 >= 1 , notwaitersi >= 1 , notwaitersj >= 1 ->
        Jchoice3' = Jchoice3 - 1,
        Jwhile2' = Jwhile2 + 1;


Jchoice3 >= 1 , notwaitersi >= 1 , notwaitersj >= 1 ->
        Jchoice3' = Jchoice3 - 1,
        Jwhile2' = Jwhile2 + 1,
	notwaitersj' = notwaitersj - 1,
	waitersj' = waitersj + 1;

Jwhile2 >= 1 , notwaitersj >= 1 , lock >= 1 ->
	Jwhile2' = Jwhile2 - 1,
	Jwait' = Jwait + 1,
	lock' = lock - 1,
	unlock' = unlock + 1;

Jwhile2 >= 1 , waitersj >= 1 ->
	Jwhile2' = Jwhile2 - 1,
	Jafterwhile' = Jafterwhile + 1;

Jafterwait >= 1 , unlock >= 1 ->
	Jafterwait' = Jafterwait - 1,
	Jwhile2' = Jwhile2 + 1,
	unlock' = unlock - 1,
	lock' = lock + 1;
	
Jafterwhile >= 1 ->
	Jafterwhile' = Jafterwhile - 1,
	Jbeforeendget' = Jbeforeendget + 1,
	flageqmej' =  flageqmej + notflageqmej + 0,
	notflageqmej' = 0,
	notflageqmei' =  flageqmei + notflageqmei + 0,
	flageqmei' = 0,
	flageqj' = flageqj + notflageqj + 0,
	notflageqj' = 0,
	notflageqi' = notflageqi + flageqi + 0,
	flageqi' = 0;

Jbeforeendget >= 1 ->
	Jbeforeendget' = Jbeforeendget - 1,
	Jendget' = Jendget + 1,
	busycounteq0' = busycounteq0 + notbusycounteq0 + 0,
	notbusycounteq0' =  0;

Jendget >= 1 , lock >= 1 ->
	Jendget' = Jendget - 1,
	Jwork' = Jwork + 1,
	lock' = lock - 1,
	unlock' = unlock + 1;

Jwork >= 1 , unlock >= 1 ->
	Jwork' = Jwork - 1,
	Jif2' = Jif2 + 1,
	unlock' = unlock - 1,
	lock' = lock + 1;

Jif2 >= 1 , notflageqmej >= 1, lock >= 1 ->
	Jif2' = Jif2 - 1,
	Jerror' = Jerror + 1,
	lock' = lock - 1,
	unlock' = unlock + 1;

Jif2 >= 1 , flageqmej >= 1 ->
	Jif2' = Jif2 - 1,
	Jif3' = Jif3 + 1;

Jif3 >= 1 , busycounteq0 >= 1 ->
	Jif3' = Jif3 - 1,
	Jchoice4' = Jchoice4 + 1;

Jif3 >= 1 , notbusycounteq0 >= 1 ->
	Jif3' = Jif3 - 1,
	Jchoice5' = Jchoice5 + 1;

Jchoice4 >= 1 ->
	Jchoice4' = Jchoice4 - 1,
	Jnotifyall' = Jnotifyall + 1,
	notwaitersi' = notwaitersi + waitersi + 0,
	waitersi' = 0,
	notwaitersj' = notwaitersj + waitersj + 0,
        waitersj' = 0;

Jchoice4 >= 1 ->
        Jchoice4' = Jchoice4 - 1,
        Jnotifyall' = Jnotifyall + 1,
        waitersi' = notwaitersi + waitersi + 0,
        notwaitersi' = 0,
        notwaitersj' = notwaitersj + waitersj + 0,
        waitersj' = 0;	

Jchoice4 >= 1 ->
        Jchoice4' = Jchoice4 - 1,
        Jnotifyall' = Jnotifyall + 1,
        notwaitersi' = notwaitersi + waitersi + 0,
        waitersi' = 0,
        waitersj' = notwaitersj + waitersj + 0,
        notwaitersj' = 0;

Jnotifyall >= 1 ->
	Jnotifyall' = Jnotifyall -1,
	Jassign' = Jassign + 1,
	Iafterwait' = Iafterwait + Iwait + 0,
	Iwait' = 0,
	Jafterwait' =Jafterwait + Jwait + 0,
	Jwait' = 0,
	Oafterwait' = Oafterwait + Owait + 0,
	Owait' = 0;

Jassign >= 1 ->
	Jassign' = Jassign - 1,
	Jendfree' = Jendfree + 1,
	notflageqi' = notflageqi + flageqi + 0,
	flageqi' = 0,
	notflageqj' = notflageqj + flageqj + 0,
        flageqj' = 0,
	notflageqmei' = notflageqmei + flageqmei + 0,
        flageqmei' = 0,
	notflageqmej' = notflageqmej + flageqmej + 0,
        flageqmej' = 0;

Jchoice5 >= 1 , busycounteq0 >= 1 ->
	Jchoice5' = Jchoice5 - 1,
	Jendfree' = Jendfree + 1,
	busycounteq0' = busycounteq0 - 1,
	notbusycounteq0' = notbusycounteq0 + 1;

Jchoice5 >= 1 , notbusycounteq0 >= 1 ->
        Jchoice5' = Jchoice5 - 1,
        Jendfree' = Jendfree + 1;

Jchoice5 >= 1 , notbusycounteq0 >= 1 ->
        Jchoice5' = Jchoice5 - 1,
        Jendfree' = Jendfree + 1 ,
	busycounteq0' = busycounteq0 + 1,
        notbusycounteq0' = notbusycounteq0 - 1;

Jendfree >= 1, lock >= 1 -> 
	Jendfree' = Jendfree  -1,
	Jwhile' = Jwhile + 1,
	lock' = lock - 1,
	unlock' = unlock + 1;



#process O

Owhile >= 1 ->
	Owhile' = Owhile - 1,
	Ogetbusyflag' = Ogetbusyflag + 1;

Ogetbusyflag >= 1 , unlock >= 1 ->
	Ogetbusyflag' = Ogetbusyflag - 1,
	Ochoice1' = Ochoice1 + 1,
	unlock' = unlock - 1,
	lock' = lock + 1;

Ochoice1 >= 1 ->
	Ochoice1' = Ochoice1 - 1,
	Oif1' = Oif1 + 1;

Oif1 >= 1 , notflageqmej >= 1  , notflageqmei >= 1 ->
	Oif1' = Oif1 - 1,
	Ochoice2' = Ochoice2 + 1;

Oif1 >= 1 , flageqmej >= 1 ->
	Oif1' = Oif1 - 1,
	Ochoice3' = Ochoice3 + 1;

Oif1 >= 1 , flageqmei >= 1 ->
	Oif1' = Oif1 - 1,
	Ochoice3' = Ochoice3 + 1;

Ochoice2 >= 1 , busycounteq0 >= 1 ->
	Ochoice2' = Ochoice2 - 1,
	Ogoto1' = Ogoto1 + 1,
	notbusycounteq0' = notbusycounteq0 + 1,
	busycounteq0' = busycounteq0 - 1; 

Ochoice2 >= 1 , notbusycounteq0 >= 1 ->
	Ochoice2' = Ochoice2 - 1,
	Ogoto1' = Ogoto1 + 1;


Ochoice2 >= 1 , notbusycounteq0 >= 1 ->
        Ochoice2' = Ochoice2 - 1,
        Ogoto1' = Ogoto1 + 1,
	notbusycounteq0' = notbusycounteq0 - 1,
	busycounteq0' = busycounteq0 + 1;

Ogoto1 >= 1 ->
	Ogoto1' = Ogoto1 - 1,
	Oendget' = Oendget + 1;

Ochoice3 >= 1 ->
	Ochoice3' = Ochoice3 - 1,
	Owhile2' = Owhile2 + 1;

Ochoice3 >= 1 , waitersj >= 1 ->
        Ochoice3' = Ochoice3 - 1,
        Owhile2' = Owhile2 + 1;


Owhile2 >= 1 , waitersj >= 1 , lock >= 1 ->
	Owhile2' = Owhile2 - 1,
	Owait' = Owait + 1,
	lock' = lock - 1,
	unlock' = unlock + 1;

Owhile2 >= 1 , waitersi >= 1 , lock >= 1 ->
	Owhile2' = Owhile2 - 1,
	Owait' = Owait + 1,
	lock' = lock - 1,
	unlock' = unlock + 1;


Owhile2 >= 1 , notwaitersi >= 1 , notwaitersj >= 1 , lock >= 1 ->
	Owhile2' = Owhile2 - 1,
	Owait' = Owait + 1,
	lock' = lock - 1,
	unlock' = unlock + 1;


Owhile2 >= 1 , notwaitersj >= 1  , notwaitersi >= 1 ->
	Owhile2' = Owhile2 - 1,
	Oafterwhile' = Oafterwhile + 1;

Oafterwait >= 1 , unlock >= 1 ->
	Oafterwait' = Oafterwait - 1,
	Owhile2' = Owhile2 + 1,
	unlock' = unlock - 1,
	lock' = lock + 1;
	
Oafterwhile >= 1 ->
	Oafterwhile' = Oafterwhile - 1,
	Obeforeendget' = Obeforeendget + 1,
	notflageqmej' =  flageqmej + notflageqmej + 0,
	flageqmej' = 0,
        notflageqmei' =  flageqmei + notflageqmei + 0,
	flageqmei' = 0,
	notflageqj' = flageqj + notflageqj + 0,
	flageqj' = 0,
	notflageqi' = flageqi + notflageqi + 0,
	flageqi' =  0;

Obeforeendget >= 1 ->
	Obeforeendget' = Obeforeendget - 1,
	Oendget' = Oendget + 1,
	busycounteq0' = busycounteq0 + notbusycounteq0 + 0,
	notbusycounteq0' =  0;

Oendget >= 1 , lock >= 1 ->
	Oendget' = Oendget - 1,
	Owork' = Owork + 1,
	lock' = lock - 1,
	unlock' = unlock + 1;

Owork >= 1 , unlock >= 1 ->
	Owork' = Owork - 1,
	Oif2' = Oif2 + 1,
	unlock' = unlock - 1,
	lock' = lock + 1;

Oif2 >= 1 , flageqmej >= 1, lock >= 1 ->
	Oif2' = Oif2 - 1,
	Oerror' = Oerror + 1,
	lock' = lock - 1,
	unlock' = unlock + 1;

Oif2 >= 1 , flageqmei >= 1 , lock >= 1 ->
	Oif2' = Oif2 - 1,
	Oerror' = Oerror + 1,
	lock' = lock - 1,
	unlock' = unlock + 1;

Oif2 >= 1 , notflageqmej >= 1 , notflageqmei >= 1 , lock >= 1 ->
	Oif2' = Oif2 - 1,
	Oerror' = Oerror + 1,
	lock' = lock - 1,
	unlock' = unlock + 1;

Oif2 >= 1 , notflageqmej >= 1 , notflageqmei >= 1 ->
	Oif2' = Oif2 - 1,
	Oif3' = Oif3 + 1;

Oif3 >= 1 , busycounteq0 >= 1 ->
	Oif3' = Oif3 - 1,
	Ochoice4' = Ochoice4 + 1;

Oif3 >= 1 , notbusycounteq0 >= 1 ->
	Oif3' = Oif3 - 1,
	Ochoice5' = Ochoice5 + 1;

Ochoice4 >= 1 ->
	Ochoice4' = Ochoice4 - 1,
	Onotifyall' = Onotifyall + 1,
	notwaitersi' = notwaitersi + waitersi + 0,
	waitersi' = 0,
	notwaitersj' = notwaitersj + waitersj + 0,
        waitersj' = 0;

Ochoice4 >= 1 ->
        Ochoice4' = Ochoice4 - 1,
        Onotifyall' = Onotifyall + 1,
        waitersi' = notwaitersi + waitersi + 0,
        notwaitersi' = 0,
        notwaitersj' = notwaitersj + waitersj + 0,
        waitersj' = 0;	

Ochoice4 >= 1 ->
        Ochoice4' = Ochoice4 - 1,
        Onotifyall' = Onotifyall + 1,
        notwaitersi' = notwaitersi + waitersi + 0,
        waitersi' = 0,
        waitersj' = notwaitersj + waitersj + 0,
        notwaitersj' = 0;

Onotifyall >= 1 ->
	Onotifyall' = Onotifyall -1,
	Oassign' = Oassign + 1,
	Iafterwait' = Iafterwait + Iwait + 0,
	Iwait' = 0,
	Jafterwait' = Jafterwait + Jwait + 0,
	Jwait' = 0,
	Oafterwait' = Oafterwait + Owait + 0,
	Owait' = 0;

Oassign >= 1 ->
	Oassign' = Oassign - 1,
	Oendfree' = Oendfree + 1,
	notflageqi' = notflageqi + flageqi + 0,
	flageqi' = 0,
	notflageqj' = notflageqj + flageqj + 0,
        flageqj' = 0,
	notflageqmei' = notflageqmei + flageqmei + 0,
        flageqmei' = 0,
	notflageqmej' = notflageqmej + flageqmej + 0,
        flageqmej' = 0;

Ochoice5 >= 1 , busycounteq0 >= 1 ->
	Ochoice5' = Ochoice5 - 1,
	Oendfree' = Oendfree + 1,
	busycounteq0' = busycounteq0 - 1,
	notbusycounteq0' = notbusycounteq0 + 1;

Ochoice5 >= 1 , notbusycounteq0 >= 1 ->
        Ochoice5' = Ochoice5 - 1,
        Oendfree' = Oendfree + 1;

Ochoice5 >= 1 , notbusycounteq0 >= 1 ->
        Ochoice5' = Ochoice5 - 1,
        Oendfree' = Oendfree + 1 ,
	busycounteq0' = busycounteq0 + 1,
        notbusycounteq0' = notbusycounteq0 - 1;

Oendfree >= 1, lock >= 1 -> 
	Oendfree' = Oendfree  -1,
	Owhile' = Owhile + 1,
	lock' = lock - 1,
	unlock' = unlock + 1;



init

unlock  = 1 , lock = 0 , flageqi = 0 , notflageqi = 1 , flageqmei =0 , notflageqmei = 1 , flageqj = 0 , notflageqj = 1 , flageqmej = 0 , notflageqmej = 1 , busycounteq0 = 1 , notbusycounteq0 = 0 , waitersi = 0 , notwaitersi = 1 , waitersj = 0 , notwaitersj = 1 ,
Iwhile = 1 , Igetbusyflag = 0 , Ichoice1 = 0 , Iif1 = 0 , Ichoice2 = 0 , Igoto1 = 0 , Ichoice3 = 0 , Iwhile2 = 0 , Iwait = 0 , Iafterwait = 0 , Iafterwhile = 0 , Ibeforeendget = 0 ,
Iendget = 0 , Iwork = 0 , Iif2 = 0 , Iif3 = 0 , Ichoice4 = 0 , Inotifyall = 0 , Iassign = 0 , Ichoice5 = 0 , Iendfree = 0 , Ierror = 0,
Jwhile = 1 , Jgetbusyflag = 0 , Jchoice1 = 0 , Jif1 = 0 , Jchoice2 = 0 , Jgoto1 = 0 , Jchoice3 = 0 , Jwhile2  = 0 , Jwait = 0 , Jafterwait = 0 , Jafterwhile = 0 , Jbeforeendget = 0 , Jendget = 0 ,
Jwork = 0 , Jif2 = 0 , Jif3 = 0 , Jchoice4 = 0 , Jnotifyall = 0 , Jassign = 0 , Jchoice5 = 0 , Jendfree = 0 , Jerror = 0,
Owhile >= 0, Ogetbusyflag = 0 , Ochoice1 = 0 , Oif1 = 0  , Ochoice2 = 0 , Ogoto1 = 0 , Ochoice3 = 0 , Owhile2 = 0 , Owait = 0 , Oafterwait = 0 , Oafterwhile = 0 , Obeforeendget = 0 , Oendget = 0 , Owork = 0 ,
Oif2 = 0 , Oif3  = 0 , Ochoice4 = 0 , Onotifyall  = 0 , Oassign = 0 , Ochoice5 = 0 , Oendfree = 0 , Oerror = 0

target
Iwork >= 1 , Jwork >= 1

invariants

unlock = 1, lock = 1
flageqi = 1, notflageqi = 1
flageqmei = 1, notflageqmei = 1
flageqj = 1, notflageqj = 1
flageqmej = 1, notflageqmej = 1
busycounteq0 = 1, notbusycounteq0 = 1
waitersi = 1, notwaitersi  = 1
waitersj = 1, notwaitersj = 1
Iwhile = 1, Igetbusyflag = 1, Ichoice1 = 1, Iif1 = 1, Ichoice2 = 1, Igoto1 = 1, Ichoice3 = 1, Iwhile2 = 1, Iwait = 1, Iafterwait = 1, Ibeforeendget = 1, Iendget = 1, Iwork = 1, Iif2 = 1, Iif3 = 1, Ichoice4 =1, Inotifyall = 1, Iassign = 1, Ichoice5 = 1, Iendfree = 1, Ierror = 1
Jwhile = 1, Jgetbusyflag = 1, Jchoice1 = 1, Jif1 = 1, Jchoice2 = 1, Jgoto1 = 1, Jchoice3 = 1, Jwhile2 = 1, Jwait = 1, Jafterwait = 1, Jbeforeendget = 1, Jendget = 1, Jwork = 1, Jif2 = 1, Jif3 = 1, Jchoice4 =1, Jnotifyall = 1, Jassign = 1, Jchoice5 = 1, Jendfree = 1, Jerror = 1
unlock = 1,Ichoice1 = 1, Iif1 = 1, Ichoice2 = 1, Igoto1 = 1, Ichoice3 = 1, Iwhile2 = 1, Ibeforeendget = 1, Iendget = 1, Iif2 = 1, Iif3 = 1, Ichoice4 = 1, Inotifyall = 1, Iassign = 1, Ichoice5 = 1, Iendfree = 1, Jchoice1 = 1, Jif1 = 1, Jchoice2 = 1, Jgoto1 = 1, Jchoice3 = 1, Jwhile2 = 1, Jbeforeendget = 1, Jendget = 1, Jif2 = 1, Jif3 = 1, Jchoice4 = 1, Jnotifyall = 1, Jassign = 1, Jchoice5 = 1, Jendfree = 1, Ochoice1 = 1, Oif1 = 1, Ochoice2 = 1, Ogoto1 = 1, Ochoice3 = 1, Owhile2 = 1, Obeforeendget = 1, Oendget = 1, Oif2 = 1, Oif3 = 1, Ochoice4 = 1, Onotifyall = 1, Oassign = 1, Ochoice5 = 1, Oendfree = 1
