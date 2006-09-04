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

   Copyright 2003, 2004, Pierre Ganty, Laurent Van Begin
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <getopt.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/times.h>

#include "laparser.h"
#include "ist.h"

void mist(system, initial_marking, frontier) 
	transition_system_t *system;
	ISTSharingTree *frontier, *initial_marking;
{
	ISTSharingTree *old_frontier, *temp, *reached_elem;
	size_t nbr_iteration;
	boolean Continue;
	boolean reached;
	THeadListIST List;
	
	/* Now we declare variables to measure the time consumed by the tool */
	long int tick_sec=0 ;
	struct tms before, after;
	float comp_u, comp_s ;

	/* We initialize the memory management of the system */
	ist_init_system();
	times(&before) ;

	temp = ist_remove_with_all_invar_exact(frontier, system);
	ist_dispose(frontier);
	frontier = temp;
	/* reached_elem is set to frontier that is in the "worst case" the empty IST */
	reached_elem = ist_copy(frontier);

	Continue = true;
	if (ist_is_empty(frontier) == true || ist_is_initial_marking_reached(initial_marking,reached_elem) == true) {
		Continue = false;
		puts("Unsafe region is empty or lies outside the invariants or contrains some initial states");
	}

	ist_init_list_ist(&List);
	nbr_iteration = 1;
	reached = false;
	while (Continue == true) {
		printf("\n\nIteration\t%3d\n", nbr_iteration);
		puts("Computation of the symbolic predecessors states ...");
		old_frontier = frontier;
		/* As post cond, we have that frontier is minimal (in a 1to1 comparison) w.r.t reached_elem */
		frontier = ist_pre_cone(old_frontier, reached_elem, system);

		if (ist_is_empty(frontier) == false) {

				printf("The new frontier counts :\n");
				ist_checkup(frontier);
#ifdef VERBOSE
				ist_write(frontier);
#endif
			if (ist_is_initial_marking_reached(initial_marking,frontier)) {
				puts("+-------------------------------------+");
				puts("|Initial states intersect the frontier|");
				puts("+-------------------------------------+");
				Continue = false;
				reached = true;
				ist_insert_at_the_beginning_list_ist(&List,old_frontier);
				ist_insert_at_the_beginning_list_ist(&List,frontier);
				/* 
				 * Exact subsumption test is relevant ONLY FOR INTERVALS 
				 *			} else if (ist_exact_subsumption_test(frontier,reached_elem)){
				 *				printf("reached_elem subsumes frontier \n");
				 *				Continue = false; 
				 */
			} else {
				temp = ist_remove_subsumed_paths(reached_elem, frontier);
				/* 
				 * Here we prune trivial 1to1 inclusion 
				 * We don't use simulation relations, we compute it exactly !
				 */
				ist_dispose(reached_elem);
				reached_elem = ist_union(temp, frontier);
#ifdef INTERVAL
				printf("\n\tStarting minimization of reached states\n\t");
				ist_stat(reached_elem);
				ist_minimize_tree_sim_based(reached_elem);
				printf("\t");
				ist_stat(reached_elem);
				/* 
				 * Here we prune 1toN inclusion 
				 * We use heuristic, our computation are based on the simulation relation 
				 */
#endif
				puts("After union, the reached symbolic state space is:");
				ist_checkup(reached_elem);

//				ist_dispose(old_frontier);
				ist_insert_at_the_beginning_list_ist(&List,old_frontier);
				old_frontier = frontier;
			}

		} else {
			Continue = false;
		}
		nbr_iteration++;
	}

	if (nbr_iteration != 0){
		puts("The reached symbolic state space is:");
		ist_stat(reached_elem);
#ifdef VERBOSE
		ist_write(reached_elem);
#endif
	}

	if (reached == true) {
		ist_put_path_on_screen_with_parameterized_initial_marking(initial_marking,&List,system);
	}
	
	times(&after);
	tick_sec = sysconf (_SC_CLK_TCK) ;
	comp_u = ((float)after.tms_utime - (float)before.tms_utime)/(float)tick_sec ;
	comp_s = ((float)after.tms_stime - (float)before.tms_stime)/(float)tick_sec ;
	printf("Total time of computation (user)   -> %6.3f sec.\n",comp_u);
	printf("                          (system) -> %6.3f sec.\n",comp_s);
	puts("Thanks for using this tool");

	ist_dispose(reached_elem);
	ist_dispose(initial_marking);
	ist_dispose(frontier);
	//_____ FOR cOMPLementing operation ____
	//printf("____/__/ Print old_frontier\n");
	//ist_write(old_frontier);
	//reached_elem =ist_copy(old_frontier);
	//printf("____/__/ Print complement of old_frontier\n");
	//printf("____/Checkup before determinization\n");
	//ist_checkup(old_frontier);
	//ist_determinize(old_frontier);
	//printf("____/Checkup after determinization before complementation \n");
	//ist_checkup(old_frontier);
	//ist_dump_graph("deter.dot",old_frontier);
	//ist_write(old_frontier);
	//ist_complement(old_frontier);
	//printf("____/Checkup after complementation \n");
	//ist_checkup(old_frontier);
	//ist_write(old_frontier);
	//ist_dump_graph("compl.dot",old_frontier);
	//printf("____/Check(up) empty intersection \n");
	//ist_write(ist_intersection(old_frontier,reached_elem));
	//_____ FOR cOMPLementing operation ____
}

