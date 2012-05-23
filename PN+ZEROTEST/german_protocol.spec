#German's protocol with zero-test

vars 
idle serveS grantS serveE grantE null waitE exclusive waitS shared ex
#^--------LOCATIONS------------^ ^---------VARIABLES----------------^

rules 
#t1
idle>=1,null>=1 -> 
null'=null-1,
waitS'=waitS+1,
idle'=idle-1,
serveS'=serveS+1;

#t2
idle>=1,null>=1 ->
null'=null-1,
waitE'=waitE+1,
idle'=idle-1,
serveE'=serveE+1;

#t3
idle>=1,shared>=1 ->
shared'=shared-1,
waitE'=waitE+1,
idle'=idle-1,
serveE'=serveE+1;

#t4
serveS>=1,ex=1,exclusive>=1 ->
exclusive'=exclusive-1,
null'=null+1,
ex'=0,
serveS'=serveS-1,
grantS'=grantS+1;

#t5
serveS>=1,ex=0 ->
serveS'=serveS-1,
grantS'=grantS+1;


#t6
grantS>=1,waitS>=1 ->
waitS'=waitS-1,
shared'=shared+1,
grantS'=grantS-1,
idle'=idle+1;

#t7
serveE>=1,shared>=1 ->
null'=null+1,
shared'=shared-1;

#t8
serveE>=1,exclusive>=1 ->
null'=null+1,
exclusive'=exclusive-1,
ex'=0;

#t9
serveE>=1,exclusive=0,shared=0 ->
serveE'=serveE-1,
grantE'=grantE+1;

#t8
grantE>=1,waitE>=1 ->
waitE'=waitE-1,
exclusive'=exclusive+1,
ex'=1,
grantE'=grantE-1,
idle'=idle+1;


init
null>=1,exclusive=0,shared=0,waitS=0,waitE=0,ex=0,idle=1,serveS=0,serveE=0,grantS=0,grantE=0

target
shared>=1,exclusive>=1
exclusive>=2

invariants
idle=1,serveS=1,grantS=1,serveE=1,grantE=1
