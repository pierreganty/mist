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

#include "codegenrules.h"
#include "xmalloc.h"
#include "tbsymbol.h"
#include "laparser.h"

static void rules(T_PTR_tree entry);
static void rule(T_PTR_tree entry);
static void guardedcmd(T_PTR_tree entry);
static void guard(T_PTR_tree entry);
static void cmd(T_PTR_tree entry);
static void cmdrhs(T_PTR_tree entry, T_PTR_tbsymbol_info infoid);

static transition_system_t *sys;
static size_t nbrcmd;
static size_t nbrtransfers;
static boolean istransfer;
static boolean messy_spec;

void
rulescode_produce(T_PTR_tree entry,
		  transition_system_t *system) {
  /* First memory allocation of the transition system */
  sys = system;
  sys->limits.nbr_variables = nbr_var;
  rules(entry);
}

static
void
rules(T_PTR_tree entry) {
  size_t i, j, k, nbr_rules;

  if (entry) {
	/* We allocate memory for the transitions */
	nbr_rules = tree_nbrsubtrees(entry);
	sys->limits.nbr_rules = nbr_rules;
	sys->transition = (transition_t *)xmalloc(nbr_rules*sizeof(transition_t));
	for (i = 0; i < nbr_rules; i++) {
		/* We allocate memory for each gd_cmd of each transition */
		sys->transition[i].cmd_for_place = (gd_command_t *)xmalloc(nbr_var*sizeof(gd_command_t));
		/* Default, guard = 0,\infty and delta = 0 */
		for (j = 0; j < nbr_var; j++) {
			ist_assign_values_to_interval(&sys->transition[i].cmd_for_place[j].guard, 0L,  INFINITY);
			sys->transition[i].cmd_for_place[j].delta = 0L;
			/* In the case of places merged the genuine system is the abstraction of itself */
			sys->transition[i].cmd_for_place[j].places_abstracted=1;
		}
		for (k = 0; k < MAXNBTRANS; ++k){
			/* We allocate memory for each transfert */
			sys->transition[i].transfers[k].origin = (integer16 *)xmalloc(nbr_var*sizeof(integer16));
			/* We initialize the origin vector */
			for (j = 0; j < nbr_var; j++)
				sys->transition[i].transfers[k].origin[j] = 0;
		}
	}
	for (nbrcmd = 0 ; nbrcmd < nbr_rules ; nbrcmd++) {
		rule(tree_subtree(entry,nbrcmd));
	}
  }
}

static
void
rule(T_PTR_tree entry) {
  char* info;

  if (entry) {
    info = (char*) tree_getinfo(entry);
    if (strcmp(info,"guardedcmd") == 0) {
      guardedcmd(tree_subtree(entry,0));
      guardedcmd(tree_subtree(entry,1));
    }
  }
}

static
void
guardedcmd(T_PTR_tree entry) {
	size_t i;
	char* info;
	if (entry) {
		info = (char*) tree_getinfo(entry);
		if (strcmp(info,"guard") == 0) {
			for (i = 0 ; i < tree_nbrsubtrees(entry) ; i++)
				guard(tree_subtree(entry, i));
		} else if (strcmp(info,"statement") == 0) {
			/* It's a new transition, possibly with transfers */
			nbrtransfers = 0;
			for (i = 0 ; i < tree_nbrsubtrees(entry) ; i++)
				cmd(tree_subtree(entry, i));
		}
	}
}


