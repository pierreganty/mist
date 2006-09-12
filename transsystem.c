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

   Copyright 2003, 2004, Pierre Ganty, Anthony Piron
 */

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include "error.h"
#include "xmalloc.h"
#include "tbsymbol.h"
#include "laparser.h"
#include "codegeninit.h"
#include "codegengoals.h"
#include "codegenrules.h"
#include "codegeninvar.h"


static transition_system_t *_system;
static ISTSharingTree *_init;
static ISTSharingTree *_unsafe;


static
void
callback_tree_before(T_PTR_tree entry) {
  char* info;

  info = (char*) tree_getinfo(entry);
  if (strcmp(info,"rules") == 0) {
    tree_brk_branch(entry);
    /* nbr_rules will be fill at this point */
    rulescode_produce(entry, &_system);
  } else if (strcmp(info,"init") == 0) {
    tree_brk_branch(entry);
    /* system is a OUT parameter */
    initcode_produce(tree_subtree(entry,0), &_init);
  } else if (strcmp(info,"target") == 0) {
    tree_brk_branch(entry);
    /* system is a OUT parameter */
    goalscode_produce(tree_subtree(entry,0), &_unsafe);
  } else if (strcmp(info,"invariants") == 0) {
    tree_brk_branch(entry);
    /* system and init are IN/OUT parameters */
    invariantscode_produce(tree_subtree(entry,0), _system, _init);
  }
}

void
build_problem_instance(tree, system, init, unsafe) 
	T_PTR_tree tree;
	transition_system_t **system; 
	ISTSharingTree **init, **unsafe;
{


  tree_dump(tree, callback_tree_before, NULL, NULL);
  *system = _system;
  *init = _init;
  *unsafe = _unsafe;

}

void release_transition_system(transition_system_t *sys)
{
	size_t i,j;
	for (i=0;i<sys->limits.nbr_rules;++i){
		for(j=0;j<sys->transition[i].nbr_transfers;++j)
			xfree(sys->transition[i].transfers[j].origin);
		xfree(sys->transition[i].cmd_for_place);
	}
	for(j=0;j<sys->limits.nbr_invariants;++j)
			xfree(sys->invariants[j].weight_on_place);
	xfree(sys);
}
