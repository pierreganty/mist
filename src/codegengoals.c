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

   Copyright 2003, 2004, Pierre Ganty, Anthony Piron, 2015, Pedro Valero
 */

#include "codegengoals.h"
#include "xmalloc.h"
#include "tbsymbol.h"
#include "laparser.h"
#include "interval.h"

static void goals(T_PTR_tree entry, ISTSharingTree *unsafe);
static void goalsor(T_PTR_tree entry);
static void goalsand(T_PTR_tree entry);

static ISTInterval **tokensgoals ;
static size_t nbrgoalscmd;


void
goalscode_produce(T_PTR_tree entry, ISTSharingTree *unsafe) {
  goals(entry, unsafe);
}

static
void
goals(T_PTR_tree entry, ISTSharingTree *unsafe) {
	size_t i;
	char* info;

	if (entry) {
		info = (char*) tree_getinfo(entry);
		if (strcmp(info,"or") == 0) {
			tokensgoals = (ISTInterval**)xmalloc(nbr_var*tree_nbrsubtrees(entry)*sizeof(ISTInterval *));
			for (i = 0 ; i < nbr_var * tree_nbrsubtrees(entry) ; i++)
				tokensgoals[i] = ist_build_interval(0,INFINITY);

			for (nbrgoalscmd = 0 ; nbrgoalscmd < tree_nbrsubtrees(entry) ; nbrgoalscmd++)
				goalsor(tree_subtree(entry,nbrgoalscmd));

		} else if (strcmp(info,"and") == 0) {
			nbrgoalscmd = 0;

			tokensgoals = (ISTInterval **)xmalloc(nbr_var * sizeof(ISTInterval *));
			for (i = 0 ; i < nbr_var; i++)
				tokensgoals[i] = ist_build_interval(0,INFINITY);

			goalsor(entry);

			nbrgoalscmd = 1;
		}
		for (i = 0; i < nbrgoalscmd; ++i) {
			if (ist_add(unsafe, &tokensgoals[i * nbr_var], nbr_var) == false)
				err_msg("codegengoals.c: redundant unsafe cones\n");
		}
		for (i=0;i<nbr_var*nbrgoalscmd;++i)
			xfree(tokensgoals[i]);
		xfree(tokensgoals);
	}
}


static
void
goalsor(T_PTR_tree entry) {
  size_t i;

  if (entry) {
    for (i = 0 ; i < tree_nbrsubtrees(entry) ; i++)
      goalsand(tree_subtree(entry,i));
  }
}


static
void
goalsand(T_PTR_tree entry) {
	T_PTR_tbsymbol_info infoid, infonb_left, infonb_right;
	char *info;

	if (entry) {
		if (tree_nbrsubtrees(entry)){
			info = (char *) tree_getinfo(entry);
			infoid = tbsymbol_getinfo(tree_getinfo(tree_subtree(entry, 0)));
			infonb_left = tbsymbol_getinfo(tree_getinfo(tree_subtree(entry, 1)));
			if (strcmp(info,"=") == 0){
				ist_assign_values_to_interval(tokensgoals[nbrgoalscmd * nbr_var + infoid->info.id.addr],
						(integer32) infonb_left->info.nb.value, (integer32) infonb_left->info.nb.value);
			} else if (strcmp(info,">=") == 0){
				ist_assign_values_to_interval(tokensgoals[nbrgoalscmd * nbr_var + infoid->info.id.addr],
						(integer32) infonb_left->info.nb.value,INFINITY);
			} else if (strcmp(info,"in") == 0){
				infonb_right = tbsymbol_getinfo(tree_getinfo(tree_subtree(entry, 2)));
				ist_assign_values_to_interval(tokensgoals[nbrgoalscmd * nbr_var + infoid->info.id.addr],
						(integer32) infonb_left->info.nb.value,(integer32) infonb_right->info.nb.value);
			}
		}
	}
}
