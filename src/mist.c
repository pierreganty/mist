// vim:sw=4 ts=4:cindent
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

   Copyright 2002-2007 Pierre Ganty, Laurent Van Begin, 2014 Pedro Valero
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
#include <sys/time.h>
#include <sys/times.h>
#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/resource.h>

#include "laparser.h"
#include "ist.h"
#include "typechecking.h"
#include "debug.h"

int verbose = 0;
FILE *file;
int iterations = 1;
char description[100];

/*To end the executing when timeout event occurs*/
void timeout_func (int sgn) {
	if(file != NULL) fprintf(file, "\n%s",description);
	err_quit("Timeout");
}

/* For printing the error trace */
void ist_print_error_trace(ISTSharingTree * initial_marking,THeadListIST * list_ist, transition_system_t * rules)
{
	ISTSharingTree *S, *post, *Siter, *intersect;
	size_t i;
	boolean Continue;

	printf("Run to reach the bad states from the initial marking:\n");

	S=ist_copy(initial_marking);
	Siter = ist_first_element_list_ist(list_ist);
	while (Siter != NULL) {
		i = 0;
		Continue = true;
		while ((i < rules->limits.nbr_rules) && (Continue == true)) {
			post = ist_enumerative_post_transition(S,rules,i);
			intersect = ist_intersection(post,Siter);
			ist_dispose(post);
			if (!ist_is_empty(intersect)) {
				printf("[%d> ",i);
				ist_dispose(S);
				S = intersect;
				Continue = false;
			} else {
				ist_dispose(intersect);
				++i;
			}
		}
		if (i == rules->limits.nbr_rules) {
			err_msg("\nError: No Path Found!\n");
			Siter = NULL;
		} else
			Siter = ist_next_element_list_ist(list_ist);
	}
	printf("\n");
	ist_dispose(S);
}

int * compute_bound_from_ucs(ISTSharingTree * S) {
	int nblayer = 0;
	ISTLayer * layer;
	int* result;
	int i;

	for(layer = S->FirstLayer; layer->Next != NULL; nblayer++,layer = layer->Next);

	result = (int *)xmalloc(nblayer * sizeof(int));
	for(i=0,layer=S->FirstLayer ; i<nblayer ; i++,layer=layer->Next) {
		result[i] = layer->LastNode->Info->Left;
	}

	return result;
}


ISTSharingTree *backward(system, initial_marking, frontier, bounds)
	transition_system_t *system;
	ISTSharingTree *frontier, *initial_marking;
	int *bounds;
{
	ISTSharingTree *old_frontier, *temp, *reached_elem;
	size_t nbr_iteration;
	boolean Continue;
	boolean reached;
	THeadListIST List;
	int *new_bounds, i;

	/* Now we declare variables to measure the time consumed by the function */
	long int tick_sec=0 ;
	struct tms before, after;
	float comp_u, comp_s ;

	times(&before) ;

	//we test if we reached the fixpoint
	ISTSharingTree *a=ist_pre(frontier,system);
	a = ist_remove_subsumed_paths(a,frontier);
	if (ist_is_empty(a) == true)
		puts("<<<<<<<<<<<<<<<<<<<<< fix point >>>>>>>>>>>>>>>>>>>>>>");
	else
		puts("<<<<<<<<<<<<<<<<<<<<< not fix point >>>>>>>>>>>>>>>>>>");
	ist_checkup(frontier);




//	temp = ist_remove_with_all_invar_exact(frontier, system);
	/* We de not call ist_dispose(frontier); since we do not want
	 * to modify the IN parameter */
//	frontier = temp;
	/* reached_elem is set to frontier that is in the "worst case" the empty IST */
	reached_elem = ist_copy(frontier);
	//we copy frontier, since we do not want to dispose the parameter
	frontier = ist_copy(frontier);

	Continue = true;
	temp=ist_intersection(initial_marking,reached_elem);
	reached = (ist_is_empty(temp) == true ? false : true);
	ist_dispose(temp);
	if ( reached == true) {
		Continue = false;
	}
	ist_init_list_ist(&List);
	nbr_iteration = 1;
	while (Continue == true) {
		printf("\n\nIteration\t%3d\n", nbr_iteration);
		puts("Computation of the symbolic predecessors states ...");
		old_frontier = frontier;
		/* As post cond, we have that frontier is minimal (in a 1to1 comparison) w.r.t reached_elem */
		frontier = //ist_pre_pruned_wth_inv_and_prev_iterates(old_frontier, reached_elem, system);
				ist_pre(old_frontier,system);
		ist_dispose(old_frontier);

		/* il faut reduire par rapport a reached elem */
		temp = ist_remove_subsumed_paths(frontier,reached_elem);
		ist_dispose(frontier);
		frontier = temp;

		if (ist_is_empty(frontier) == false) {

			printf("The new frontier counts :\n");
			ist_checkup(frontier);
			PRINT_IST(frontier);
//			temp=ist_intersection(initial_marking,frontier);
//			reached = (ist_is_empty(temp) == true ? false : true);
//			ist_dispose(temp);
//			if (reached == true) {
			if(false) {
				puts("+-------------------------------------+");
				puts("|Initial states intersect the frontier|");
				puts("+-------------------------------------+");
				Continue = false;
				ist_insert_at_the_beginning_list_ist(&List,old_frontier);
				/*
				 * Exact subsumption test is relevant ONLY FOR INTERVALS
				 *			 else if (ist_exact_subsumption_test(frontier,reached_elem))
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
				/* To minimize we can use:
				 * ist_minimal_form or ist_minimal_form_sim_based
				 */
				puts("After union, the reached symbolic state space is:");
				ist_checkup(reached_elem);
				//ist_insert_at_the_beginning_list_ist(&List,old_frontier);
				old_frontier = frontier;
			}
			/* We test if we must stop because we find new bounds*/
			new_bounds = compute_bound_from_ucs(reached_elem);

			for(i=0 ; (i< system->limits.nbr_variables) && (new_bounds[i] == bounds[i]) ; i++);

			if (i!=system->limits.nbr_variables)
				Continue = false;
			else
				xfree(new_bounds);

		} else  {
			Continue = false;
			new_bounds = compute_bound_from_ucs(reached_elem);
		}
		nbr_iteration++;
	}

	ist_dispose(frontier);

	if (nbr_iteration != 0){
		puts("The reached symbolic state space is:");
		ist_stat(reached_elem);
		PRINT_IST(reached_elem);
	}
	if (reached == true)
		ist_print_error_trace(initial_marking,&List,system);
	ist_empty_list_ist_with_info(&List);
	times(&after);
	tick_sec = sysconf (_SC_CLK_TCK) ;
	comp_u = ((float)after.tms_utime - (float)before.tms_utime)/(float)tick_sec ;
	comp_s = ((float)after.tms_stime - (float)before.tms_stime)/(float)tick_sec ;
	printf("Total time of computation (user)   -> %6.3f sec.\n",comp_u);
	printf("                          (system) -> %6.3f sec.\n",comp_s);

	//ist_dispose(reached_elem);

	/* Is the system safe ? */
	return (reached_elem);
}

