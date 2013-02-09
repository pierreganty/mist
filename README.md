mist
====

Safety checker for Petri Nets (and some extensions)

% last updated: Sat Feb  9 20:53:10 CET 2013

COMPILATION
-----------

To compile:
   './configure'
   'make'

You can optionally install the package (in /usr/local/bin) with
root privileges:
   'make install'

To uninstall:
   'make uninstall'


EXECUTION
---------

The runnable file are in 'src/':
   'src/mist'
   or 'mist' if you installed the package.

Running:
   'mist --<algorithm> <input_file>


CONTEXT
-------

mist provides a set of functions together with a data structure (IST that
stands for Interval Sharing Tree) to manipulate interval constraints in a
multi-dimensional space.  Additionnaly to the traditional operations on sets,
some operations are specialized to verification of MTNs (an extension of the
Petri Net formalism). MTN is a infinite state model for finite abstraction of
Java multi-threaded programs. In fact, each thread can be viewed as a finite
automata for which an abitrary number of instances can be considered.

REFERENCES
----------

For the original sharing tree data structure

- [Zam97]	Denis Zampuniéris' Ph.D. Thesis The Sharing Tree Data Structure
Theory and Applications in Formal Verification University of Namur (Belgium)
May 1997 

Denis Zampuniéris is the original author of a data structure called "Sharing
Tree".  Our "Interval Sharing Tree" as well as the "Covering Sharing Tree" of
G. Delzanno, J.F. Raskin, L. Van Begin are extension of the "Sharing Tree" data
structure. Further reading on CST:

- [DRV04] Giorgio Delzanno, Jean-François Raskin and Laurent Van Begin. CSTs (Covering Sharing Trees):  compact Data Structures for Parameterized Verification. Will appear in Software Tools for Technology Transfer manuscript, 2004.

- [Van03] Laurent Van Begin, Ph.D Thesis Efficient Verificaition of
Counting Abstraction.  Université Libre de Bruxelles. 2003.

Further reading on IST:

- [GMD+07] Pierre Ganty, Cédric Meuter, Giorgio Delzanno, Gabriel Kalyon, Jean-François Raskin, Laurent Van Begin. Symbolic Data Structure for sets of k-uples. Technical Report 570, Université Libre de Bruxelles, 2007.

- [Gan02] Pierre Ganty's  Master Thesis. (in French) Algorithmes et
structures de données efficaces pour la manipulation de contraintes sur les
intervalles. Université Libre de Bruxelles. 2002.

USEFUL STUFF
------------

The following list is not exhaustive, some modules are missing, e.g. the parser.
I found convenient to give a small overview of the modules and their main task :
- ./basis			General Purpose	 	basic manipulation of ISTs (see [Gan02],[Zam97]) + I/O primitives.	
- ./checkup			General Purpose	 	check that an IST respect his mathematical definition.
- ./complement   	        General Purpose	 	complements the denotation of an IST (see [Gan02]).
- ./computesim	                General Purpose	 	compute a simulation relation (see [Gan02]) between nodes of tree(s).
- ./determinize                   General Purpose	 	determinize the data structure (see [Gan02]), needed by complement.
- ./father_mangmt                 General Purpose		build the father relation (in complement of the son one) between nodes of an IST.
- ./interval                  	General Purpose	 	handles the interval data structure.
- ./ist_to_ps			General Purpose	 	gives a graphical depiction of an IST.
- ./listnode			General Purpose	 	module to handle list of nodes.
- ./minimize			General Purpose	 	minimize the size of an IST without modifying their information (see [Gan02]).
- ./minimizeinvarexact		Specific Purpose	reduce  an IST with respect to an invariant of a Petri Net (see [Gan02]).
- ./minimizeinvarheuristic	Specific Purpose	same as before but based on heuristics.
- ./normalize			General Purpose	 	modify an IST so that he respects his mathematical definition.
- ./precone			Specific Purpose	build the predecessor IST with respect to a transition system (see [Gan02]).
- ./proc				General Purpose	 	management of the memory and manipulation of atomic compoments of ISTs (see [Zam97]).
- ./transsystem			Specific Purpose	fills the transition_t data structure with input from file.
- ./remove			General Purpose	 	removes cleanly orphans nodes.
- ./stat			General Purpose	 	statistics about an IST.

MAIN AUTHOR COORDINATES
-----------------------

pierreganty@gmail.com
