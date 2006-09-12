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


#ifndef __PRECONE_H
#define __PRECONE_H

#include "proc.h"
#include "transsystem.h"


/*
 * This modules implements the predecessor function of a sharing tree, w.r.t a transition system.
 */
ISTSharingTree *ist_intersection_with_formula_transfer(ISTSharingTree *ST1, transfers_t *Trans, ISTInterval *Value);
ISTSharingTree *ist_pre_of_transfer(ISTSharingTree *S, transition_t *transition);
ISTSharingTree *ist_post_of_transfer(ISTSharingTree *S, transition_t *transition);
ISTSharingTree *ist_pre_of_rule_plus_transfer(ISTSharingTree *Prec, transition_t *transition);
/* Post operator only implemented for system with transfert */
ISTSharingTree *ist_post(ISTSharingTree *forward_p, transition_system_t *system);
ISTSharingTree *ist_post_transition(ISTSharingTree *forward_p, transition_system_t *system, int transition);
/* Computation of the Pre ... with TransSharingTree */
ISTSharingTree *ist_pre_of_rules (ISTSharingTree *prec);
/* Pre operator */
ISTSharingTree *ist_pre_cone(ISTSharingTree *Prec, ISTSharingTree *ReachedElem, transition_system_t *system) ;
#endif
