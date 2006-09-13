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

   Copyright 2006, Pierre Ganty
 */

#include"abstraction.h"
#include"concretisation.h"
#include"xmalloc.h"

/* Only works for Petri Nets w/o invariants */
transition_system_t *
build_sys_using_abs(sys,abs)
	transition_system_t *sys;
	abstraction_t *abs;
{
	transition_system_t *retval;
	size_t i,j,k;
	
	retval=(transition_system_t *)xmalloc(sizeof(transition_system_t));
	retval->limits.nbr_rules = sys->limits.nbr_rules;
	retval->limits.nbr_variables = abs->nbV;
	/* Assignment for sanity of data structure */
	retval->limits.nbr_invariants=0;
	retval->transition=(transition_t *)xmalloc(retval->limits.nbr_rules*sizeof(transition_t));
	for(i=0;i<retval->limits.nbr_rules;++i){
		retval->transition[i].cmd_for_place = (gd_command_t *)xmalloc(abs->nbV*sizeof(gd_command_t));
		/* Assignment for sanity of data structure */
		retval->transition[i].nbr_transfers=0;
		for(j=0;j<retval->limits.nbr_variables;++j){
			/* We maintain for each place in the abstract net how many concrete ones it represent */
			retval->transition[i].cmd_for_place[j].places_abstracted=0;
			/* Start from bottom values for delta and guard */
			retval->transition[i].cmd_for_place[j].delta =0L;
			ist_assign_values_to_interval(&retval->transition[i].cmd_for_place[j].guard, 0L,  INFINITY);
			for(k=0;k<sys->limits.nbr_variables;++k){
				if(abs->A[j][k]>0){
					retval->transition[i].cmd_for_place[j].delta+= sys->transition[i].cmd_for_place[k].delta;
					ist_add_value_to_interval(&retval->transition[i].cmd_for_place[j].guard, sys->transition[i].cmd_for_place[k].guard.Left);
					++retval->transition[i].cmd_for_place[j].places_abstracted;
				}
			}
		}
	}
	return retval;	
}

void print_abstraction(abs)
	abstraction_t *abs;
{
	size_t i,j;
	printf("The system is %d variables.\n",(int) abs->nbV);
	puts("The abstraction:");
	puts("	For the merging:");
	for(i=0;i<abs->nbV;++i){
		for(j=0;j<abs->nbConcreteV;++j)
			printf("%d",(int) abs->A[i][j]);
		puts("");
	}
	puts("	For the bounds:");
	for(i=0;i<abs->nbV;++i)
		printf("%d",(int) abs->bound[i]);
	puts("");

}

boolean is_layer_unbounded(layer)
	ISTLayer *layer;
{
	ISTNode *Node=layer->FirstNode;
	while(Node!=NULL){
		if(ist_is_unbounded(Node->Info)==true) {
			return true;
		}
		Node=Node->Next;
	}
	return false;
}

ISTSharingTree *ist_abstraction(S,abs)
	ISTSharingTree *S;
	abstraction_t *abs;
{
	size_t i, j;
	ISTSharingTree *temp, *temp2, *result;
	ISTLayer * L;
	ISTNode * N;
	transition_t * t = (transition_t *)xmalloc(sizeof(transition_t));
	boolean * in_abs = (boolean *)xmalloc(abs->nbConcreteV * sizeof(boolean));
	int *mask;

	if (ist_is_empty(S) == false) {
		/* initialisation */
		for(i = 0; i < abs->nbConcreteV;i++)
			in_abs[i] = false;
	
		/* adding of abstract variables */
		temp = ist_copy(S);
		ist_add_variables(temp,abs->nbV);
		/*
		 * construction of the transfers that defines the mapping from concrete
		 * variables to abstract variables
		 */
		t->nbr_transfers = abs->nbV;	
		for(i=0;i< abs->nbV;i++) {
			t->transfers[i].target = abs->nbConcreteV+i;
			t->transfers[i].origin = (integer16 *) xmalloc
				((abs->nbConcreteV + abs->nbV) * sizeof(integer16)); 
			for(j=0;j < abs->nbConcreteV;j++) 
				if (abs->A[i][j] != 0) {
					t->transfers[i].origin[j] = abs->A[i][j];
					in_abs[j] = true;
				}
			for(j=0; j < abs->nbV;j++) {
				t->transfers[i].origin[abs->nbConcreteV + j] = 0;
			}
		}
		/* Computation of the abstract values */
		temp2 = ist_post_of_transfer(temp,t);
		ist_dispose(temp);
		for(i=0; i < abs->nbV;i++) 
			xfree(t->transfers[i].origin);
		xfree(t);	
		/* projection to only keep the concrete variables */
		mask = (integer16 *) xmalloc((abs->nbConcreteV + \
					abs->nbV+1) * sizeof(integer16));
		for(i = 0; i < abs->nbConcreteV;i++) 
			mask[i] = 0;
		/* i = abs->nbConcreteV */
		for(; i < abs->nbV + abs->nbConcreteV; i++) 
			mask[i] = 1;
		/* by convention */
		mask[abs->nbV + abs->nbConcreteV] = 1;
		result = ist_projection(temp2,mask);
		ist_dispose(temp2);
		xfree(mask);
		/*
		 * assignment of variables not in abstraction (viz. each entry of the column equals to 0)
		 */
		for(i = 0, L = result->FirstLayer; i < abs->nbV; i++,L=L->Next) {
			if (in_abs[i] == false) {
				N = L->FirstNode;
				while (N != NULL) {
					ist_assign_values_to_interval(N->Info,0,INFINITY);
					N = N->Next;
				}
			}		
		}
	} else ist_new(&result);
	return result;
}

