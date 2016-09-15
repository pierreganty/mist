dnl usage m4 -DPARAM=Y should be at least 2
# This file was actually generated from a m4 template file called ME-k-bingham.spec.m4
vars
dnl
Xin Xnotin X0 X1 dnl
define(`varbetween', `X$1 ifelse(eval(incr($1) < $2), 1, `varbetween(incr($1),$2)', `X$2')')dnl
define(`varupto', `varbetween(2,$1)')dnl
  varupto(PARAM)

rules

Xnotin >=1, X0 >= 1 -> Xnotin'=Xnotin-1, X0'=X0-1, Xin'=Xin+1, X1'=X1+1; #a
Xnotin >=1, X1 >= 1 -> Xnotin'=Xnotin-1, X1'=X1-1, Xin'=Xin+1, X0'=X0+1; #b
changequote([,])dnl
define([QUOTE],['])dnl
changequote dnl
define(`circulatebetween', `X$1 >=1 -> X$1`'QUOTE=X$1-1, X`'incr($1)`'QUOTE=X`'incr($1)+1;
 ifelse(eval(incr($1) < $2), 1, `circulatebetween(incr($1),$2)', `')')dnl
define(`circulateupto', `circulatebetween(1,$1)')dnl
circulateupto(PARAM)

define(`leavebetween', `Xin >=1, X$1 >=1 -> Xin`'QUOTE=Xin-1, X$1`'QUOTE=X$1-1, X0`'QUOTE=X0+1, Xnotin`'QUOTE=Xnotin+1;
 ifelse(eval($1 < $2), 1, `leavebetween(incr($1),$2)', `')')dnl
define(`leaveupto', `leavebetween(1,$1)')dnl
leaveupto(PARAM)

init
dnl
  Xin=0, Xnotin=1, X0>=1, X1=0, dnl
define(`initbetween', `X$1=0, ifelse(eval(incr($1) < $2), 1, `initbetween(incr($1),$2)', `X$2=0')')dnl
define(`initupto', `initbetween(2,$1)')dnl
initupto(PARAM)
  
target
  dnl Xin>=0, Xnotin>=0, X0>=0, X1>=0, 
  X`'PARAM >= 2
dnl define(`targetbetween', `X$1>=0, ifelse(eval(incr($1) < $2), 1, `targetbetween(incr($1),$2)', `X$2>=2')')dnl
dnl define(`targetupto', `targetbetween(2,$1)')dnl
dnl targetupto(PARAM)

dnl invariants
dnl define(`invariantbetween', `X$1=1, ifelse(eval(incr($1) < $2), 1, `invariantbetween(incr($1),$2)', `X$2=1')')dnl
dnl define(`invariantupto', `invariantbetween(1,$1)')dnl
dnl invariantupto(PARAM)
