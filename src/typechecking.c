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
#include <string.h>

#include "error.h"
#include "xmalloc.h"
#include "tree.h"
#include "typechecking.h"
#include "def.h"
#include "tree.h"
#include "tbsymbol.h"
#include "limits.h"

/* Global variable to store guards.
   These lists will be initialized by the function possible_petri_net_check_guard and used by possible_petri_net_check_action.*/
struct guard_list *list = NULL;
struct guard_list *list_last;

/* If the entry is representing a rule of a Petri Net it will have
 the following structure
      =
     /  \
    x    +
        /  \
       x'  int
*/
boolean possible_petri_net_check_action(T_PTR_tree entry){
   //printf("Checking if the node: %s belongs to a petri net \n", (char*)entry->info);
   // Checking if the entry represents the definition of a guard.
   // As this function will be called over all the nodes of the tree
   // that represents the net, it must detect when
   if (strcmp((char*)entry->info, "=") == 0){

      // Checking that the structure of the entry is correct.
      if (entry->nbrsubtrees != 2) return false;

      T_PTR_tree child, subchild_var;
      child = entry->subtrees[1];

      if (child->nbrsubtrees != 2) return false;
      subchild_var = child->subtrees[0];
      int value = atoi(child->subtrees[1]->info);

      if (subchild_var->nbrsubtrees != 0) return false;

      if (strcmp((char*)entry->subtrees[0]->info,(char*)subchild_var->info) != 0) return false;

      // Find the guard_element related to the variables that
      // the transition is modifying.
      struct guard_list *aux = list;
      struct guard_list *iter = list;
      while (iter != NULL){
         if (strcmp(iter->variable, (char*)subchild_var->info) == 0) aux = iter;
         iter = iter->next;
      }

      // Check that the action is valid and do not lead to
      if (strcmp((char*)child->info, "-") == 0){
         if (aux->min_value < value) return false;
      }else if (strcmp((char*)child->info, "+") == 0){
         if (aux->max_value < value) return false;
      }

      return true;

   }
   return true; // Nothing to do with this node but it doesn't represent any problem
}

boolean possible_petri_net_check_guards(T_PTR_tree entry){
if ((strcmp((char*)entry->info, ">=") == 0) || strcmp((char*)entry->info, "=") == 0){

      T_PTR_tree value_child, name_child;
      name_child = entry->subtrees[0];
      value_child = entry->subtrees[1];

      // Check if the "=" belong to a guard declaration or to an action one
      if (value_child->nbrsubtrees != 0) return true;

      // Create and initializa a new element for the list.
      struct guard_list *new;
      new = (struct guard_list *)xmalloc(sizeof(struct guard_list));

      new->variable = (char *)xmalloc(strlen(((T_PTR_tbsymbol_entry) name_child->info)->name)+1);
      strcpy(new->variable,((T_PTR_tbsymbol_entry) name_child->info)->name);
      new->min_value = atoi((char*)value_child->info);
      new->max_value = INT_MAX;
      new->next = NULL;

      //printf("Adding to the list variable: %s with values:[%d,%d]\n", new->variable, new->min_value,new->max_value);

      // If it's the first call to the function we must initialize the list
      if (list == NULL) {
         list = new;
         list_last = new;
      } else {
         list_last->next = new;
         list_last = new;
      }


   } else if (strcmp((char*)entry->info, "in") == 0){
      T_PTR_tree value_min_child, value_max_child, name_child;
      name_child = entry->subtrees[0];
      value_min_child = entry->subtrees[1];
      value_max_child = entry->subtrees[2];

      // Create and initializa a new element for the list
      struct guard_list *new;
      new = (struct guard_list *)xmalloc(sizeof(struct guard_list));

      new->variable = (char *)xmalloc(strlen(((T_PTR_tbsymbol_entry) name_child->info)->name)+1);
      strcpy(new->variable,((T_PTR_tbsymbol_entry) name_child->info)->name);
      new->min_value = atoi((char*)value_min_child->info);
      new->max_value = atoi((char*)value_max_child->info);
      new->next = NULL;

      //printf("Adding to the list variable: %s with values:[%d,%d]\n", new->variable, new->min_value,new->max_value);

      // If it's the first call to the function we must initialize the list
      if (list == NULL) {
         list = new;
         list_last = new;
      } else {
         list_last->next = new;
         list_last = new;
      }
   }

   return true; // Nothing to do with this node but it doesn't represent any problem
}

boolean is_petri_net(T_PTR_tree tree){
   // To check if we are working with a Petri Net we just have to look at the rules subtree
   boolean ret = tree_dump(tree->subtrees[0], possible_petri_net_check_guards, possible_petri_net_check_action, NULL);

   struct guard_list *iter = list;
   while (iter != NULL){
      iter = list->next;
      xfree(list->variable);
      xfree(list);
      list = iter;
   }

   return ret;
}


