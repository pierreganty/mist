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
#include "debug.h"


static transition_system_t *_system;
static ISTSharingTree *_init;
static ISTSharingTree *_unsafe;


static
boolean
callback_tree_before(T_PTR_tree entry) {
  char* info;

  info = (char*) tree_getinfo(entry);
  if (strcmp(info,"rules") == 0) {
  	PRINTF("Entering rules section\n");
    tree_brk_branch(entry);
    /* nbr_rules will be filled at this point */
    rulescode_produce(entry, _system);
  } else if (strcmp(info,"init") == 0) {
  	PRINTF("Entering init section\n");
    tree_brk_branch(entry);
    /* system is a OUT parameter */
    initcode_produce(tree_subtree(entry,0), _init);
  } else if (strcmp(info,"target") == 0) {
  	PRINTF("Entering target section\n");
    tree_brk_branch(entry);
    /* system is a OUT parameter */
    goalscode_produce(tree_subtree(entry,0), _unsafe);
  } else if (strcmp(info,"invariants") == 0) {
  	PRINTF("Entering invariants section\n");
    tree_brk_branch(entry);
    /* system and init are IN/OUT parameters */
    invariantscode_produce(tree_subtree(entry,0), _system, _init);
  }

  return true;
}

void
build_problem_instance(tree, system, init, unsafe)
	T_PTR_tree tree;
	transition_system_t **system;
	ISTSharingTree **init, **unsafe;
{
  _system = (transition_system_t *)xmalloc(sizeof(transition_system_t));
  _system->transition = NULL;
  _system->tree_of_transitions = NULL;
  _system->invariants = NULL;
  _system->limits.nbr_variables = 0;
  _system->limits.nbr_rules = 0;
  _system->limits.nbr_invariants = 0;

  ist_new(&_init);
  ist_new(&_unsafe);

  tree_dump(tree, callback_tree_before, NULL, NULL);
  *system = _system;
  *init = _init;
  *unsafe = _unsafe;

}

/* precond: sys->tree_of_transitions has been disposed if necessary */
void
from_transitions_to_tree(transition_system_t *sys, boolean *mask)
{
	ISTInterval **transition;
	int i,j;

	ist_new(&sys->tree_of_transitions);

	transition = (ISTInterval **)xmalloc(sys->limits.nbr_variables*sizeof(ISTInterval *));
	for (i=0; i<sys->limits.nbr_rules;++i) {
			if (mask[i]==true) {
				for (j=0; j<sys->limits.nbr_variables; ++j) {
					transition[j]=ist_copy_interval(&sys->transition[i].cmd_for_place[j].guard);
					transition[j]->Right=sys->transition[i].cmd_for_place[j].delta;
				}
				ist_add(sys->tree_of_transitions, transition, sys->limits.nbr_variables);
			}
	}
}


void dispose_transition_system(transition_system_t *sys)
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


void print_transition_system(transition_system_t *sys)
{
	size_t i,j;
	ISTInterval *true_gd=ist_build_interval(0,INFINITY);
	printf("System has %d variables, %d transitions, %d invariants\n",sys->limits.nbr_variables, sys->limits.nbr_rules, sys->limits.nbr_invariants);
	for(i=0;i<sys->limits.nbr_rules;++i){
		printf("transition %zu\n",i);
		for(j=0;j<sys->limits.nbr_variables;++j){
			if(ist_equal_interval(&sys->transition[i].cmd_for_place[j].guard,true_gd)==false){
				if(ist_is_unbounded(&sys->transition[i].cmd_for_place[j].guard)==true)
					printf("X%lu in [%ld,INF] ",j+1,sys->transition[i].cmd_for_place[j].guard.Left);
				else
					printf("X%lu in [%ld,%ld] ",j+1,sys->transition[i].cmd_for_place[j].guard.Left,sys->transition[i].cmd_for_place[j].guard.Right);
			}
		}
		printf("\n");
		for(j=0;j<sys->limits.nbr_variables;++j) {
			if(sys->transition[i].cmd_for_place[j].delta !=0)
				printf("delta(X%lu)=%3d ",j+1,sys->transition[i].cmd_for_place[j].delta);
		}
		printf("\n");
	}
}
