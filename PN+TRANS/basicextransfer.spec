#little example of the second page of the paper [DEP99]

#variable
vars
  think wait use

#transitions
rules
think >= 1 ->
	use' = use + 1,
	wait' = wait + think - 1,
	think' = 0;

use >= 1 ->
	use' = use - 1,
	think' = think + wait + 1,
	wait' = 0;

#initial configuration
init
  think >= 1, wait = 0, use = 0	

target
  use >= 2

invariants
	think=1, wait=1, use=1