void backward_basic(system, initial_marking, frontier)
	transition_system_t *system;
	ISTSharingTree *frontier, *initial_marking;
{
	ISTSharingTree *old_frontier=NULL, *temp, *reached_elem;
	size_t nbr_iteration;
	boolean Continue;
	boolean reached;
	THeadListIST List;

	/* Now we declare variables to measure the time consumed by the function */
	long int tick_sec=0 ;
	struct tms before, after;
	float comp_u, comp_s ;

	struct rusage beforeIt, afterIt;
    float diff;

	times(&before);
	getrusage(RUSAGE_SELF, &beforeIt);


	// Auxiliar variable for the graphs
	int last_print=0;

	temp = ist_remove_with_all_invar_exact(frontier, system);
	/* We de not call ist_dispose(frontier); since we do not want
	 * to modify the IN parameter */
	frontier = temp;
	/* reached_elem is set to frontier that is in the "worst case" the empty IST */
	reached_elem = ist_copy(frontier);

	Continue = true;
	temp=ist_intersection(initial_marking,reached_elem);
	reached = (ist_is_empty(temp) == true ? false : true);
	ist_dispose(temp);
	if (ist_is_empty(frontier) == true ||  reached == true) {
		Continue = false;
		puts("Unsafe region is empty or lies outside the invariants or contains some initial states");
	}
	ist_init_list_ist(&List);
	nbr_iteration = 1;

	if (file != NULL) fprintf(file, "Iterations,Frontier,Total elems,Time\n");

	while (Continue == true) {
		printf("\n\nIteration\t%3d\n", nbr_iteration);
		puts("Computation of the symbolic predecessors states ...");
		old_frontier = frontier;
		/* As post cond, we have that frontier is minimal (in a 1to1 comparison) w.r.t reached_elem */
		frontier = ist_pre_pruned_wth_inv_and_prev_iterates(old_frontier, reached_elem, system);

		if (ist_is_empty(frontier) == false) {
			last_print = 0;
			printf("The new frontier counts :\n");
			ist_checkup(frontier);
			if (file != NULL){
				fprintf(file, "%d,", nbr_iteration);
				ist_stat_plot(frontier, file);
				fprintf(file, ",");
			}
			PRINT_IST(frontier);
			temp=ist_intersection(initial_marking,frontier);
			reached = (ist_is_empty(temp) == true ? false : true);
			ist_dispose(temp);
			if (reached == true) {
				Continue = false;
				ist_insert_at_the_beginning_list_ist(&List,old_frontier);
				/*
				 * Exact subsumption test is relevant ONLY FOR INTERVALS
				 *			 else if (ist_exact_subsumption_test(frontier,reached_elem))
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
				ist_dispose(temp);
				/* To minimize we can use:
				 * ist_minimal_form or ist_minimal_form_sim_based
				 */
				puts("After union, the reached symbolic state space is:");
				last_print = 1;
				ist_checkup(reached_elem);
				if (file != NULL){
					ist_stat_plot(reached_elem, file);
					getrusage(RUSAGE_SELF, &afterIt);
					diff = ((float)afterIt.ru_utime.tv_usec +(float)afterIt.ru_utime.tv_sec*1000000 - (float)beforeIt.ru_utime.tv_sec*1000000- (float)beforeIt.ru_utime.tv_usec);
					if(file != NULL) fprintf(file, ",\t %f", ((float) diff / (float)1000));
					getrusage(RUSAGE_SELF, &beforeIt);
					fprintf(file, "\n");
				}
				ist_insert_at_the_beginning_list_ist(&List,old_frontier);
				old_frontier = frontier;
			}

		} else
			Continue = false;
		nbr_iteration++;
	}

	if (nbr_iteration != 0){
		puts("The reached symbolic state space is:");
		ist_stat(reached_elem);
		if (file != NULL && last_print == 0){
			ist_stat_plot(reached_elem, file);
			fprintf(file, "\n");
		}
		PRINT_IST(reached_elem);
	}
	if (reached == true)
		ist_print_error_trace(initial_marking,&List,system);
	ist_empty_list_ist_with_info(&List);
	times(&after);
	tick_sec = sysconf (_SC_CLK_TCK) ;
	comp_u = ((float)after.tms_utime - (float)before.tms_utime)/(float)tick_sec ;
	comp_s = ((float)after.tms_stime - (float)before.tms_stime)/(float)tick_sec ;
	printf("Total time of computation (user)   -> %6.3f sec.\n",comp_u);
	printf("                          (system) -> %6.3f sec.\n",comp_s);
	if(file != NULL) fprintf(file, "User: %6.3f sec\t System: %6.3f sec\n", comp_u, comp_s);

	ist_dispose(reached_elem);
 	/* old_frontier should be disposed, when the loop is exited but it wasn't added to the error trace */
   	if(ist_is_empty(frontier)) ist_dispose(old_frontier);

	/* Is the system safe ? */
	if (reached == true){
		puts("backward algorithm concludes unsafe");
		if(file != NULL) fprintf(file, "backward: unsafe");
	}
	else{
		puts("backward algorithm concludes safe");
		if(file != NULL) fprintf(file, "backward: safe");
	}
}

static void print_version() {
	printf("Version %s\n", VERSION);
}

/*
	This function shows the output according to the sintax described at:
	http://docopt.org/

*/
static void print_help()
{
	puts("Usage: ");
	puts("     mist --algorithm <filename> [option]...");
	puts("     mist --help");
	puts("     mist --version");
	puts(" ");
	puts("Algorithm:");
	puts("     backward       the backward algorithm with invariant pruning");
	puts("     ic4pn          the algorithm described in FI");
	puts("     tsi            the algorithm described in TSI");
	puts("     eec            the Expand, Enlarge and Check algorithm");
	puts("     cegar          the Expand, Enlarge and Check algorithm with counterexample guided refinement");
	puts("");
	puts("Options:");
	puts("     --timeout <T>      establish an execution timeout of T seconds");
	puts("     --verbose <V>      establish a verbose level V");
	puts("     --graph <filename> generate filename.csv which contains the needed data to plot graphs of the memory usage. Graphs could be generated by using generate_graphs.sh");
	puts("     --help           this help");
	puts("     --version        show version number");
}

