# Notes

The examples in the directory are interesting but **contrived**.
So please mention that when you report on empirical results.
The files with extension `.m4` have to be preprocessed by `m4` first.
They are parameterized Petri nets (or extension).
Look at the top of each file for precise instructions.

The `ME-k-bingham.spec.m4` are described in [“A New Approach to Upward-Closed Set Backward Reachability Analysis”](http://www.cs.ubc.ca/~jbingham/jbingham-infinity2004.pdf) by Jesse D. Bingham, INFINITY'04.
Mist with `--backward` does not perform well on these because there are too many places.
Mist with `--ic4pn` does not perform better because it has to refine until almost all places are separated.

On a positive note, `ME-250-bigtarget.spec` has a different target. 
On this example, mist with `--ic4pn` terminates with 10 variables.