/* We assume exists i s.t. sum cur_abs->A[i][j] > 1 */
abstraction_t *refine_abs(cur_abs, S)
	abstraction_t *cur_abs;
	ISTSharingTree *S;
{
	size_t i,j,current,first_non_singleton_row;
	boolean before_non_null_entry,first_bound,nl_added;
	abstraction_t *retval;
	ISTLayer *layer;
	retval=(abstraction_t *)xmalloc(sizeof(abstraction_t));
	/* The number of concrete places never changes */
	retval->nbConcreteV=cur_abs->nbConcreteV;
	/* A refinement step add exactly one new dimension w.r.t. the current abstraction */
	retval->nbV=(cur_abs->nbV+1);
	retval->A=(integer16 **)xmalloc(retval->nbV*sizeof(integer16));
	retval->bound=(integer16 *)xmalloc(retval->nbV*sizeof(integer16));
	for(i=0;i<retval->nbV;++i){
		retval->A[i]=(integer16 *)xmalloc(cur_abs->nbConcreteV*sizeof(integer16));
		/* The new abstraction is initialized w/ 0's everywhere */
		for(j=0;j<cur_abs->nbConcreteV;++j)
			retval->A[i][j]=0;
		retval->bound[i]=1;
	}
	/* current : pointer in retval, i : pointer in cur_abs */
	for(current=0,i=0;i<cur_abs->nbV && current<retval->nbV;++i,++current){
		/* Did we add a new line ? */
		nl_added=false;
		/* Is i before the first non null entry ? */
		before_non_null_entry=true;
		/* We refine according to the value in S */
		layer = S->FirstLayer;
		for(j=0;j<cur_abs->nbConcreteV;++j){
			if(cur_abs->A[i][j] == 1) {
				/* By default we copy the line */
				retval->A[current][j]=1;
				/* if haven't added a new line yet */
				if(current == i){
					if (before_non_null_entry==false){
						if(is_layer_unbounded(layer) != first_bound ){
							/* We add a new line. We split unbounded from bounded places in cur_abs->A[i] */
							nl_added=true;
							retval->A[current][j]=0;
							retval->A[current+1][j]=1;
						}
						first_non_singleton_row=i;
					} else {
						/* For the 1st non null entry. We remember if it is unbounded or no */
						first_bound = is_layer_unbounded(layer);
						before_non_null_entry=false;
					}
				}
			}
			layer=layer->Next;
		}
		if(nl_added == true)
			++current;

	}
	/* If cur_abs->A == retval->A */
	if (current==i){
		for(j=0;cur_abs->A[first_non_singleton_row][j]==0;++j);
		retval->A[first_non_singleton_row][j]=0;
		retval->A[current][j]=1;
	}
	return retval;
}