static void head_msg()
{
	puts("Copyright (C) 2002-2015 Pierre Ganty, 2003-2008 Laurent Van Begin, 2014-2015 Pedro Valero.");
	puts("mist is free software, covered by the GNU General Public License, and you are");
	puts("welcome to change it and/or distribute copies of it under certain conditions.");
	puts("There is absolutely no warranty for mist. See the COPYING for details.");
}

/*
 * apply the (0,...,k,INFINITY) abstraction.
 * POST: for each layer the list of nodes remains sorted.
 */
void abstract_bound(ISTSharingTree *S, integer16 *bound)
{
	ISTLayer *L;
	ISTNode *N;
	size_t i;

	for(L = S->FirstLayer, i = 0 ; L != S->LastLayer ; L = L->Next, i++)
		for(N = L->FirstNode ; N != NULL ; N = N->Next)
			if (!ist_less_or_equal_value(N->Info->Right,bound[i]))
				N->Info->Right=INFINITY;
}

void bound_values(ISTSharingTree *S, integer16 *bound)
{
	ISTLayer *L;
	ISTNode *N;
	size_t i;

	for(i=0,L = S->FirstLayer;L != S->LastLayer;i++,L = L->Next)
		for(N= L->FirstNode;N != NULL;N = N->Next)
			ist_assign_values_to_interval(N->Info,min(N->Info->Left,bound[i]),min(N->Info->Right,bound[i]));
}




/*
 * lfp is a out parameter which is
 * 1) an inductive invariant of the system
 * 2) a dc-set
 * WORKS FOR PETRI NET ONLY
 */
boolean eec_fp(system, abs, initial_marking, bad, lfp)
	transition_system_t *system;
	abstraction_t *abs; /* For the bounds on the places */
	ISTSharingTree *initial_marking, *bad, **lfp;
{
	boolean retval;
	ISTSharingTree *abs_post_star, *inter, *downward_closed_initial_marking, *bpost, *tmp, *_tmp;
	boolean finished;
	size_t i;

	float comp_u,comp_s;
	long int tick_sec=0 ;
	struct tms before, after;

	//int *new_bound;

	times(&before);

	/*initialisation des bornes */
	//new_bound = compute_bound_from_ucs(bad);
	for(i = 0; i < system->limits.nbr_variables; i++)
		abs->bound[i] = 1;
	/* fin de l'initialisation des bornes */


	downward_closed_initial_marking = ist_downward_closure(initial_marking);
	ist_normalize(downward_closed_initial_marking);
	assert(ist_checkup(downward_closed_initial_marking)==true);

	finished=false;
	while (finished == false) {
		printf("eec: ENLARGE begin\t\n");
		if (file != NULL) fprintf(file, "1, \t 0.2, \t 0\n");
		fflush(NULL);

		/* To OVERapproximate we use abstract_bound */
		/* Do not use ist_abstract_post_star_until_reach_bad it produces incorrect results/non termination.
		abs_post_star = ist_abstract_post_star_until_reach_bad(downward_closed_initial_marking,abstract_bound,abs->bound,system,bad);
		*/
		abs_post_star = ist_abstract_post_star(downward_closed_initial_marking,abstract_bound,abs->bound,system);
//		assert(ist_checkup(abs_post_star)==true);
		ist_write(abs_post_star);
		puts("end");
		*lfp = abs_post_star;
		inter = ist_intersection(abs_post_star,bad);
		finished=ist_is_empty(inter);
		ist_dispose(inter);
		if (finished==true) {
			/* finished==true -> the system is safe */
//			ist_write(*lfp);
//			ist_write(bad);
			retval = true;
		} else {
			printf("eec: EXPAND begin\t");
			if (file != NULL) fprintf(file, "1,\t 0.1,\t 0\n");
			fflush(NULL);
			/* use bpost = ist_abstract_post_star(downward_closed_initial_marking,bound_values,abs->bound,system)
			 * if you want to compute the lfp. Instead we make something more
			 * efficient by testing, at each iteration, for the emptiness of
			 * intersection w/ bad. */

			bpost = ist_copy(downward_closed_initial_marking);
			bound_values(bpost,abs->bound);
			ist_normalize(bpost);
			inter=ist_intersection(bpost,bad);
			finished= ist_is_empty(inter) == true ? false : true;
			ist_dispose(inter);
			while (finished==false) {
				if(file != NULL) fprintf(file, "%d,",iterations++);
				tmp = ist_abstract_post(bpost,bound_values,abs->bound,system);
				if(file != NULL) fprintf(file, "\n");

				//tmp = ist_abstract_post_transtree(bpost,bound_values,abs->bound,system);
				_tmp =  ist_remove_subsumed_paths(tmp,bpost);
				ist_dispose(tmp);
				if (ist_is_empty(_tmp)==false) {
					inter=ist_intersection(_tmp,bad);
					finished=ist_is_empty(inter) == true ? false : true;
					ist_dispose(inter);
					tmp = ist_remove_subsumed_paths(bpost,_tmp);
					ist_dispose(bpost);
					bpost = ist_union(tmp,_tmp);
					ist_dispose(tmp);
					ist_dispose(_tmp);
				} else {
					ist_dispose(_tmp);
					break;
				}
			}
			assert(ist_checkup(bpost)==true);
			ist_dispose(bpost);
			puts("end");
			if (finished==true)
				/* finished==true -> we hitted the bad states, the system is unsafe */
				retval=false;
			else {
				/* One more iteration is needed because both bpost and
				 * abs_post_star does not allow to conclude */
				ist_dispose(abs_post_star);
				printf("eec: BOUNDS\t");
				for(i=0;i<abs->nbV;printf("%d ",++abs->bound[i++]));
				/* nouvelle methode de calcul des bornes */
				printf("\n");
				printf("New bounds");
				print_abstraction(abs);
			}
		}
	}
	ist_dispose(downward_closed_initial_marking);

	times(&after);
	tick_sec = sysconf (_SC_CLK_TCK) ;
	comp_u = ((float)after.tms_utime - (float)before.tms_utime)/(float)tick_sec ;
	comp_s = ((float)after.tms_stime - (float)before.tms_stime)/(float)tick_sec ;
	printf("EEC time of computation (user)   -> %6.3f sec.\n",comp_u);
	printf("                          (system) -> %6.3f sec.\n",comp_s);
	if(file != NULL) fprintf(file, "User: %6.3f sec\t System: %6.3f sec\n", comp_u, comp_s);

	return retval;
}

