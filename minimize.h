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

#ifndef __MINIMIZE_H
#define __MINIMIZE_H

#include "def.h"
#include "proc.h"


/*
 * As explained in [Gan02] the ISTs do not have a canonical from. (However CST
 * have.) In this module we implemented functions that try however to shrink
 * the size of the ISTs by pruning some paths insides the trees. Obviously,
 * these functions do not affect the denotation of the tree.
 */

/* Exact minimization of the tree, i.e. we use necessary and sufficient
 * condition to prune paths 
 */
ISTSharingTree *ist_compute_subsumed_paths(ISTSharingTree *ST1, ISTSharingTree *ST2) ;
ISTSharingTree *ist_remove_subsumed_paths(ISTSharingTree *S, ISTSharingTree *T) ;
ISTSharingTree *ist_compute_who_subsumes(ISTSharingTree *ST1,ISTSharingTree *ST2);
ISTSharingTree *ist_compute_subsumed_paths_within_tree(ISTSharingTree *S) ;
ISTSharingTree *ist_minimal_form(ISTSharingTree *S) ;

/* Minimization approximated with the simulation relation */
boolean ist_prune_within_tree_sim_based( ISTSharingTree *S); 
void ist_minimize_tree_sim_based( ISTSharingTree *S);
void ist_prune_tree_sim_based( ISTSharingTree *S, ISTSharingTree *T); 
boolean ist_exact_subsumption_test(ISTSharingTree *T,ISTSharingTree *S);

#endif
