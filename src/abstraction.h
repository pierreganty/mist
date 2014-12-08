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

   Copyright 2003, 2004, Pierre Ganty, 2007 Laurent Van Begin
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
abstraction_t *glb(abstraction_t *abs1, abstraction_t *abs2);

transition_system_t *build_sys_using_abs(transition_system_t *sys, abstraction_t *abs);

ISTSharingTree *ist_abstraction(ISTSharingTree *S, abstraction_t *abs);
ISTSharingTree *ist_concretisation(ISTSharingTree *S, abstraction_t * abs);

/* operator for refinement */
abstraction_t *naive_new_abstraction(ISTSharingTree *S,int nb_var);
abstraction_t *new_abstraction_dc_set(ISTSharingTree *S,int nb_var);
abstraction_t *new_abstraction_finite_set(ISTSharingTree *S,int nb_var);
abstraction_t *new_abstraction_lub(ISTSharingTree *S, int nb_var, abstraction_t *old_abs);
abstraction_t *glb(abstraction_t *abs1, abstraction_t *abs2);


/* abstract in the sens {0,..,k} U {omega} abstraction */
/* the abstract post is only for petri nets. Moreover you have to pass
 * the approximation functio in parameter. */
ISTSharingTree
*ist_abstract_post_of_rules(ISTSharingTree *S, void (*approx)(ISTSharingTree\
			*S, integer16 *b), integer16 *bound, transition_t *t);
ISTSharingTree
*ist_abstract_post(ISTSharingTree * S, void (*approx)(ISTSharingTree\
			*S, integer16 *b), integer16 *bound, transition_system_t *t);
ISTSharingTree
*ist_abstract_post_star(ISTSharingTree *initial_marking, void
		(*approx)(ISTSharingTree *S, integer16* b), integer16 *bound,
		transition_system_t *t);
void RemoveUnboundedNodes(ISTSharingTree *S);
ISTSharingTree *ist_abstract_post_star_tsi(ISTSharingTree *initial_marking, void
		(*approx)(ISTSharingTree *S, integer16* b), integer16 *bound,
		transition_system_t *t);

/* adhoc in the sense that only a subset is actulally considered, see atpn
 * paper for details */
/* the pretild works for transfers */
ISTSharingTree *adhoc_pretild_rule(ISTSharingTree *S, transition_t *t);
ISTSharingTree *adhoc_pretild(ISTSharingTree *S, transition_system_t *t);
/* the pre works for transfers */
ISTSharingTree *adhoc_pre_rule(ISTSharingTree *S, transition_t *t);
ISTSharingTree *adhoc_pre(ISTSharingTree *S, transition_system_t *t);
ISTSharingTree *adhoc_pre_star_pruned_unless_hit_m0(ISTSharingTree *S,\
		ISTSharingTree *cutter, transition_system_t *sys, ISTSharingTree\
		*initial_marking);
ISTSharingTree
*ist_abstract_post_transtree(ISTSharingTree *S, void (*approx)(ISTSharingTree
			*S, integer16 *b), integer16 *bound, transition_system_t *t);

ISTSharingTree *ist_abstract_post_star_until_reach_bad(ISTSharingTree *initial_marking, void
		(*approx)(ISTSharingTree *S, integer16* b), integer16 *bound,
		transition_system_t *t, ISTSharingTree *bad);
#endif