/*
 * List is a out parameter from which we
 * will extract the counterexample
 * WORKS FOR PETRI NET ONLY
 */
boolean eec_cegar(system, abs, initial_marking, bad, List)
	transition_system_t *system;
	abstraction_t *abs; /* For the bounds on the places */
	ISTSharingTree *initial_marking, *bad;
	THeadListIST *List;
{
	boolean retval;
	ISTSharingTree *abs_post_star, *inter, *downward_closed_initial_marking, *bpost, *tmp, *_tmp;
	boolean finished;
	size_t i;

	float comp_u,comp_s;
	long int tick_sec=0 ;
	struct tms before, after;

	times(&before);

	downward_closed_initial_marking=ist_downward_closure(initial_marking);
	ist_normalize(downward_closed_initial_marking);
	assert(ist_checkup(downward_closed_initial_marking)==true);


	finished=false;
	while (finished == false) {
		printf("eec: ENLARGE begin\t");
		if (file != NULL) fprintf(file, "1, \t 0.2,\t 0\n");
		fflush(NULL);
		/* To OVERapproximate we use abstract_bound */
		abs_post_star = ist_abstract_post_star(downward_closed_initial_marking,abstract_bound,abs->bound,system);
		assert(ist_checkup(abs_post_star)==true);
		puts("end");
		inter = ist_intersection(abs_post_star,bad);


		ist_dispose(abs_post_star);
		finished=ist_is_empty(inter);
		ist_dispose(inter);
		if (finished==true)
			/* finished==true -> the system is safe */
			retval = true;
		else {
			/* Pay attention here, initial_marking might contain infinitely
			 * many markings; so in the expand phase we work with a finite
			 * subset of it. */
			printf("eec: EXPAND begin\t");
			if (file != NULL) fprintf(file, "1,\t 0.1, \t 0\n");
			fflush(NULL);
			ist_init_list_ist(List);

			/* expand works w/ dc-sets */
			bpost = ist_copy(downward_closed_initial_marking);
			bound_values(bpost,abs->bound);
			ist_normalize(bpost);
			inter=ist_intersection(bpost,bad);
			finished= ist_is_empty(inter) == true ? false : true;
			ist_dispose(inter);
			/* insert a copy of initial_marking in the list */
			ist_insert_at_the_beginning_list_ist(List,ist_copy(initial_marking));
			while (finished==false) {
				if(file != NULL) fprintf(file, "%d,",iterations++);
				tmp = ist_abstract_post(bpost,bound_values,abs->bound,system);
				if(file != NULL) fprintf(file, "\n");
				_tmp =  ist_remove_subsumed_paths(tmp,bpost);
				ist_dispose(tmp);
				if (ist_is_empty(_tmp)==false) {
					/* insert a copy of _tmp in the list */
					ist_insert_at_the_beginning_list_ist(List,ist_copy(_tmp));
					inter=ist_intersection(_tmp,bad);
					finished=!ist_is_empty(inter);
					ist_dispose(inter);
					tmp=ist_remove_subsumed_paths(bpost,_tmp);
					ist_dispose(bpost);
					bpost = ist_union(tmp,_tmp);
					ist_dispose(tmp);
					ist_dispose(_tmp);
				} else {
					// we reached a fixpoint, so exit of the loop
					ist_dispose(_tmp);
					break;
				}
			}
			assert(ist_checkup(bpost)==true);
			ist_dispose(bpost);
			puts("end");
			if (finished==true)
				/* finished==true -> we hitted the bad states, the system is unsafe */
				retval=false;
			else {
				/* One more iteration is needed because both bpost and
				 * abs_post_star does not allow to conclude.
				 * Free the list of sharing tree (here info means st) */
				ist_empty_list_ist_with_info(List);
				printf("eec: BOUNDS\t");
				for(i=0;i<abs->nbV;printf("%d ",++abs->bound[i++]));
				printf("\n");
			}
		}
	}
	ist_dispose(downward_closed_initial_marking);

	times(&after);
	tick_sec = sysconf (_SC_CLK_TCK) ;
	comp_u = ((float)after.tms_utime - (float)before.tms_utime)/(float)tick_sec ;
	comp_s = ((float)after.tms_stime - (float)before.tms_stime)/(float)tick_sec ;
	printf("Total time of computation (user)   -> %6.3f sec.\n",comp_u);
	printf("                          (system) -> %6.3f sec.\n",comp_s);
	if(file != NULL) fprintf(file, "User: %6.3f sec\t System: %6.3f sec\n", comp_u, comp_s);

	return retval;
}

