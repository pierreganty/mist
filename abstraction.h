// vim:sw=4:ts=4
/*
   This file is part of mist2.

   mist2 is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   mist2 is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with mist2; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   Copyright 2003, 2004, Pierre Ganty
 */

#ifndef __ABSTRACTION_H
#define __ABSTRACTION_H


#include"def.h"
#include"proc.h"
#include"transsystem.h"
#include"basis.h"
#include"predtrans.h"
#include"remove.h"
#include"complement.h"
#include"minimize.h"

typedef struct abstraction_t {
		/* A local copy of system->limits.nbr_variables */
		integer16 nbConcreteV;
		integer16 nbV;
		integer16 **A;
		integer16 *bound;
} abstraction_t;

/* To display the content and free the abstraction_t data structure */
void print_abstraction(abstraction_t *abs);
void dispose_abstraction(abstraction_t *abs);

/* Only works for Petri Nets w/o invariants */
transition_system_t *build_sys_using_abs(transition_system_t *sys, abstraction_t *abs);
abstraction_t *refine_abs(abstraction_t *cur_abs, ISTSharingTree *S, ISTSharingTree *cpreS);

ISTSharingTree *ist_abstraction(ISTSharingTree *S, abstraction_t *abs);
ISTSharingTree *ist_concretisation(ISTSharingTree *S, abstraction_t * abs);

void abstract_bound(ISTSharingTree *S, integer16 *bound);
void bound_values(ISTSharingTree *S, integer16 *bound);


/* abstract operators */
ISTSharingTree *ist_abstract_post_of_rules(ISTSharingTree *S, abstraction_t *abs, transition_t *t);
ISTSharingTree *ist_abstract_post(ISTSharingTree *S, abstraction_t *abs, transition_system_t *t);
ISTSharingTree *ist_abstract_post_star(ISTSharingTree *initial_marking, abstraction_t *abs, transition_system_t *t);

ISTSharingTree *adhoc_place_pretild_rule(ISTSharingTree *S, transition_t *t);
ISTSharingTree *adhoc_pretild(ISTSharingTree *S, transition_system_t *t);

ISTSharingTree *bounded_post_rule(ISTSharingTree *S, integer16 *bound, transition_t *t);
ISTSharingTree *bounded_post(ISTSharingTree *S, abstraction_t *abs, transition_system_t *t);
ISTSharingTree *bounded_post_star(ISTSharingTree * initial_marking, abstraction_t * abs, transition_system_t *t);

#endif