ISTSharingTree *ist_symbolic_post_of_rules(ISTSharingTree * S, abstraction_t * abs, transition_system_t *t, int rule) 
{
	ISTSharingTree * result;
	ISTLayer * L;
	ISTNode * N;
//	ISTInterval * v;
	int i;
	ISTInterval **g = (ISTInterval **)xmalloc(abs->nbV * sizeof(ISTInterval *));
	ISTSharingTree *G;

	for(i = 0;i < abs->nbV;i++) {
		g[i] = ist_copy_interval(&t->transition[rule].cmd_for_place[i].guard);
	}
	ist_new(&G);
	ist_add(G,g,abs->nbV);
	for(i= 0;i< abs->nbV;i++) {
		ist_dispose_info(g[i]);
	}
	xfree(g);

	result = ist_intersection(S,G);
	ist_dispose(G);
	
/*	printf("entree symbolic_post_of_rule\n");

	ist_write(S);
	
	result =  ist_copy(S);	
	//intersection with guard
	//If the info of a node has no intersection with guard, we remove its sons, i.e. the node 
	//becomes useless and will be removed later.	
	for (i = 0, L = result->FirstLayer; i < abs->nbV; i++,L = L->Next) {
		for(N = L->FirstNode;N != NULL;N = N->Next) {
			v = ist_intersect_intervals(N->Info,&t->transition[rule].cmd_for_place[i].guard);
			if (v == NULL) {
				ist_remove_sons(N);
			} else {
				ist_dispose_info(N->Info);
				N->Info = v;
			}
		}
	}

	//we remove the useless nodes
	ist_remove_node_without_son(result);
*/
	//If the IST is not empty, we apply the effect of the function
	if (ist_is_empty(result) == false) {

		printf(" result n est pas vide:\n");
		ist_checkup(result);
		
		for (i = 0, L = result->FirstLayer; i < abs->nbV; i++, L = L->Next) {
			for(N = L->FirstNode;N != NULL;N=N->Next) {
					ist_assign_values_to_interval(N->Info, 
							N->Info->Left + t->transition[rule].cmd_for_place[i].delta,
							N->Info->Right + t->transition[rule].cmd_for_place[i].delta);
			}
		}	
	} else printf("IST vide\n");
	return result;
}

ISTSharingTree *ist_symbolic_post(ISTSharingTree * S, abstraction_t * abs, transition_system_t *t) {

	int i;
	ISTSharingTree * result;
	ISTSharingTree * tmp;
	ISTSharingTree * tmp2;

	printf("entree abstract_post\n");
	
	ist_new(&result);
	for(i=0;i< t->limits.nbr_rules;i++) {
		tmp = ist_symbolic_post_of_rules(S,abs,t,i);
		if ( ist_exact_subsumption_test(tmp,result) == false) {
			tmp2 = ist_union(tmp,result);
			ist_dispose(tmp);
			ist_dispose(result);
			result = tmp2;
		} else {
			ist_dispose(tmp);
		}
	}
	return result;
}


ISTSharingTree *ist_abstract_post_of_rules(ISTSharingTree * S, abstraction_t * abs, transition_system_t *t, int rule) 
{
	ISTSharingTree * result;
	ISTLayer * L;
	ISTNode * N;
	ISTInterval * v;
	int i;

	printf("entree abstract_post_of_rule\n");

	ist_write(S);
	
	result =  ist_copy(S);	
	//intersection with guard
	//If the info of a node has no intersection with guard, we remove its sons, i.e. the node 
	//becomes useless and will be removed later.	
	for (i = 0, L = result->FirstLayer; i < abs->nbV; i++,L = L->Next) {
		for(N = L->FirstNode;N != NULL;N = N->Next) {
			v = ist_intersect_intervals(N->Info,&t->transition[rule].cmd_for_place[i].guard);
			if (v == NULL) {

				printf("1)\n");
				
				ist_remove_sons(N);
			} else {

				printf("2)\n");
				
				ist_dispose_info(N->Info);
				N->Info = v;
			}
		}
	}

	//we remove the useless nodes
	ist_remove_node_without_son(result);

	//If the IST is not empty, we apply the effect of the function
	if (ist_is_empty(result) == false) {
		for (i = 0, L = result->FirstLayer; i < abs->nbV; i++, L = L->Next) {
			N = L->FirstNode;
	    		while (N != NULL) {
				if ((N->Info->Right != INFINITY) &&
					(N->Info->Right + t->transition[rule].cmd_for_place[i].delta <=
					abs->bound[i])) 
					ist_assign_values_to_interval(N->Info,0,N->Info->Right + t->transition[rule].cmd_for_place[i].delta);
					
				else
					ist_assign_values_to_interval(N->Info,0,N->Info->Right = INFINITY);
				N = N->Next;
			}
		}	
	}
	return result;
}

