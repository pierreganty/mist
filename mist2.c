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

   Copyright 2003, 2004, 2006, Pierre Ganty, Laurent Van Begin
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
#include <assert.h>

#include "laparser.h"
#include "ist.h"


/* For printing the error trace */
void ist_put_path_on_screen_with_parameterized_initial_marking(ISTSharingTree * initial_marking,THeadListIST * list_ist, transition_system_t * rules) 
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
			if (!ist_is_empty(intersect)) { 
				printf("[%d> ",i);
				ist_dispose(S);
				S = intersect;
				Continue = false;
			} else { 
				ist_dispose(intersect);
				ist_dispose(post);
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
}

boolean backward_lfp(system, initial_marking, frontier) 
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

	times(&before) ;

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
		puts("Unsafe region is empty or lies outside the invariants or contrains some initial states");
	}
	ist_init_list_ist(&List);
	nbr_iteration = 1;
	while (Continue == true) {
		printf("\n\nIteration\t%3d\n", nbr_iteration);
		puts("Computation of the symbolic predecessors states ...");
		old_frontier = frontier;
		/* As post cond, we have that frontier is minimal (in a 1to1 comparison) w.r.t reached_elem */
		frontier = ist_pre_pruned_wth_inv_and_prev_iterates(old_frontier, reached_elem, system);

		if (ist_is_empty(frontier) == false) {

			printf("The new frontier counts :\n");
			ist_checkup(frontier);
#ifdef VERBOSE
			ist_write(frontier);
#endif
			temp=ist_intersection(initial_marking,frontier);
			reached = (ist_is_empty(temp) == true ? false : true);
			ist_dispose(temp);
			if (reached == true) {
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
#ifdef INTERVAL
				printf("\n\tStarting minimization of reached states\n\t");
				ist_stat(reached_elem);
				ist_minimal_form_sim_based(reached_elem);
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

		} else 
			Continue = false;
		nbr_iteration++;
	}

	if (nbr_iteration != 0){
		puts("The reached symbolic state space is:");
		ist_stat(reached_elem);
#ifdef VERBOSE
		ist_write(reached_elem);
#endif
	}
	if (reached == true)
		ist_put_path_on_screen_with_parameterized_initial_marking(initial_marking,&List,system);
	times(&after);
	tick_sec = sysconf (_SC_CLK_TCK) ;
	comp_u = ((float)after.tms_utime - (float)before.tms_utime)/(float)tick_sec ;
	comp_s = ((float)after.tms_stime - (float)before.tms_stime)/(float)tick_sec ;
	printf("Total time of computation (user)   -> %6.3f sec.\n",comp_u);
	printf("                          (system) -> %6.3f sec.\n",comp_s);

	ist_dispose(reached_elem);
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
	
	/* Is the system safe ? */
	return (reached== true ? false : true);
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

/* lfp is a out parameter which contains de lfp */
boolean eec(system, abs, initial_marking, bad, lfp)
	transition_system_t *system;
	abstraction_t *abs; /* For the bounds on the places */
	ISTSharingTree *initial_marking, *bad, **lfp;
{
	boolean retval;
	ISTSharingTree * abs_post_star;
	ISTSharingTree * inter;
	ISTSharingTree * downward_closed_initial_marking;
	boolean finished = false;
	ISTSharingTree * bpost;
	size_t i;

	downward_closed_initial_marking = ist_downward_closure(initial_marking);
	ist_normalize(downward_closed_initial_marking);
	assert(ist_checkup(downward_closed_initial_marking)==true);
	
	while (finished == false) {
		printf("eec: ENLARGE begin\t");
		fflush(NULL);
		abs_post_star = ist_abstract_post_star(downward_closed_initial_marking,abs,system);
		puts("end");
		inter = ist_intersection(abs_post_star,bad);
		if (ist_is_empty(inter) == true) {
			ist_dispose(inter);
			*lfp = abs_post_star;
			retval = true;
			finished = true;
		} else {
			ist_dispose(inter);
		
			printf("eec: EXPAND begin\t");
			fflush(NULL);
			bpost = bounded_post_star(downward_closed_initial_marking,abs,system);	
			puts("end");
			inter = ist_intersection(bpost,bad);
			if (ist_is_empty(inter) == false) {
				*lfp = abs_post_star;
				retval = false;
				finished = true;
			} else {
				ist_dispose(abs_post_star);
				for (i=0;i< system->limits.nbr_variables;i++)
					++abs->bound[i];
				printf("eec: BOUNDS\t");
				for(i=0;i<abs->nbV;printf("%d ",abs->bound[i++]));
				printf("\n");
			}
			ist_dispose(bpost);
			ist_dispose(inter);
		}
	}
	ist_dispose(downward_closed_initial_marking);
	return retval;
}

void ic4pn(system, initial_marking, bad) 
	transition_system_t *system;
	ISTSharingTree *bad, *initial_marking;
{
	abstraction_t *myabs, *newabs;
	transition_system_t *sysabs;
	ISTSharingTree *lfp_eec, *gamma_gfp, *tmp, *_tmp, *safe, *alpha_safe,
				   *iterates, *new_iterates, *alpha_initial_marking,
				   *alpha_bad;
	size_t i,j,nb_iteration, iterator;
	boolean out, conclusive, eec_conclusive, bw_conclusive;

	/* Memory allocation */
	myabs=(abstraction_t *)xmalloc(sizeof(abstraction_t));
	/* We copy the number of places of the original system into the abstraction */
	myabs->nbConcreteV=system->limits.nbr_variables;
	/* We start with a unique abstract place */
	myabs->nbV=1;
	myabs->bound=(integer16 *)xmalloc(myabs->nbV*sizeof(integer16));
	myabs->A=(integer16 **)xmalloc(myabs->nbV*sizeof(integer16));
	for(i=0;i<myabs->nbV;++i)
		myabs->A[i]=(integer16 *)xmalloc(system->limits.nbr_variables*sizeof(integer16));
	/* Initial abstraction */
	for(i=0;i<myabs->nbV;++i) {
		myabs->bound[i]=1;
		for(j=0;j<system->limits.nbr_variables;++j)
			myabs->A[i][j]=1;
	}
	tmp=ist_intersection(initial_marking,bad);
	conclusive = (ist_is_empty(tmp)==true ? false : true);
	ist_dispose(tmp);

	/* safe = \neg bad */
	tmp=ist_copy(bad);
	ist_complement(tmp,system->limits.nbr_variables);
	/* tmp represents a dc-set */
	safe=ist_downward_closure(tmp);
	ist_normalize(safe);
	/* safe = tmp and each path is a dc-closed */

	nb_iteration=0;
	while(conclusive == false) {
		puts("begin of iteration");
		/* We build the abstract system */
		sysabs=build_sys_using_abs(system,myabs);
		puts("The current abstraction is :");
		print_abstraction(myabs);
		puts("The current abstracted net is:");
		print_transition_system(sysabs);
		/* We abstract bad and initial_marking for eec */	
		alpha_bad = ist_abstraction(bad,myabs);
		assert(ist_checkup(alpha_bad)==true);
		alpha_initial_marking = ist_abstraction(initial_marking,myabs);
		assert(ist_checkup(alpha_initial_marking)==true);

		bw_conclusive=backward_lfp(sysabs,alpha_initial_marking,alpha_bad);
		eec_conclusive=eec(sysabs,myabs,alpha_initial_marking,alpha_bad,&lfp_eec);
		/* To be sure that eec is correct */
		assert(bw_conclusive==eec_conclusive);

		ist_dispose(alpha_initial_marking);
		ist_dispose(alpha_bad);

		if (eec_conclusive==true) {
			/* says "safe" because it is indeed safe */
			puts("EEC concludes safe with the abstraction");
			print_abstraction(myabs);
			conclusive = true;
		} else { /* refine the abstraction */
			puts("The EEC fixpoint");
			assert(ist_checkup(lfp_eec)==true);
			/* safe is given by safe /\ gamma(lfp_eec) */
			_tmp=ist_concretisation(lfp_eec,myabs);
			tmp=ist_intersection(safe,_tmp);
			ist_dispose(_tmp);
			ist_dispose(safe);
			safe=tmp;
			tmp = ist_downward_closure(safe);
			ist_normalize(tmp);
			ist_dispose(safe);
			safe=ist_minimal_form(tmp);

			/* alpha_safe is given by \alpha(safe). Note that alpha <= lfp_eec */
			tmp = ist_abstraction(safe,myabs);
			_tmp = ist_downward_closure(tmp);
			ist_normalize(_tmp);
			ist_dispose(tmp);
			tmp=ist_minimal_form(_tmp);
			ist_dispose(_tmp);
			alpha_safe=tmp;
			
			ist_dispose(lfp_eec);

			/* iterates = alpha_safe */
			iterates = ist_copy(alpha_safe);
			assert(ist_checkup(iterates)==true);
			assert(ist_equal(iterates,alpha_safe));
			assert(ist_checkup(iterates)==true);

			/* compute the gfp for the abstraction */
			iterator=0;
			do {
				++iterator;

				assert(ist_checkup(iterates)==true);
				assert(ist_nb_layers(iterates)-1==myabs->nbV);
				tmp = adhoc_pretild(iterates,sysabs);

				new_iterates = ist_intersection(tmp,alpha_safe);
				assert(ist_checkup(new_iterates)==true);
				ist_dispose(tmp);
				tmp = ist_downward_closure(new_iterates);
				ist_normalize(tmp);
				ist_dispose(new_iterates);
				new_iterates = ist_minimal_form(tmp);
				ist_dispose(tmp);
				assert(ist_checkup(new_iterates)==true);

				/* We remove the subsumed paths in iterates */
				tmp = ist_remove_subsumed_paths(iterates,new_iterates);
				out = ist_is_empty(tmp);
				ist_dispose(tmp);
				ist_dispose(iterates);
				iterates = new_iterates;
			} while(out == false);
			printf("number of iterations for the gfp = %d\n",iterator);

			/* we compute gamma(gfp) */
			assert(ist_checkup(iterates)==true);
			gamma_gfp = ist_concretisation(iterates,myabs);
			ist_dispose(iterates);
			iterates = ist_copy(gamma_gfp);

			assert(ist_checkup(iterates)==true);

			puts("gamma_gfp");
			//ist_write(gamma_gfp);
			puts("¬ (gamma_gfp)");
			ist_complement(gamma_gfp,system->limits.nbr_variables);
			//ist_write(gamma_gfp);

			/* conclusive = true implies initial_marking \nsubseteq gamma_gfp */
			tmp = ist_intersection(gamma_gfp,initial_marking);	
			conclusive = (ist_is_empty(tmp)==true ? false : true);
			ist_dispose(tmp);

			/* We compute a concrete iterates */
			puts("pre o ¬ (gamma_gfp)");
			tmp=ist_pre(gamma_gfp,system);
			ist_dispose(gamma_gfp);
			//ist_write(tmp);
			puts("min o v o ¬ o pre o ¬ (gamma_gfp)");
			ist_complement(tmp,system->limits.nbr_variables);
			_tmp=ist_downward_closure(tmp);
			ist_dispose(tmp);
			tmp=ist_minimal_form(_tmp);
			//ist_write(tmp);

			/* Now intersects w/ safe */
			_tmp=ist_intersection(safe,tmp);
			ist_dispose(tmp);
			tmp=ist_downward_closure(_tmp);
			ist_dispose(_tmp);
			ist_normalize(tmp);
			puts("min(cpre(gamma_gfp) & safe )");
			new_iterates=ist_minimal_form(tmp);
			//ist_write(new_iterates);
			ist_dispose(tmp);

			/* We build the next abstraction */
			newabs=refine_abs(myabs,iterates,new_iterates);
			ist_dispose(new_iterates);
			iterator=0; 
			/* We go through this loop to refine transition per transition */
			while(iterator < system->limits.nbr_rules && newabs->nbV == myabs->nbV) {
				dispose_abstraction(newabs);
				tmp=adhoc_place_pretild_rule(iterates,&system->transition[iterator]);
				_tmp=ist_downward_closure(tmp);
				ist_normalize(_tmp);
				ist_dispose(tmp);	
				tmp=ist_minimal_form(_tmp);
				ist_dispose(_tmp);
				newabs = refine_abs(myabs,iterates,tmp);
				ist_dispose(tmp);
				++iterator;
			}
			/* we dispose iterates and myabs */
			ist_dispose(iterates);
			assert(newabs->nbConcreteV == myabs->nbConcreteV);
			assert(newabs->nbV > myabs->nbV);
			dispose_abstraction(myabs);
			 
			myabs=newabs;
			assert(newabs->nbV >0);
		}
		/* We release the abstract system */
		dispose_transition_system(sysabs);
		printf("end of iteration %d\n",++nb_iteration);
	}
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

	/* We initialize the memory management of the system */
	printf("Allocating memory for data structure.. ");
	ist_init_system();
	printf("DONE\n");

	//backward_lfp(system,initial_marking,unsafe_cone);
	ic4pn(system,initial_marking,unsafe_cone);

	ist_dispose(initial_marking);
	ist_dispose(unsafe_cone);
	dispose_transition_system(system);

	tbsymbol_destroy(&tbsymbol);

	puts("Thanks for using this tool");

	return 0;
}