void cegar(system, initial_marking, bad)
	transition_system_t *system;
	ISTSharingTree *bad, *initial_marking;
{
	abstraction_t *myabs, *newabs, *abs_tmp;
	transition_system_t *sysabs = NULL;
	ISTSharingTree *tmp, *_tmp, *alpha_bad, *predecessor, *inter = NULL, *seed, *cutter, *Z;
	ISTLayer *layer;
	ISTNode *node;
	THeadListIST cex;
	size_t i, nb_iteration, lg_cex;
	int *countex, j;
	boolean out, conclusive, eec_conclusive;

	if (file != NULL) fprintf(file, "Iterations,Total elems,Time\n");

	printf("CEGAR..\n");
	tmp=ist_intersection(initial_marking,bad);
	conclusive = (ist_is_empty(tmp)==true ? false : true);
	ist_dispose(tmp);

	// since new_abstraction works w/ dc-sets we compute \neg bad
	tmp=ist_copy(bad);
	ist_complement(tmp,system->limits.nbr_variables);
	// _tmp is a dc-set but we want each path to be dc-closed
	_tmp=ist_downward_closure(tmp);
	ist_normalize(_tmp);
	ist_dispose(tmp);

	/* the initial abstraction is given by refinement(_tmp) */
	myabs=new_abstraction_dc_set(_tmp,system->limits.nbr_variables);

	nb_iteration=0;
	while(conclusive == false) {
		puts("begin of iteration");
		// We build the abstract system
		sysabs=build_sys_using_abs(system,myabs);
		puts("The current abstraction is :");
		print_abstraction(myabs);
		//puts("The current abstracted net is:");
		//print_transition_system(sysabs);
		//Set tmp=alpha(initial_marking), alpha_bad
		tmp = ist_abstraction(initial_marking,myabs);
		alpha_bad = ist_abstraction(bad,myabs);

		eec_conclusive=eec_cegar(sysabs, myabs, tmp, alpha_bad, &cex);
		ist_dispose(tmp);

		if (eec_conclusive==true) {
			// says "safe" because it is indeed safe
			puts("EEC concludes safe with the abstraction");
			if (file != NULL) fprintf(file, "cegar: safe");
			print_abstraction(myabs);
			conclusive = true;
		} else {
			/* compute the length of cex */
			lg_cex=ist_count_elem_list_ist(&cex)-1;
			printf("run to bad is %d transition(s) long.\n",lg_cex);
			tmp=ist_first_element_list_ist(&cex);

			/* seed is neither a dc-set, nor a uc-set */
			seed=ist_intersection(alpha_bad,tmp);
			/* so we compute the uc_closure. This should not break the
			 * invariant that nodes, sons, etc are ordered. */
			layer=seed->FirstLayer;
			while(layer!=seed->LastLayer) {
				node=layer->FirstNode;
				while(node!=NULL){
					node->Info->Right=INFINITY;
					node=node->Next;
				}
				layer=layer->Next;
			}
			ist_normalize(seed);

			/* extract the cex */
			countex=(int *)xmalloc(lg_cex*sizeof(int));

			puts("seed of the abstract cex (backward analysis)");
			//ist_write(seed);
			_tmp=seed;

			for(j=0;j<lg_cex;j++) {
				i=0;
				out=false;
				/* Take the next elem in the list */
				cutter=ist_next_element_list_ist(&cex);
				while((i < sysabs->limits.nbr_rules) && (out == false)) {
					/* computes the minpre[transition[i]] */
					predecessor=ist_pre_of_rule_plus_transfer(_tmp,&sysabs->transition[i]);
					/* as specified in ist_pre_of_rule_plus_transfer the result is not
					 * supposed to be in normal form. */
					if (!ist_is_empty(predecessor))
						ist_normalize(predecessor);
					/* and intersect with the dc-set computed during expand */
					inter=ist_intersection(cutter,predecessor);
					assert(ist_checkup(inter)==true);
					ist_dispose(predecessor);
					if (!ist_is_empty(inter)) {
						/* Compute the uc-closure of inter. This should not
						 * break the invariant that nodes, sons, etc are
						 * ordered. */
						layer=inter->FirstLayer;
						while(layer!=inter->LastLayer) {
							node=layer->FirstNode;
							while(node!=NULL){
								node->Info->Right=INFINITY;
								node=node->Next;
							}
							layer=layer->Next;
						}
						ist_normalize(inter);
						countex[j]=i;
						//ist_write(inter);
						ist_dispose(_tmp);
						_tmp=inter;
						out=true;
					} else {
						ist_dispose(inter);
						++i;
					}

				}
			}
			/* release the list of dc-sets from which the cex is extracted. this list is now useless */
			ist_empty_list_ist_with_info(&cex);
			puts("The counter example for the abstract net");
			for(j=0;j<lg_cex;printf("<%d]",countex[j++]));
			printf("\n");

			/* We have countex, we need a seed which is obtained by replaying
			 * forward the countex starting from inter.  Inter is a uc-set
			 * whose minimal elements are given by the intersection of
			 * initial_marking with the uc-set computed previously. Inter
			 * remains uc-closed after firing the transitions of countex. */
			tmp=inter;
			j=lg_cex-1;
			while(j>=0) {
				_tmp=ist_enumerative_post_transition(tmp,sysabs,countex[j--]);
				ist_dispose(tmp);
				tmp=_tmp;
			}
			/* we take the minimal elements of the outcoming uc-set */
			layer=tmp->FirstLayer;
			while(layer!=tmp->LastLayer) {
				node=layer->FirstNode;
				while(node!=NULL){
					node->Info->Right=node->Info->Left;
					node=node->Next;
				}
				layer=layer->Next;
			}
			ist_normalize(tmp);

			/* seed=gamma(tmp); then take intersection with bad */
			seed=ist_concretisation(tmp,myabs);
			predecessor=ist_intersection(seed,bad);
			puts("seed of the concrete cex (backward analysis)");
			assert(ist_checkup(predecessor)==true);

			/* analysis of the counter example in the concrete */
			j=0;
			do {
				ist_dispose(tmp);
				tmp=predecessor;
				predecessor=ist_symbolic_pre_of_rule(tmp, &system->transition[countex[j++]]);
			} while(!ist_is_empty(predecessor) && j<lg_cex);
			Z=NULL;
			/* if we went through the whole counterexample then we found a real counterexample */
			if(!ist_is_empty(predecessor)) {
				_tmp=ist_intersection(initial_marking,predecessor);
				conclusive=!ist_is_empty(_tmp);
				ist_dispose(_tmp);
				if (conclusive==true) {
					puts("Find a real counterexample");
					for(j=0;j<lg_cex;printf("<%d]",countex[j++]));
					printf("\n");
				} else
					Z=ist_copy(predecessor);
			} else
				Z=ist_copy(tmp);
			/* release predecessor, tmp */
			ist_dispose(predecessor);
			ist_dispose(tmp);
			// Z is the last non empty pre_computation
			// We refine by representing exactly Z; Z are bad
			// states in the sense of CEGAR (i.e. unreachable but lead to bad)
			if(Z!=NULL) {
				puts("Z is");
				assert(ist_checkup(Z)==true);
				/* new_abstraction_finite_set is tailored for Z which is finite */
				puts("building new absraction");
				abs_tmp = new_abstraction_finite_set(Z,system->limits.nbr_variables);
				puts("abs_tmp");
				print_abstraction(abs_tmp);
				newabs = glb(abs_tmp,myabs);
				// release abs_tmp
				dispose_abstraction(abs_tmp);
				// release myabs
				dispose_abstraction(myabs);
				myabs = newabs;
			}
		}
	}
	// release sysabs
	dispose_transition_system(sysabs);
	printf("end of iteration %d\n",++nb_iteration);
}