static
void
guard(T_PTR_tree entry) {
	T_PTR_tbsymbol_info infoid, infonb_left, infonb_right;
	char *info;

	if (entry) {
		/* If entry = TRUE, we haven't any subtrees and nothing to do OK */
		if (tree_nbrsubtrees(entry)){
			info = (char *) tree_getinfo(entry);
			infoid = tbsymbol_getinfo(tree_getinfo(tree_subtree(entry, 0)));
			infonb_left = tbsymbol_getinfo(tree_getinfo(tree_subtree(entry, 1)));
			if (strcmp(info,"=") == 0){
				ist_assign_values_to_interval(
						&sys->transition[nbrcmd].cmd_for_place[infoid->info.id.addr].guard,
						(integer32) infonb_left->info.nb.value, (integer32) infonb_left->info.nb.value);
			} else if (strcmp(info,">=") == 0){
				ist_assign_values_to_interval(
						&sys->transition[nbrcmd].cmd_for_place[infoid->info.id.addr].guard,
						(integer32) infonb_left->info.nb.value,INFINITY);
			} else if (strcmp(info,"in") == 0){
				infonb_right = tbsymbol_getinfo(tree_getinfo(tree_subtree(entry, 2)));
				ist_assign_values_to_interval(
						&sys->transition[nbrcmd].cmd_for_place[infoid->info.id.addr].guard,
						(integer32) infonb_left->info.nb.value,(integer32) infonb_right->info.nb.value);
			}
		}
	}
}


static
void
cmd(T_PTR_tree entry) {
  T_PTR_tbsymbol_info infoid;
  T_PTR_tree rhs;

  if (entry) {
    infoid = tbsymbol_getinfo(tree_getinfo(tree_subtree(entry, 0)));
    rhs = tree_subtree(entry, 1);
	/* We assume that cmd is not a transfer */
	istransfer = false;
	/* We assume the user does not know how to write specification */
	messy_spec = true;
	/* For instance X' = c s.t. c > 0 is a messy_spec
	 * we must check for it. If we meet a var in RHS or RHS = 0
	 * then messy_spec = false.
	 */
    cmdrhs(rhs, infoid);
	if (messy_spec == true)
		err_quit("\nCannot assign non null value to a variable in transition %2d\n",(nbrcmd+1));
	if (istransfer == true) {
		/* We are ready for a new transfer, pay attention to the ++ */
		sys->transition[nbrcmd].transfers[nbrtransfers].target =
			infoid->info.id.addr;
		sys->transition[nbrcmd].nbr_transfers = ++nbrtransfers;
		istransfer = false;

	}

  }
}


static
void
cmdrhs(T_PTR_tree entry, T_PTR_tbsymbol_info infoid) {
  T_PTR_tree lhs, rhs;
  T_PTR_tbsymbol_info entry_info;
  char* entry_info_char;

  if (entry) {
	  if (tree_nbrsubtrees(entry)) {

		  entry_info_char = (char*) tree_getinfo(entry);

		  lhs = tree_subtree(entry, 0);
		  rhs = tree_subtree(entry, 1);

		  if (strcmp(entry_info_char,"+") == 0) {
			  cmdrhs(lhs, infoid);
			  cmdrhs(rhs, infoid);
		  } else if (strcmp(entry_info_char,"-") == 0) {
			  cmdrhs(lhs, infoid);
			  cmdrhs(rhs, infoid);
			  /* By Grammar's definition, to the right we have a natural number
			  */
			  sys->transition[nbrcmd].cmd_for_place[infoid->info.id.addr].delta
				  = -
				  sys->transition[nbrcmd].cmd_for_place[infoid->info.id.addr].delta;
		  }
	  } else { /* Leaf */
		  entry_info = (T_PTR_tbsymbol_info)
			  tbsymbol_getinfo(tree_getinfo(entry));

		  if (entry_info->tag == tbsymbol_INFO_ID) {
			  /* At least one variable occurs in the cmd RHS */
			  messy_spec = false;
			  if (entry_info != infoid) {
				  /* It is a transfer */
				  istransfer = true;
				  if (nbrtransfers >= MAXNBTRANS) {
					  err_quit("Too many transfers for a single transition, increase the\
							  MAXNBTRANS constant.\n");
				  }
				  sys->transition[nbrcmd].transfers[nbrtransfers].origin[entry_info->info.id.addr] = 1;
			  }
		  } else if (entry_info->tag == tbsymbol_INFO_NB) {
			  sys->transition[nbrcmd].cmd_for_place[infoid->info.id.addr].delta =
				  entry_info->info.nb.value;
			  if (entry_info->info.nb.value == 0)
				  messy_spec = false;
		  } else
			  err_quit("Unrecognized tbsymbol entry\n");
	  }
  }
}
