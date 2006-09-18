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
	ISTSharingTree * S;
	ISTSharingTree * post;
	ISTSharingTree * Siter;
	ISTSharingTree * intersect;
	int Continue, i;

	printf("Path to reach the set of target states:\n");

	S=ist_copy(initial_marking);
	Siter = ist_first_element_list_ist(list_ist);
	while (Siter != NULL) {
		i = 0;
		Continue = 1;
		while ((i < rules->limits.nbr_rules) && (Continue == 1)) {
			post = ist_enumerative_post_transition(S,rules,i);
			intersect = ist_intersection(post,Siter);
			if (!ist_is_empty(intersect)) { 
				printf(" --> %d",i);
				ist_dispose(S);
				S = intersect;
				Continue = 0;
			} else { 
				ist_dispose(intersect);
				ist_dispose(post);
				i = i + 1;
			} 
		} 
		if (i == rules->limits.nbr_rules) { 
			printf("\nNo Path Found\n");
			Siter = NULL;
		} else {
			Siter = ist_next_element_list_ist(list_ist);
		}
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
		frontier = ist_pruned_pre(old_frontier, reached_elem, system);

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
				ist_insert_at_the_beginning_list_ist(&List,frontier);
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

static ISTNode *Downward_closure(node, LINK)
	ISTNode *node;
	struct LOC_ist_method  *LINK;
{
	ISTSon *s;
	ISTNode *rnode;
	ISTInterval i;
	
	if (ist_not_equal_interval(node->Info,&IST_end_of_list)) {
		i.Left = 0;
		i.Right = node->Info->Right;
		rnode = ist_create_node(&i);
		node->AuxI = ist_get_magic_number();
		LINK->rlayer = LINK->rlayer->Next;
		if (LINK->rlayer == NULL)
			LINK->rlayer = ist_add_last_layer(LINK->STR);
		s = node->FirstSon;
		while (s != NULL) {
			if (s->Son->AuxI == ist_get_magic_number())
				ist_add_son(rnode, s->Son->AuxP);
			else
				ist_add_son(rnode, Downward_closure(s->Son, LINK));
			s = s->Next;
		}
		LINK->rlayer = LINK->rlayer->Previous;
	} else {
		rnode = ist_create_node(node->Info);
		node->AuxI = ist_get_magic_number();
	}
	node->AuxP = ist_add_node(LINK->rlayer, rnode);
	return (node->AuxP);
}


/* This function returns the copy of the IST provided in parameter */
ISTSharingTree *ist_downward_closure(ST)
	ISTSharingTree *ST;
{
	struct LOC_ist_method  V;
	ISTSon *s;
	ISTNode *rchild;

	ist_new_magic_number();
	ist_new(&V.STR);
	if (!ist_is_empty(ST)) {
		V.rlayer = ist_add_last_layer(V.STR);
		s = ST->Root->FirstSon;
		while (s != NULL) {
			rchild = Downward_closure(s->Son, &V);
			ist_add_son(V.STR->Root, rchild);
			s = s->Next;
		}
	}
	V.STR->NbElements = ST->NbElements;
	return V.STR;
}

void bound_values(ISTSharingTree * S, int *bound)
{
	ISTNode *N;
	ISTLayer *L;
	size_t i;
	
	for(i=0,L = S->FirstLayer;L != S->LastLayer;i++,L = L->Next)
		for(N= L->FirstNode;N != NULL;N = N->Next)
			ist_assign_values_to_interval(N->Info,min(N->Info->Left,bound[i]),min(N->Info->Right,bound[i]));
}

ISTSharingTree *bounded_post_rule(ISTSharingTree * S, abstraction_t * abs, transition_system_t *t, int rule,int*bound) 
{
   ISTSharingTree *result;
   ISTSharingTree * tmp = ist_symbolic_post_of_rules(S,abs,t,rule);
   if (ist_is_empty(tmp) == false) {
      ist_checkup(tmp);
      bound_values(tmp,bound);
      result = ist_downward_closure(tmp);
	  ist_dispose(tmp);
      ist_normalize(result);
   } else result = tmp;
   return result;
}

ISTSharingTree *bounded_post(ISTSharingTree *S, abstraction_t *abs, transition_system_t *t, int*bound) {
	ISTSharingTree *result;
	ISTSharingTree *tmp;
	ISTSharingTree *tmp2;
	size_t i;
	
	ist_new(&result);
	for(i = 0;i<t->limits.nbr_rules;i++) {
		tmp = bounded_post_rule(S,abs,t,i,bound);
		if (ist_is_empty(tmp) == false) {
			tmp2 = ist_union(tmp,result);
			ist_dispose(tmp);
			ist_dispose(result);
			result = tmp2;
		} else 
			ist_dispose(tmp);
	}
	return result;
}

ISTSharingTree *bounded_post_star(ISTSharingTree * initial_marking, abstraction_t * abs, transition_system_t *t,
		int*bound) {
	ISTSharingTree *result;
	ISTSharingTree *tmp;
	ISTSharingTree *tmp2;


	result = ist_copy(initial_marking);
	bound_values(result, abs->bound);
	ist_normalize(result);
	while (true) {
		tmp = bounded_post(result,abs,t,bound);
		if (ist_exact_subsumption_test(tmp,result) == false) {
			tmp2 = ist_union(tmp,result);
			ist_dispose(tmp);
			ist_dispose(result);
			result = tmp2;
		} else {
			ist_dispose(tmp);
			break;
		}
	}
	return result;
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
	ist_checkup(downward_closed_initial_marking);
	
	while (finished == false) {
		puts("eec: enlarge");
		abs_post_star = ist_abstract_post_star(downward_closed_initial_marking,abs,system);
		inter = ist_intersection(abs_post_star,bad);
		if (ist_is_empty(inter) == true) {
			ist_dispose(inter);
			*lfp = abs_post_star;
			retval = true;
			finished = true;
		} else {
			puts("eec: expand");
			ist_dispose(inter);
		
			assert(ist_checkup(downward_closed_initial_marking)==true);

			bpost = bounded_post_star(downward_closed_initial_marking,abs,system,abs->bound);	
			inter = ist_intersection(bpost,bad);
			if (ist_is_empty(inter) == false) {
				*lfp = abs_post_star;
				retval = false;
				finished = true;
			} else {
				ist_dispose(abs_post_star);
				for (i = 0;i< system->limits.nbr_variables;i++)
					++abs->bound[i];
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
	size_t i,j,nb_iteration, nb_iter_gfp;
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
	safe=ist_copy(bad);
	ist_complement(safe,system->limits.nbr_variables);

	nb_iteration=0;
	while(conclusive == false) {
		/* We build the abstract system */
		sysabs=build_sys_using_abs(system,myabs);
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
			print_abstraction(myabs);
			puts("The EEC fixpoint");
			assert(ist_checkup(lfp_eec)==true);


			/* safe is given by \alpha(\neg bad) /\ lfp_eec */
			tmp = ist_abstraction(safe,myabs);
			_tmp = ist_intersection(lfp_eec,tmp);
			ist_dispose(tmp);
			ist_dispose(lfp_eec);
			alpha_safe=_tmp;

			/* def of the first iterates of the gfp in the abstract */
			iterates = ist_copy(alpha_safe);
			assert(ist_checkup(iterates)==true);
			tmp = ist_downward_closure(iterates);
			ist_dispose(iterates);
			iterates = tmp;
			ist_normalize(iterates);
			assert(ist_checkup(iterates)==true);

			/* compute the gfp for the abstraction */
			nb_iter_gfp=0;
			do {
				++nb_iter_gfp;
				/* We simplify the iterates */
				tmp=ist_downward_closure(iterates);
				ist_dispose(iterates);
				ist_normalize(tmp);
				iterates=ist_minimal_form(tmp);
				ist_dispose(tmp);

				assert(ist_checkup(iterates)==true);
				assert(ist_nb_layers(iterates)-1==myabs->nbV);
				tmp = adhoc_pretild(iterates,sysabs);

				new_iterates = ist_intersection(tmp,alpha_safe);
				assert(ist_checkup(new_iterates)==true);
				ist_dispose(tmp);
				tmp = ist_downward_closure(new_iterates);
				ist_dispose(new_iterates);
				new_iterates = tmp;
				ist_normalize(new_iterates);
				assert(ist_checkup(new_iterates)==true);

				/* We remove the subsumed paths in iterates */
				tmp = ist_remove_subsumed_paths(iterates,new_iterates);
				out = ist_is_empty(tmp);
				ist_dispose(tmp);
				ist_dispose(iterates);
				iterates = new_iterates;
			} while(out == false);
			printf("nb_iter_gfp = %d\n",nb_iter_gfp);

			/* we compute gamma(gfp) */
			assert(ist_checkup(iterates)==true);
			
			gamma_gfp = ist_concretisation(iterates,myabs);
			ist_dispose(iterates);

			assert(ist_checkup(gamma_gfp)==true);

			ist_complement(gamma_gfp,system->limits.nbr_variables);

			/* conclusive = true implies initial_marking \nsubseteq gamma_gfp */
			tmp = ist_intersection(gamma_gfp,initial_marking);	
			conclusive = (ist_is_empty(tmp)==true ? false : true);
			ist_dispose(tmp);

			/* We compute a concrete iterates */
			tmp=ist_pre(gamma_gfp,system);
			ist_complement(tmp,system->limits.nbr_variables);
			/* Now intersects w/ safe */
			_tmp=ist_intersection(safe,tmp);
			ist_dispose(tmp);

			/* We simplify the set */
			tmp=ist_downward_closure(_tmp);
			ist_dispose(_tmp);
			ist_normalize(tmp);
			_tmp=ist_minimal_form(tmp);
			ist_dispose(tmp);

			ist_dispose(gamma_gfp);

			/* We build the next abstraction */
			newabs=refine_abs(myabs,_tmp);
			assert(newabs->nbV > myabs->nbV);
			assert(newabs->nbConcreteV == myabs->nbConcreteV);
			release_abstraction(myabs);
			myabs=newabs;
		}
		/* We release the abstract system */
		release_transition_system(sysabs);
		printf("End of iteration %d\n",++nb_iteration);
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

	ic4pn(system,initial_marking,unsafe_cone);
	//mist(system,initial_marking,unsafe_cone);

	ist_dispose(initial_marking);
	ist_dispose(unsafe_cone);

	tbsymbol_destroy(&tbsymbol);

	puts("Thanks for using this tool");

	return 0;
}