//pre^*, we use the complement of the least fixpoint in pre^*, and we compute an abstract greatest fixpoint
void ic4pn(system, initial_marking, bad)
	transition_system_t *system;
	ISTSharingTree *bad, *initial_marking;
{
	abstraction_t *myabs, *newabs;
	transition_system_t *sysabs;
	ISTSharingTree *tmp, *_tmp, *a_neg_Z, *alpha_initial_marking, *neg_Z,\
		*inter, *frontier, *lfp, *old_lfp /* used to ensure properties of the checker */;
	size_t i,j,nb_iteration;
	boolean *maskpre, *maskpost, conclusive, eec_conclusive;


        // initialization of old_lfp used to ensure the properties of the checker
        old_lfp = NULL;

	/* Set constant abstraction top (all places merged) */
	abstraction_t *topabs;
	topabs=(abstraction_t *)xmalloc(sizeof(abstraction_t));
	topabs->nbConcreteV=system->limits.nbr_variables;
	topabs->nbV=1;
	topabs->bound=(integer16 *)xmalloc(topabs->nbV*sizeof(integer16));
	topabs->A=(integer16 **)xmalloc(topabs->nbV*sizeof(integer16 *));
	for(i=0;i<topabs->nbV;++i) {
		topabs->A[i]=(integer16 *)xmalloc(system->limits.nbr_variables*sizeof(integer16));
		topabs->bound[i]=1;
		for(j=0;j<topabs->nbConcreteV;++j)
			topabs->A[i][j]=1;
	}
	/* for xrealloc to behave correctly */
	maskpost=maskpre=NULL;
	/* printf("EEC for the concrete system\n");
	eec_conclusive=eec_fp(system, bottomabs, initial_marking, bad, &lfp);
	if (eec_conclusive == true)
		printf("Answer = true\n");
	else
		printf("Answer = false\n"); */
	puts("IC4PN..");

	neg_Z = ist_copy(bad);

	tmp=ist_intersection(initial_marking,bad);
	conclusive = (ist_is_empty(tmp)==true ? false : true);
	ist_dispose(tmp);

	/* A_0 = refinement(Z_0) */
	myabs=new_abstraction_lub(neg_Z,system->limits.nbr_variables,topabs);
	puts("first abstraction");
	print_abstraction(myabs);
	dispose_abstraction(topabs);

	nb_iteration=0;
	if (file != NULL) fprintf(file, "Iterations,Total elems,Time\n");
	while(conclusive == false) {
		puts("begin of iteration");
		// We build the abstract system
		sysabs=build_sys_using_abs(system,myabs);
		/* mask for post (for EEC) */
		maskpost=(boolean *)xrealloc(maskpost, sysabs->limits.nbr_rules*sizeof(boolean));
		for(i=0;i<sysabs->limits.nbr_rules;++i)
			maskpost[i]=true;
		from_transitions_to_tree(sysabs, maskpost);
//		ist_stat(sysabs->tree_of_transitions);
//		ist_write(sysabs->tree_of_transitions);

		puts("The current abstraction is :");
		print_abstraction(myabs);
		// Set a_neg_Z, alpha_initial_marking
		a_neg_Z = ist_abstraction(neg_Z,myabs);
		ist_dispose(neg_Z);
		assert(ist_checkup(a_neg_Z)==true);
		alpha_initial_marking = ist_abstraction(initial_marking,myabs);
		assert(ist_checkup(alpha_initial_marking)==true);

		eec_conclusive=eec_fp(sysabs, myabs, alpha_initial_marking, a_neg_Z, &lfp);

		if (eec_conclusive==true) {
			// says "safe" because it is indeed safe
			puts("EEC concludes safe with the abstraction");
			if(file != NULL) fprintf(file, "EEC: safe");
			print_abstraction(myabs);
			conclusive = true;
		} else {

			//here we must add intersection to ensure properties of the checker
			if (nb_iteration > 0) {
				tmp = ist_abstraction(old_lfp,myabs);
				ist_dispose(old_lfp);
				old_lfp = tmp;

				tmp = ist_intersection(lfp,old_lfp);
				ist_dispose(lfp);
				ist_dispose(old_lfp);
				lfp = tmp;
				old_lfp = ist_concretisation(lfp,myabs);

			} else
				old_lfp = ist_concretisation(lfp,myabs);
			////////////////////////////////////////////////////////////////////

			tmp = adhoc_pre_star_pruned_unless_hit_m0(a_neg_Z, lfp, sysabs, alpha_initial_marking);

			if (tmp == NULL) {
				conclusive = true;
				puts("Reachable (adhoc_pre_star_unless_hit_m0 concludes)");
			} else {
				frontier = ist_concretisation(tmp,myabs);
				ist_dispose(tmp);

				tmp = ist_pre(frontier,system);
				/* Should we prune it ist_concretisation(lfp,myabs) ? */

				_tmp = ist_union(tmp,frontier);
				ist_dispose(tmp);
				ist_dispose(frontier);
				neg_Z = ist_minimal_form(_tmp);

				inter = ist_intersection(initial_marking,neg_Z);
				if (ist_is_empty(inter) == false) {
					conclusive = true;
					puts("Reachable (neg_Z concludes)");
				} else {
					newabs = new_abstraction_lub(neg_Z,system->limits.nbr_variables,myabs);
					dispose_abstraction(myabs);
					myabs = newabs;

//					puts("myabs");
//					print_abstraction(myabs);

				}
				ist_dispose(inter);
			}
		}
		// release the parameters of eec
		ist_dispose(alpha_initial_marking);
		ist_dispose(a_neg_Z);
		ist_dispose(lfp);
		// release sysabs
		dispose_transition_system(sysabs);
		printf("end of iteration %d\n",++nb_iteration);
	}
	// release abstraction
	dispose_abstraction(myabs);
}

/*
 * TSI
 */
