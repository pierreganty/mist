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

#ifndef __FATHER_MANGMT_H
#define __FATHER_MANGMT_H

#include "proc.h"


/*
 * This modules is in charge to build and provide an interface to manage the father relation inside an IST.
 * (e.g. ist_prune_tree_sim_based)
 */
void ist_remove_father( ISTNode *Father, ISTNode *Son);
void ist_add_father( ISTNode *Father, ISTNode *Child);
void ist_add_son_father( ISTNode *node, ISTNode *child);
void ist_remove_sons_fathers( ISTNode *node);
void ist_copy_sons_fathers( ISTNode *orgnode, ISTNode*tgtnode);
void ist_remove_son_father( ISTNode *node, ISTNode *child);
void ist_remove_fathers_sons( ISTNode *node);
void ist_remove_node_fathers_sons( ISTLayer *layer, ISTNode *node);
void ist_construct_fathers_info( ISTSharingTree *S);
void ist_dispose_fathers_info(ISTSharingTree *S);
void ist_merge_sons_fathers( ISTNode *Source, ISTNode *Target);
void ist_adjust_first_condition_sons_fathers( ISTSharingTree *S);
void ist_adjust_second_condition_sons_fathers( ISTSharingTree *S);
void ist_normalize_sons_fathers( ISTSharingTree *S);

/*Remove and dispose the nodes without fathers*/
void ist_dispose_node_without_father( ISTSharingTree *S);
/*Eliminate the nodes in S that have no sons*/
void ist_dispose_node_without_son( ISTSharingTree *S);

#endif
