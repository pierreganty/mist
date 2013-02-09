# mist #

---

Safety checker for Petri Nets (and some extensions). Find more details (e.g.
the input format) on <http://software.imdea.org/~pierreganty/ist.html>.

## Compilation ##

To compile: `./configure` followed by `make`

You can optionally install the package using: `make install`

To uninstall: `make uninstall`


## Name ##

`mist`, safety checker for Petri Nets and extensions.

## Synopsis ##

`mist [options] <input_file>`

### Options ###
 
`--help`       |this help 
`--version`    |show version numbers 
`--backward`   |the backward algorithm with invariant pruning 
`--ic4pn`      |the algorithm described in FI 
`--tsi`        |the algorithm described in TSI 
`--eec`        |the Expand, Enlarge and Check algorithm 

## Context ##

mist provides a set of functions together with a data structure (IST that
stands for Interval Sharing Tree) to manipulate interval constraints in a
multi-dimensional space.  Additionnaly to the traditional operations on sets,
some operations are specialized to verification of MTNs (an extension of the
Petri Net formalism). MTN is a infinite state model for finite abstraction of
Java multi-threaded programs. In fact, each thread can be viewed as a finite
automata for which an abitrary number of instances can be considered.

## Further Questions ##

* Details about the [algorithms and input format][inputformat].
* Contact the author, [Pierre Ganty][email].

[inputformat]:http://software.imdea.org/people/pierre.ganty/index.html
[email]:mailto:pierreganty@gmail.com
