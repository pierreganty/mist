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

   Copyright 2002, Anthony Piron
 */

#ifndef __TREE_H
#define __TREE_H

#include <sys/types.h>
#include <stdarg.h>
#include "def.h"

typedef enum{CONT, BRK, BRK_BRANCH} T_tree_state;

struct STRUCT_T_tree;
typedef struct STRUCT_T_tree T_tree;
typedef T_tree* T_PTR_tree;

struct STRUCT_T_tree {
  void* info;
  T_tree_state state;
  size_t nbrsubtrees;
  T_PTR_tree* subtrees;
};


T_PTR_tree tree_new(void* info, size_t size, ...);
#define tree_new0(info) tree_new(info,0)
#define tree_new1(info,node) tree_new(info,1,node)
#define tree_new2(info,node1,node2) tree_new(info,2,node1,node2)
#define tree_new3(info,node1,node2,node3) tree_new(info,3,node1,node2,node3)
#define tree_new4(info,node1,node2,node3,node4) tree_new(info,4,node1,node2,node3,node4)

void* tree_getinfo(T_PTR_tree tree);

#define tree_brk(tree) tree->state = BRK;
#define tree_brk_branch(tree) tree->state = BRK_BRANCH
#define tree_subtree(entry,i) entry->subtrees[i]
#define tree_nbrsubtrees(entry) entry->nbrsubtrees

T_PTR_tree tree_merge(void* info, T_PTR_tree t1, T_PTR_tree t2);
void tree_destroy(T_PTR_tree* tree);
boolean tree_dump(T_PTR_tree tree,
               boolean (*callback_node_before)(T_PTR_tree entry),
               boolean (*callback_node_after)(T_PTR_tree entry),
               boolean (*callback_leaf)(T_PTR_tree entry));



#endif
