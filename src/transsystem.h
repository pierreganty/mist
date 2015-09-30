// vim:sw=4:ts=4
/*
   This file is part of mist.

   mist is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   mist is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with mist; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   Copyright 2003, 2004, Pierre Ganty
 */

#ifndef __TRANSSYSTEM_H
#define __TRANSSYSTEM_H

#include "proc.h"
#include "interval.h"
#include "stddef.h"
#include "stdio.h"
#include "tree.h"

/* This is the last relevant define */
#define MAXNBTRANS				 			60

/*
 * Shorten the names should be a good idea, e.g.
 * nbR nbV nbI (remove limits)
 * transi
 * cmd4pl
 * transf
 * src
 * tgt
 * inv
 * wght4pl
 * m0p
 *
 *
 * transition_system_t
 * |-> limits
 * |		|-> nbr_rules, nbr_variables, nbr_invariants
 * |
 * |-> *transition
 * |        |-> *cmd_for_place
 * |		|       |-> delta, guard
 * |        |       |-> places_abstracted
 * |		|-> transfers[MAXNBTRANS]
 * |		| 		|-> *origin
 * |		|		|-> target
 * |		|-> nbr_transfers
 * |
 * |-> *tree_of_transitions
 * |
 * |-> *invariants
 * 			|-> *weight_on_place
 * 			|-> m0_p
 *
 */

/* Definition of the data structure representing the transition relation */
typedef struct gd_command_t {
    integer16  delta;
    ISTInterval guard;
	size_t places_abstracted;
} gd_command_t;
/*
 * If the system abstracts another one with the merging places technique then
 * the places_abstracted field tells how many places the current place abstracts.
 */

typedef struct transfers_t {
    integer16 *origin;
    integer16 target;
} transfers_t;

typedef struct transition_t {
	gd_command_t *cmd_for_place;
	/* Should be size_t as it defines the size of a data structure */
	integer16 nbr_transfers;
    transfers_t transfers[MAXNBTRANS];
} transition_t;

typedef struct invariant_t {
	integer16 *weight_on_place;
	ISTInterval *m0_p;
} invariant_t;

typedef struct limits_t {
	/* Should be size_t as they define the size of a data structure and they are used in e.g. xmalloc function */
	integer16 nbr_rules;
	integer16 nbr_variables;
	integer16 nbr_invariants;
} limits_t;

typedef struct transition_system_t {
	limits_t limits;
	transition_t *transition;
	ISTSharingTree *tree_of_transitions;
	invariant_t *invariants;
} transition_system_t;


void build_problem_instance(T_PTR_tree tree, transition_system_t **system,
	ISTSharingTree **init, ISTSharingTree **safe);
void dispose_transition_system(transition_system_t *sys);
void print_transition_system(transition_system_t *sys);
void from_transitions_to_tree(transition_system_t *sys, boolean *mask);

#endif