static void print_version() {
	printf("Version %s\n", VERSION);
}

static void print_help() 
{
	puts("Usage: mist2 [options] filename\n");
	puts("Options:");
	puts("     --help, -h   this help");
	puts("     --version    show version numbers");
}

static void head_msg()
{
	puts("Copyright (C) 2002, 2003, 2004 Pierre Ganty.");
	puts("mist2 is free software, covered by the GNU General Public License, and you are");
	puts("welcome to change it and/or distribute copies of it under certains conditions.");
	puts("There is absolutely no warranty for mist2. See the COPYING for details.");
}

void mist_cmdline_options_handle(int argc, char *argv[ ]) 
{
	int c;

	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			{"help", 0, 0, 'h'},
			{"version", 0, 0, 0},
			{0, 0, 0, 0}
		};

		c = getopt_long (argc, argv, "h", long_options, &option_index);
		if (c == -1)
			break;

		switch (c)
		{
			case 0:
				if (strcmp(long_options[option_index].name,"version") == 0) {
					print_version();
					exit(EXIT_SUCCESS);
				} 
				break;

			case 'h':
				print_help();
				exit(EXIT_SUCCESS);
				break;

			default:
				print_help();
				err_quit("?? getopt returned character code 0%o ??\n", c);
		}
	}

	if (optind >= argc) {
		print_help();
		err_quit("Missing filename");
	}
}

void ic4pn(system, initial_marking, frontier) 
	transition_system_t *system;
	ISTSharingTree *frontier, *initial_marking;
{
	abstraction_t *myabs, *newabs;
	transition_system_t *sysabs;
	size_t i;

	myabs=(abstraction_t *)xmalloc(sizeof(abstraction_t));
	/* For a first test */
	myabs->nbV=1;
	myabs->A=(integer16 **)xmalloc(myabs->nbV*sizeof(integer16));
	for(i=0;i<myabs->nbV;++i)
		myabs->A[i]=(integer16 *)xmalloc(system->limits.nbr_variables*sizeof(integer16));
	myabs->A[0][0]=0;
	myabs->A[0][1]=0;
	myabs->A[0][2]=0;
	myabs->A[0][3]=1;
	myabs->A[0][4]=1;
	myabs->bound=(integer16 *)xmalloc(myabs->nbV*sizeof(integer16));
	/* For a first test */
	myabs->bound[0]=2;
	myabs->bound[1]=2;
	ist_complement(frontier,system->limits.nbr_variables);
	newabs=refine_abs(system,myabs,frontier);
}

int main(int argc, char *argv[ ])
{
  T_PTR_tree atree;
  transition_system_t *system;
  ISTSharingTree *initial_marking, *unsafe_cone;

  head_msg();
  mist_cmdline_options_handle(argc, argv);

  linenumber = 1;
  tbsymbol_init(&tbsymbol, 4096);

  printf("\n\n"); 
  printf("Parsing the problem instance.\n");
  
  my_yyparse(&atree, argv[optind++]);

#ifdef TBSYMB_DUMP  
  printf("\n\n");
  tbsymbol_dump(tbsymbol, &callback);
#endif    

#ifdef TREE_DUMP  
  printf("\n\n");
  tree_dump(atree, callback_tree_before, callback_tree_after, callback_leaf);
#endif
  
  build_problem_instance(atree, &system, &initial_marking, &unsafe_cone);
  printf(".. DONE\n");
  printf("System has %3d variables, %3d transitions and %2d actual invariants\n",system->limits.nbr_variables, system->limits.nbr_rules, system->limits.nbr_invariants);

  ic4pn(system,initial_marking,unsafe_cone);

  tbsymbol_destroy(&tbsymbol);
 

  return 0;
}
