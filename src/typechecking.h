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

   Copyright 2014 Pedro Valero
 */

#ifndef __TYPECHECKING_H
#define __TYPECHECKING_H

#include "tree.h"
#include "def.h"

/*
This structure represents the list used to store
the pairs variable-constant for each guard.
*/
struct guard_list {
  char *variable;
  int min_value, max_value;
  struct guard_list *next;
};

/*
   This function checks if a node satisfies the conditions needed to belong to a petri net checking.

   Returns:
    * true: The node could belong to a petri net but we can't ensure that the tree which contains it comes from a petri net
    * false: The node couldn't belong to a petri net so the tree doesn't come from a petri net
*/
boolean possible_petri_net_check_action(T_PTR_tree entry);


/*
   This function builds a list which contains the guards readed

   Returns:
    * true: The node could belong to a petri net but we can't ensure that the tree which contains it comes from a petri net
    * false: The node couldn't belong to a petri net so the tree doesn't come from a petri net
*/
boolean possible_petri_net_check_guards(T_PTR_tree entry);


/*
   This functions moves along the tree checking, for each node, if it belongs to a petri net in order to tell us if the tree comes from a petri net or doens't

   Returns:
    * true: The tree 'atree' represents a petri net
    * false: The tree 'atree' doesn't represent a petri net
*/
boolean is_petri_net(T_PTR_tree tree);

#endif
