dnl usage m4 -DIN=X -DTRESHOLD=Y
dnl this implements the treshold predicate for population protocols using the natural PN encoding 
# This file was actually generated from a m4 template file called pp-treshold-PN.spec.m4
vars
dnl
define(`varbetween', `X$1 ifelse(eval(incr($1) < $2), 1, `varbetween(incr($1),$2)', `X$2')')dnl
define(`varupto', `varbetween(1,$1)')dnl
  varupto(TRESHOLD)

rules
dnl
changequote([,])dnl
define([QUOTE],['])dnl
changequote dnl
define(`towerbetween', `X$1 >=2 -> X$1`'QUOTE=X$1-1, X`'incr($1)`'QUOTE=X`'incr($1)+1;
 ifelse(eval(incr($1) < $2), 1, `towerbetween(incr($1),$2)', `')')dnl
define(`towerupto', `towerbetween(1,$1)')dnl
towerupto(TRESHOLD)

define(`contaminatebetween', `X$2 >=1, X$1 >=1 -> X$1`'QUOTE=X$1-1, X$2`'QUOTE=X`'$2+1;
 ifelse(eval(incr($1) < $2), 1, `contaminatebetween(incr($1),$2)', `')')dnl
define(`contaminateupto', `contaminatebetween(1,$1)')dnl
contaminateupto(TRESHOLD)

init
dnl
  X1=IN, dnl
define(`initbetween', `X$1=0, ifelse(eval(incr($1) < $2), 1, `initbetween(incr($1),$2)', `X$2=0')')dnl
define(`initupto', `initbetween(2,$1)')dnl
initupto(TRESHOLD)
  
target
define(`targetbetween', `X$1=0, ifelse(eval(incr($1) < $2), 1, `targetbetween(incr($1),$2)', `X$2>=1')')dnl
define(`targetupto', `targetbetween(1,$1)')dnl
targetupto(TRESHOLD)

invariants
define(`invariantbetween', `X$1=1, ifelse(eval(incr($1) < $2), 1, `invariantbetween(incr($1),$2)', `X$2=1')')dnl
define(`invariantupto', `invariantbetween(1,$1)')dnl
invariantupto(TRESHOLD)
