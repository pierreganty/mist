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
#include"precone.h"
#include"remove.h"
#include"complement.h"

typedef struct abstraction_t {
		integer16 nbV;
		integer16 **A;
		integer16 *bound;
} abstraction_t;

/* Only works for Petri Nets w/o invariants */
transition_system_t *build_sys_using_abs(transition_system_t *sys, abstraction_t *abs);
abstraction_t *refine_abs(transition_system_t *system, abstraction_t *cur_abs, ISTSharingTree *S);

/* abstract operators */
ISTSharingTree *ist_abstract_post_of_rules(ISTSharingTree * S, abstraction_t * abs, transition_system_t *t, int rule); 
void abstract_bound(ISTSharingTree *S, abstraction_t * abs);
void ist_downward_closure(ISTSharingTree * S);
ISTSharingTree * abstract_place_pretild_rule(ISTSharingTree * S, abstraction_t * abs, transition_system_t *t, int rule);
ISTSharingTree * abstract_place_pretild(ISTSharingTree * S, abstraction_t * abs, transition_system_t *t);
ISTSharingTree * abstract_pretild(ISTSharingTree * S, abstraction_t * abs, transition_system_t *t);
#endif