ISTSharingTree *ist_abstract_post(ISTSharingTree * S, abstraction_t * abs, transition_system_t *t) {

	int i;
	ISTSharingTree * result;
	ISTSharingTree * tmp;
	ISTSharingTree * tmp2;

	printf("entree abstract_post\n");
	
	ist_new(&result);
	for(i=0;i< t->limits.nbr_rules;i++) {
		tmp = ist_abstract_post_of_rules(S,abs,t,i);
		if ( ist_exact_subsumption_test(tmp,result) == false) {
			tmp2 = ist_union(tmp,result);
			ist_dispose(tmp);
			ist_dispose(result);
			result = tmp2;
		} else {
			ist_dispose(tmp);
		}
	}
	return result;
}

ISTSharingTree *ist_abstract_post_star(ISTSharingTree * initial_marking, abstraction_t * abs, transition_system_t *t) {
	ISTSharingTree * S;
	ISTSharingTree * tmp;
	ISTSharingTree * tmp2;

	printf("entree abstract_post_star\n");
	
	S = ist_copy(initial_marking);

	while (true) {
		tmp = ist_abstract_post(S,abs,t);
		if ( ist_exact_subsumption_test(tmp,S) == false) {		
			tmp2 = ist_union(tmp,S);
			ist_dispose(S);
			ist_dispose(tmp);
			S = tmp2;
		} else {
			ist_dispose(tmp);
			break;
		}
	}
	return S;	
}

/*
 * apply the (0,...,k,INFINITY) abstraction
 */

void abstract_bound(ISTSharingTree *S, abstraction_t * abs) 
{
	ISTLayer * L;
	ISTNode * N;
	int i;

	for(L = S->FirstLayer, i = 0 ; L != S->LastLayer ; L = L->Next, i++) 
		for(N = L->FirstNode ; N != NULL ; N = N->Next) 
			if (N->Info->Right > abs->bound[i])
				ist_assign_values_to_interval(N->Info,N->Info->Left,INFINITY);
}


/*
 * compute the pretild for one transition t for the abstract system
 */
ISTSharingTree * abstract_place_pretild_rule(ISTSharingTree * S, abstraction_t * abs, transition_system_t *t, int rule) 
{
	ISTSharingTree * result = NULL;
	ISTSharingTree * temp;
	ISTLayer * L;
	int i;
	boolean top;

	if (ist_is_empty(S) == false) {
	
		for (i=0,L = S->FirstLayer, top = false; (i < abs->nbV) && (top == false);i++,L = L->Next) {
			if ((t->transition[rule].cmd_for_place[i].guard.Left > 0) && 
			(t->transition[rule].cmd_for_place[i].places_abstracted > 1)) {
				top = true;
			}
		}
		if (top == false) {
			temp = ist_copy(S);
			ist_complement(temp,abs->nbV);
			result = ist_pre_of_transfer(temp, &t->transition[rule]);
			ist_dispose(temp);
			ist_complement(result,abs->nbV);
		}
		else {
			ist_new(&result);
			ist_complement(result,abs->nbV);
		}
	} else ist_new(&result);
	return result;	
}


/*
 * compute the abstrat pretild for the abstract system for all transitions
 *
 */

ISTSharingTree * abstract_place_pretild(ISTSharingTree * S, abstraction_t * abs, transition_system_t *t)
{
	ISTSharingTree * result;
	ISTSharingTree * temp1;
	ISTSharingTree * temp2;
	int i;

	ist_new(&result);
	ist_complement(result,abs->nbV);
	for(i = 0; i < t->limits.nbr_rules; i++) {
		temp1 = abstract_place_pretild_rule(S,abs,t,i);
		temp2 = ist_intersection(result,temp1);
		ist_dispose(result);
		ist_dispose(temp1);
		result= temp2;
	}
	
	return result;
}

/*
 * compute the abstract pretild
 *
 */

ISTSharingTree *abstract_pretild(ISTSharingTree * S, abstraction_t * abs, transition_system_t *t)
{
	ISTSharingTree * result;
	result = abstract_place_pretild(S,abs,t);
	abstract_bound(result,abs);
	return result;	
}

void release_abstraction(abstraction_t *abs)
{
	size_t i;
	for(i=0;i<abs->nbV;++i)
		free(abs->A[i]);
	free(abs->bound);
	free(abs);
}
