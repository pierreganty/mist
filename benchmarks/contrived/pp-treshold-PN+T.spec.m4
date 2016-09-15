dnl usage m4 -DTRESHOLD=Y
dnl this implements the treshold predicate for population protocols using a sophisticated encoding based on PN ans transfers 
# This file was actually generated from a m4 template called pp-treshold-PN+T.spec.m4
vars
dnl
define(`varbetween', `X$1 Y$1 ifelse(eval(incr($1) < $2), 1, `varbetween(incr($1),$2)', `X$2')')dnl
define(`varupto', `varbetween(1,$1)')dnl
  varupto(TRESHOLD)

rules
# First, the widgets to have only transitions of the form (q1,q2) → (q1',q2) where q1 ≠ q2.
# We transfer one agent at a time from X_ to Y_ but from Y_ to X_ we transfer all them back.
dnl
changequote([,])dnl
define([QUOTE],['])dnl
changequote dnl
define(`widgetbetween', `X$1 >=1 -> X$1`'QUOTE=X$1-1, Y$1`'QUOTE=Y$1+1;
Y$1 >= 1 -> X$1`'QUOTE=X$1+Y$1, Y$1`'QUOTE=0;
 ifelse(eval(incr($1) < $2), 1, `widgetbetween(incr($1),$2)', `')')dnl
define(`widgetupto', `widgetbetween(1,$1)')dnl
widgetupto(TRESHOLD)

# For the tower we use transfers. This is ok only because this is the treshold predicate
define(`towerbetween', `Y$1 >=1, X$1 >=1 -> X$1`'QUOTE=0, X`'incr($1)`'QUOTE=X`'incr($1)+X$1;
 ifelse(eval(incr($1) < $2), 1, `towerbetween(incr($1),$2)', `')')dnl
define(`towerupto', `towerbetween(1,$1)')dnl
towerupto(TRESHOLD)

# Once the treshold is reached, contaminate
define(`contaminatebetween', `X$2 >=1, X$1 >=1 -> X$1`'QUOTE=0, X$2`'QUOTE=X`'$2+X$1;
 ifelse(eval(incr($1) < $2), 1, `contaminatebetween(incr($1),$2)', `')')dnl
define(`contaminateupto', `contaminatebetween(1,$1)')dnl
contaminateupto(TRESHOLD)

init
dnl
define(`initbetween', `X$1=0, Y$1=0, ifelse(eval(incr($1) < $2), 1, `initbetween(incr($1),$2)', `X$2=0')')dnl
define(`initupto', `initbetween(1,$1)')dnl
initupto(TRESHOLD)

target
define(`targetbetween', `X$1=0, Y$1=0, ifelse(eval(incr($1) < $2), 1, `targetbetween(incr($1),$2)', `X$2>=1')')dnl
define(`targetupto', `targetbetween(1,$1)')dnl
targetupto(TRESHOLD)

#invariants
#  X1=1, Y1=1, X2=1, Y2=1, X3=1