boolean eec_bound(system, abs, initial_marking, bad, lfp)
	transition_system_t *system;
	abstraction_t *abs; /* For the bounds on the places */
	ISTSharingTree *initial_marking, *bad, **lfp;
{
	boolean retval;
	ISTSharingTree *abs_post_star, *inter, *downward_closed_initial_marking, *bpost, *tmp, *new_bad;
	boolean finished;
	size_t i;

	float comp_u,comp_s;
	long int tick_sec=0 ;
	struct tms before, after;

	int *new_bound;

	times(&before);

	/*initialisation des bornes */
	new_bound = compute_bound_from_ucs(bad);
	for(i = 0; i < system->limits.nbr_variables; i++)
		abs->bound[i] = new_bound[i];
	print_abstraction(abs);
	/* fin de l'initialisation des bornes */


	downward_closed_initial_marking = ist_downward_closure(initial_marking);
	ist_normalize(downward_closed_initial_marking);
//	assert(ist_checkup(downward_closed_initial_marking)==true);

	finished=false;
	while (finished == false) {
		printf("eec: ENLARGE begin\t\n");
		if (file != NULL) fprintf(file, "1, \t 0.2, 0\n");
		fflush(NULL);
		/* To OVERapproximate we use abstract_bound */
		//abs_post_star = ist_abstract_post_star(downward_closed_initial_marking,abstract_bound,abs->bound,system);
		abs_post_star = ist_abstract_post_star_until_reach_bad(downward_closed_initial_marking,abstract_bound,abs->bound,system,bad);
//		assert(ist_checkup(abs_post_star)==true);
		puts("end");
		*lfp = abs_post_star;
		inter = ist_intersection(abs_post_star,bad);
		finished=ist_is_empty(inter);
		ist_dispose(inter);
		if (finished==true) {
			/* finished==true -> the system is safe */
//			ist_write(*lfp);
//			ist_write(bad);
			retval = true;
		} else {
			printf("eec: EXPAND begin\t");
			if (file != NULL) fprintf(file, "1,\t 0.1, 0\n");
			fflush(NULL);
			/* use bpost = ist_abstract_post_star(downward_closed_initial_marking,bound_values,abs->bound,system)
			 * if you want to compute the lfp. Instead we make something more
			 * efficient by testing, at each iteration, for the emptiness of
			 * intersection w/ bad. */

			bpost = ist_copy(downward_closed_initial_marking);
			bound_values(bpost,abs->bound);
			ist_normalize(bpost);
			inter=ist_intersection(bpost,bad);
			finished= ist_is_empty(inter) == true ? false : true;
			ist_dispose(inter);

			ISTSharingTree * Frontier = ist_copy(bpost);

			while (finished==false) {
				if(file != NULL) fprintf(file, "%d,",iterations++);
				tmp = ist_abstract_post(Frontier,bound_values,abs->bound,system);
				if(file != NULL) fprintf(file, "\n");
				//tmp = ist_abstract_post_transtree(bpost,bound_values,abs->bound,system);
				ist_dispose(Frontier);
				Frontier =  ist_remove_subsumed_paths(tmp,bpost);
				ist_dispose(tmp);
				if (ist_is_empty(Frontier)==false) {
					inter=ist_intersection(Frontier,bad);
					finished=ist_is_empty(inter) == true ? false : true;
					ist_dispose(inter);
					tmp = ist_remove_subsumed_paths(bpost,Frontier);
					ist_dispose(bpost);
					bpost = ist_union(tmp,Frontier);
					ist_dispose(tmp);
				} else {
					break;
				}
			}
			assert(ist_checkup(bpost)==true);
			ist_dispose(bpost);
			puts("end");
			if (finished==true)
				/* finished==true -> we hitted the bad states, the system is unsafe */
				retval=false;
			else {
				/* One more iteration is needed because both bpost and
				 * abs_post_star does not allow to conclude */
				ist_dispose(abs_post_star);
				printf("eec: BOUNDS\t");
				//for(i=0;i<abs->nbV;printf("%d ",++abs->bound[i++]));
				/* nouvelle methode de calcul des bornes */
				new_bad = backward(system, initial_marking, bad, new_bound);
				ist_dispose(bad);
				bad = new_bad;
				xfree(new_bound);
				new_bound = compute_bound_from_ucs(bad);
				for(i = 0; i < system->limits.nbr_variables; i++)
					abs->bound[i] = new_bound[i];

				printf("\n");
				printf("New bounds");
				print_abstraction(abs);
			}
		}
	}
	ist_dispose(downward_closed_initial_marking);

	times(&after);
	tick_sec = sysconf (_SC_CLK_TCK) ;
	comp_u = ((float)after.tms_utime - (float)before.tms_utime)/(float)tick_sec ;
	comp_s = ((float)after.tms_stime - (float)before.tms_stime)/(float)tick_sec ;
	printf("TSI time of computation (user)   -> %6.3f sec.\n",comp_u);
	printf("                          (system) -> %6.3f sec.\n",comp_s);
	if(file != NULL) fprintf(file, "User: %6.3f sec\t System: %6.3f sec\n", comp_u, comp_s);

	return retval;
}


void eec(system, initial_marking, bad)
	transition_system_t *system;
	ISTSharingTree *bad, *initial_marking;
{
	boolean *maskpost, eec_conclusive;
	abstraction_t *bottomabs;
	ISTSharingTree *lfp_eec=NULL;
        int i,j;
	/* no abstraction is used to we build the bottom partition */
	bottomabs=(abstraction_t *)xmalloc(sizeof(abstraction_t));
	bottomabs->nbConcreteV=system->limits.nbr_variables;
	bottomabs->nbV=system->limits.nbr_variables;
	bottomabs->bound=(integer16 *)xmalloc(bottomabs->nbV*sizeof(integer16));
	bottomabs->A=(integer16 **)xmalloc(bottomabs->nbV*sizeof(integer16 *));
	for(i=0;i<bottomabs->nbV;++i) {
		bottomabs->A[i]=(integer16 *)xmalloc(system->limits.nbr_variables*sizeof(integer16));
		bottomabs->bound[i]=1;
		for(j=0;j<system->limits.nbr_variables;++j)
			if (i==j)
				bottomabs->A[i][j]=1;
			else
				bottomabs->A[i][j]=0;
	}

	printf("EEC for the concrete system\n");
	print_abstraction(bottomabs);

	maskpost=(boolean *)xmalloc(system->limits.nbr_rules*sizeof(boolean));
	for(i=0;i<system->limits.nbr_rules;++i)
		maskpost[i]=true;
	printf("transition system\n");
	from_transitions_to_tree(system, maskpost);
	ist_stat(system->tree_of_transitions);
	ist_write(system->tree_of_transitions);
	if (file != NULL) fprintf(file, "Iterations,Total elems,Time\n");
        eec_conclusive=eec_fp(system,bottomabs,initial_marking,bad,&lfp_eec);
	if (eec_conclusive == true){
		puts("EEC concludes safe");
		if(file != NULL) fprintf(file, "EEC: safe");
	}
	else {
		puts("EEC concludes unsafe");
		if(file != NULL) fprintf(file, "EEC: unsafe");
	}
}

