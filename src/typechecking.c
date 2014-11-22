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

boolean possible_petri_net(T_PTR_tree entry){
  //printf("Checking if the node: %s belongs to a petri net \n", (char*)entry->info);

   if (strcmp((char*)entry->info, "=") == 0){
      if (entry->nbrsubtrees != 2) return false;

      T_PTR_tree child, subchild;
      //printf("Obtaining child\n");
      child = entry->subtrees[1];
      //printf("Child: %s \t has %d subchilds\n", (char*)child->info, child->nbrsubtrees);

      if (child->nbrsubtrees != 2) return false;
      //printf("Obtaining subchild\n");
      subchild = child->subtrees[0];

      if (subchild->nbrsubtrees != 0) return false;

      return true;

   }
   return true; // Nothing to do with this node but it doesn't represent any problem
}

boolean is_petri_net(T_PTR_tree tree){
  size_t i;
  //printf("Welcome to is_petri_net\n");
  if (tree) {
    //printf("We have a tree\n");
     if (tree->nbrsubtrees) {
        tree->state = CONT;
        //printf("Checking if is possible to have a petri net\n");
        if (!possible_petri_net(tree)) return false;
        //printf("It's possible!!");
        switch (tree->state)
        {
           case CONT:
              for (i = 0; i < tree->nbrsubtrees && tree->state != BRK; i++)
                 if (tree->subtrees[i]) {
                    //printf("Calling myself\n");
                    if (!is_petri_net(tree->subtrees[i])) return false;
                    if (tree->subtrees[i]->state == BRK)
                       tree_brk_branch(tree);
                 }
              break;
           case BRK:
              break;
           case BRK_BRANCH:
              tree->state = CONT;
              break;
           default:
              err_quit ("Unknown state in is_petri_net\n");
        }
     }
  }
  return true;
}