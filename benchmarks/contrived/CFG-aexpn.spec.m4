dnl usage m4 -DDEPTH=Y
dnl it corresponds to the PN for the CFG generating 1 parse tree that is a full binary tree of depth DEPTH
dnl the invariant shows tree surgery (http://dx.doi.org/10.1016/j.ipl.2011.03.019) is crucial for scalability of the backward search
# This file was actually generated from a m4 template file called CFG-aexpn.spec.m4
vars
dnl
define(`varbetween', `X$1 ifelse(eval(incr($1) < $2), 1, `varbetween(incr($1),$2)', `X$2 dummy')')dnl
define(`varupto', `varbetween(1,$1)')dnl
  varupto(DEPTH) dnl

rules
dnl
changequote([,])dnl
define([QUOTE],['])dnl
changequote dnl
define(`towerbetween', `X$1 >=1 -> X$1`'QUOTE=X$1-1, X`'incr($1)`'QUOTE=X`'incr($1)+2;
 ifelse(eval(incr($1) < $2), 1, `towerbetween(incr($1),$2)', `')')dnl
define(`towerupto', `towerbetween(1,$1)')dnl
towerupto(DEPTH)

init
dnl
  X1=1, dnl
define(`initbetween', `X$1=0, ifelse(eval(incr($1) < $2), 1, `initbetween(incr($1),$2)', `X$2=0, dummy=eval(decr(DEPTH))')')dnl
define(`initupto', `initbetween(2,$1)')dnl
initupto(DEPTH)

  
target
define(`targetbetween', `X$1=0, ifelse(eval(incr($1) < $2), 1, `targetbetween(incr($1),$2)', `X$2>=1')')dnl
define(`targetupto', `targetbetween(1,$1)')dnl
targetupto(DEPTH)

invariants
define(`invariantbetween', `X$1=1, ifelse(eval(incr($1) < $2), 1, `invariantbetween(incr($1),$2)', `X$2=1, dummy=1')')dnl
define(`invariantupto', `invariantbetween(1,$1)')dnl
invariantupto(DEPTH)
