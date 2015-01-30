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

#include "codegeninit.h"
#include "xmalloc.h"
#include "tbsymbol.h"
#include "laparser.h"
#include "interval.h"
#include "ist.h"

static void init(T_PTR_tree entry, ISTSharingTree *initial);
static void initor(T_PTR_tree entry);
static void initand(T_PTR_tree entry);

static ISTInterval **tokensinit;
static size_t nbrinitcmd;

void
initcode_produce(T_PTR_tree entry, ISTSharingTree *initial) {
  init(entry, initial);
}

static
void
init(T_PTR_tree entry, ISTSharingTree *initial) {
  size_t i;
  char* info;

  if (entry) {
    info = (char*) tree_getinfo(entry);
    if (strcmp(info,"and") == 0) {
      nbrinitcmd = 0;

      tokensinit = (ISTInterval **)xmalloc(nbr_var*sizeof(ISTInterval *));
      for (i = 0 ; i < nbr_var; i++)
		  tokensinit[i] = ist_build_interval(0,INFINITY);

      initor(entry);
      nbrinitcmd = 1;
	  ist_add(initial,tokensinit, nbr_var);
    }
  }
}


static
void
initor(T_PTR_tree entry) {
  size_t i;

  if (entry) {
    for (i = 0 ; i < tree_nbrsubtrees(entry) ; i++)
      initand(tree_subtree(entry,i));
  }
}


static
void
initand(T_PTR_tree entry) {
	T_PTR_tbsymbol_info infoid, infonb_left, infonb_right;
	char *info;

	if (entry) {
		if (tree_nbrsubtrees(entry)){
			info = (char *) tree_getinfo(entry);
			infoid = tbsymbol_getinfo(tree_getinfo(tree_subtree(entry, 0)));
			infonb_left = tbsymbol_getinfo(tree_getinfo(tree_subtree(entry, 1)));
			if (strcmp(info,"=") == 0){
				ist_assign_values_to_interval(tokensinit[nbrinitcmd * nbr_var + infoid->info.id.addr],
						(integer32) infonb_left->info.nb.value, (integer32) infonb_left->info.nb.value);
			} else if (strcmp(info,">=") == 0){
				ist_assign_values_to_interval(tokensinit[nbrinitcmd * nbr_var + infoid->info.id.addr],
						(integer32) infonb_left->info.nb.value,INFINITY);
			} else if (strcmp(info,"in") == 0){
				infonb_right = tbsymbol_getinfo(tree_getinfo(tree_subtree(entry, 2)));
				ist_assign_values_to_interval(tokensinit[nbrinitcmd * nbr_var + infoid->info.id.addr],
						(integer32) infonb_left->info.nb.value, (integer32) infonb_right->info.nb.value);
			}
		}
	}
}