void tsi(system, initial_marking, bad)
	transition_system_t *system;
	ISTSharingTree *bad, *initial_marking;
{
	boolean *maskpost, eec_conclusive;
	abstraction_t *bottomabs;
	ISTSharingTree *lfp_eec=NULL;
        int i,j;
	/* no abstraction is used, so we build the bottom partition */
	bottomabs=(abstraction_t *)xmalloc(sizeof(abstraction_t));
	bottomabs->nbConcreteV=system->limits.nbr_variables;
	bottomabs->nbV=system->limits.nbr_variables;
	bottomabs->bound=(integer16 *)xmalloc(bottomabs->nbV*sizeof(integer16));
	bottomabs->A=(integer16 **)xmalloc(bottomabs->nbV*sizeof(integer16 *));
	for(i=0;i<bottomabs->nbV;++i) {
		bottomabs->A[i]=(integer16 *)xmalloc(system->limits.nbr_variables*sizeof(integer16));
		bottomabs->bound[i]=1;
		for(j=0;j<system->limits.nbr_variables;++j)
			if (i==j)
				bottomabs->A[i][j]=1;
			else
				bottomabs->A[i][j]=0;
	}
	print_abstraction(bottomabs);
	maskpost=(boolean *)xmalloc(system->limits.nbr_rules*sizeof(boolean));
	for(i=0;i<system->limits.nbr_rules;++i)
		maskpost[i]=true;
	printf("transition system\n");
	from_transitions_to_tree(system, maskpost);
	ist_stat(system->tree_of_transitions);
	ist_write(system->tree_of_transitions);

	/* the TSI algorithm is built as a modification of the EEC algorithm */
	if (file != NULL) fprintf(file, "Iterations,Total elems,Time\n");
        eec_conclusive=eec_bound(system,bottomabs,initial_marking,bad,&lfp_eec);
	if (eec_conclusive == true){
		puts("TSI concludes safe");
		if(file != NULL) fprintf(file, "TSI: safe");
	}
	else{
		puts("TSI concludes unsafe");
		if(file != NULL) fprintf(file, "TSI: unsafe");
	}
}

static void* mist_cmdline_options_handle(int argc, char *argv[ ], int *timeout, char **filename, char **graph)
{
	int c;
	void *retval=NULL;

	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			{"help", 0, 0, 'h'},
			{"version", 0, 0, 0},
			{"backward", 0, 0, 'b'},
			{"ic4pn", 0, 0, 'i'},
			{"tsi", 0, 0, 't'},
			{"eec", 0, 0, 'e'},
			{"cegar", 0, 0, 'c'},
			{"timeout", 0, 0, 'o'},
			{"verbose", 0, 0, 'v'},
			{"graph", 0, 0, 'g'},
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

			case 'b':
				retval=&backward_basic;
				*filename = argv[optind++];
				break;

			case 'i':
				retval=&ic4pn;
				*filename = argv[optind++];
				break;

			case 't':
				retval=&tsi;
				*filename = argv[optind++];
				break;

			case 'e':
				retval=&eec;
				*filename = argv[optind++];
				break;

			case 'c':
				retval=&cegar;
				*filename = argv[optind++];
				break;

			case 'o':
				*timeout = atoi(argv[optind++]);
				break;

			case 'g':
				*graph = argv[optind++];
				break;

			case 'v':
				verbose = atoi(argv[optind++]);
				break;

			default:
				print_help();
				err_quit("?? getopt returned character code 0%o ??\n", c);
		}
	}

	if (retval == NULL) {
		print_help();
		err_quit("Wrong command invocation");
	}
	return retval;
}


int main(int argc, char *argv[ ])
{
	T_PTR_tree atree;
	transition_system_t *system;
	ISTSharingTree *initial_marking, *bad;
	void (*mc)(transition_system_t *sys, ISTSharingTree *init, ISTSharingTree *bad);
	int timeout=0;
	char *input_file=NULL, *graph=NULL;

	head_msg();
	mc=mist_cmdline_options_handle(argc, argv, &timeout, &input_file, &graph);
	assert(mc!=NULL);
	PRINTF("Timeout established to %d seconds\n", timeout);

	linenumber = 1;
	tbsymbol_init(&tbsymbol, 4096);

	printf("\n\n");
	printf("Parsing the problem instance.\n");
	printf("\n%s\n", input_file);
	my_yyparse(&atree, input_file);

	if (!is_petri_net(atree)){
		if (mc == eec || mc == ic4pn || mc == tsi || mc == cegar){
			err_quit("The algorithm you selected only accepts Petri Net and the input net is no a Petri net\n");
		}
	}

	/* We initialize the memory management of the system (must do it before parsing) */
	printf("Allocating memory for data structure.. ");
	ist_init_system();
	printf("DONE\n");

#ifdef TBSYMB_DUMP
	printf("\n\n");
	tbsymbol_dump(tbsymbol, &callback);
#endif

#ifdef TREE_DUMP
	printf("\n\n");
	tree_dump(atree, callback_tree_before, callback_tree_after, callback_leaf);
#endif


	build_problem_instance(atree, &system, &initial_marking, &bad);
	printf("System has %3d variables, %3d transitions and %2d actual invariants\n",system->limits.nbr_variables, system->limits.nbr_rules, system->limits.nbr_invariants);
	sprintf(description, "System: %3d variables, %3d transitions, %2d invariants\n",system->limits.nbr_variables, system->limits.nbr_rules, system->limits.nbr_invariants);

/* Our various coverability checker:
 * - backward is described in Laurent Van Begin Thesis and my master thesis (work for PN and extensions).
 * - ic4pn is described in ICATPN'07 paper and in fundamentae informatica'08.
 * - cegar development has been stalled for a couple of years, it is a counterexample based abstraction refinement coverability checker.
 * - tsi is described in our TSI journal paper.
 * - eec implements the expand, enlarge and check algorithm described briefly in our ICATPN'07 and fundamentae informatica'08 papers
 *   and described in details in Gilles Geerearts' thesis.
 */
	//backward_basic(system,initial_marking,bad);
	//ic4pn(system,initial_marking,bad);
	//cegar(system,initial_marking,bad);
	//eec(system,initial_marking,bad);
	//tsi(system,initial_marking,bad);

	struct itimerval timer;
	struct sigaction sa;

	/* Configure the timer to expire after timeout sec... */
	timer.it_value.tv_sec = timeout;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

	/* Install timer_handler as the signal handler for SIGVTALRM. */
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &timeout_func;
	sigaction (SIGVTALRM, &sa, NULL);

	/* Open the output file*/
	file = NULL;
	if (graph != NULL) file = fopen(graph, "w");
	if (file == NULL){
		printf("Can't open file %s so there won't be data for the graphics\n", graph);
	}

	/* Start a virtual timer. It counts down whenever this process is
	   executing. */
	setitimer(ITIMER_VIRTUAL, &timer, NULL);
	if(mc)
		mc(system,initial_marking,bad);
	setitimer(ITIMER_VIRTUAL, NULL, NULL);

	if(file != NULL) fprintf(file, "\n%s",description);

	ist_dispose(initial_marking);
	ist_dispose(bad);
	dispose_transition_system(system);
	if(file != NULL) fclose(file);

	tbsymbol_destroy(&tbsymbol);

	puts("Thanks for using this tool");
	return 0;
}
