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

#include "error.h"
#include "xmalloc.h"
#include "tree.h"


T_PTR_tree
tree_new(void* info, size_t size, ...) {
  va_list ap;
  size_t i;
  T_PTR_tree tree;

  tree = (T_PTR_tree) xmalloc(sizeof(T_tree));
  tree->info = info;
  tree->nbrsubtrees = size;

  va_start(ap, size);

  if (size) {
    tree->subtrees = (T_PTR_tree*) xmalloc(sizeof(T_PTR_tree) * size);
    for (i = 0; i < size; i++)
      tree->subtrees[i] = va_arg(ap, T_PTR_tree);
  } else {
    tree->subtrees = NULL;
  }

  va_end(ap);

  return tree;
}


void*
tree_getinfo(T_PTR_tree tree) {
  return tree->info;
}


T_PTR_tree
tree_merge(void* info, T_PTR_tree t1, T_PTR_tree t2) {
  size_t i, size, t1size, t2size;
  T_PTR_tree tree;

  tree = (T_PTR_tree) xmalloc(sizeof(T_tree));
  tree->info = info;
  t1size = t1?t1->nbrsubtrees:0;
  t2size = t2?t2->nbrsubtrees:0;
  size = t1size + t2size;
  tree->nbrsubtrees = size;

  if (size) {
    tree->subtrees = (T_PTR_tree*) xmalloc(sizeof(T_PTR_tree) * size);
    for (i = 0; i < t1size; i++)
      tree->subtrees[i] = t1->subtrees[i];
    for (i = 0; i < t2size; i++)
      tree->subtrees[t1size + i] = t2->subtrees[i];
  } else {
    tree->subtrees = NULL;
  }

  return tree;
}

void
tree_destroy(T_PTR_tree* tree) {

}



boolean
tree_dump(T_PTR_tree tree,
	  boolean (*callback_node_before)(T_PTR_tree entry),
	  boolean (*callback_node_after)(T_PTR_tree entry),
	  boolean (*callback_leaf)(T_PTR_tree entry)) {
  size_t i;

  if (tree) {
	  if (tree->nbrsubtrees) {
		  tree->state = CONT;
		  if (callback_node_before)
			  if (!callback_node_before(tree)) return false;
		  switch (tree->state)
		  {
			  case CONT:
				  for (i = 0; i < tree->nbrsubtrees && tree->state != BRK; i++)
					  if (tree->subtrees[i]) {
						  if(!tree_dump(tree->subtrees[i],
								  callback_node_before,
								  callback_node_after, callback_leaf)) return false;
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
				  err_quit ("Unknown state in tree_dump\n");
		  }
		  if (callback_node_after)
			  if(!callback_node_after(tree)) return false;
	  } else {
		  if (callback_leaf)
			  if(!callback_leaf(tree)) return false;
	  }
  }
  return true;
}
