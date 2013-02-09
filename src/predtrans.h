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


#ifndef __PREDTRANS_H
#define __PREDTRANS_H

#include "proc.h"
#include "transsystem.h"
#include "complement.h"

/* Symbolic Post of a transfer (used in ic4pn to abstract a set of markings) */
ISTSharingTree *ist_post_of_transfer(ISTSharingTree *S, transfers_t *transfers);
/* Symbolic Post for systems w/o transfers */
ISTSharingTree *ist_symbolic_post_of_rules(ISTSharingTree * S, transition_t *t);
ISTSharingTree *ist_symbolic_post(ISTSharingTree * S, transition_system_t *t);
/* Enumerative Post for systems w/ transfers */
ISTSharingTree *ist_enumerative_post(ISTSharingTree *forward_p, transition_system_t *system);
ISTSharingTree *ist_enumerative_post_transition(ISTSharingTree *forward_p, transition_system_t *system, size_t transition);

/* Computation of the Pre ... with TransSharingTree (DEPRECATED)
ISTSharingTree *ist_pre_of_rules(ISTSharingTree *prec); */

/* The pre is actually min o pre o ucl(X) */
ISTSharingTree *ist_intersection_with_formula_transfer(ISTSharingTree *ST1, transfers_t *Trans, ISTInterval *Value);
ISTSharingTree *ist_pre_of_all_transfer(ISTSharingTree *S, transition_t *transition);
ISTSharingTree *ist_pre_of_rule_plus_transfer(ISTSharingTree *Prec, transition_t *transition);
/* Symbolic Pre operator for system w/ transfers */
ISTSharingTree *ist_pre(ISTSharingTree *S, transition_system_t *system);
ISTSharingTree *ist_pre_pruned_wth_inv_and_prev_iterates(ISTSharingTree *Prec, ISTSharingTree *ReachedElem, transition_system_t *system);
ISTSharingTree *ist_symbolic_pre_of_rule(ISTSharingTree *Prec, transition_t *transition);
/* No transfer */
ISTSharingTree *ist_enumerative_pre(ISTSharingTree *backward_p, transition_system_t *system);
ISTSharingTree *ist_enumerative_pre_transition(ISTSharingTree *backward_p, transition_system_t *system, size_t transition);

/* for concretisation */
ISTSharingTree * ist_pre_of_all_transfer_for_concretisation(ISTSharingTree * S,transition_t * t);

ISTSharingTree *ist_post_of_rules(ISTSharingTree *IST_trans_tree, ISTSharingTree *succ);
ISTSharingTree *ist_pre_of_rules(ISTSharingTree *IST_trans_tree, ISTSharingTree *succ);

#endif
