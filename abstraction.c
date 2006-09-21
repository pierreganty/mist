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
#include<assert.h>

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

boolean exists_unbounded_node(layer)
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

boolean exists_bounded_node(layer)
	ISTLayer *layer;
{
	ISTNode *Node=layer->FirstNode;
	while(Node!=NULL){
		if(ist_is_unbounded(Node->Info)==false) {
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
	transition_t * t = (transition_t *)xmalloc(sizeof(transition_t));
	int *mask;

	if (ist_is_empty(S) == false) {
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
			t->transfers[i].origin = (integer16 *)xmalloc((abs->nbConcreteV + abs->nbV)*sizeof(integer16)); 
			for(j=0;j < abs->nbConcreteV;j++) 
				t->transfers[i].origin[j] = abs->A[i][j];
			for(j=0; j < abs->nbV;j++)
				t->transfers[i].origin[abs->nbConcreteV + j] = 0;
		}
		/* Computation of the abstract values */
		for(i=0; i<t->nbr_transfers;++i) {
			temp2 = ist_post_of_transfer(temp,&t->transfers[i]);
			ist_dispose(temp);
			temp = temp2;
		}
		for(i=0; i < abs->nbV;i++) 
			xfree(t->transfers[i].origin);
		xfree(t);	
		/* projection to only keep the concrete variables */
		mask = (integer16 *) xmalloc((abs->nbConcreteV+abs->nbV+1)*sizeof(integer16));
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
	} else ist_new(&result);
	return result;
}

/* We assume exists i s.t. sum cur_abs->A[i][j] > 1 */
abstraction_t *refine_abs(cur_abs, S, cpreS)
	abstraction_t *cur_abs;
	ISTSharingTree *S, *cpreS;
{
	abstraction_t *retval;
	ISTLayer *L, *_L;
	ISTNode *N, *_N;
	integer16 *NewB, sum, sumB;
	size_t i, j, k, splitted, Rows2Add=0, *Rows2Split=NULL;
	boolean matched, nl_added=false;

	assert(ist_nb_layers(S)==ist_nb_layers(cpreS));

	NewB = (integer16 *)xmalloc(cur_abs->nbConcreteV*sizeof(integer16));
	retval=(abstraction_t *)xmalloc(sizeof(abstraction_t));
	/* The number of concrete places never changes */
	retval->nbConcreteV=cur_abs->nbConcreteV;

	/* For each row i */
	for(i=0;i<cur_abs->nbV;++i) {
		/* We set NewB to 0's and we count the non null entries of cur_abs->A[i] */
		for(sum=0,j=0;j<cur_abs->nbConcreteV;++j) {
			NewB[j]=0;
			sum+=cur_abs->A[i][j];
		}
		if(sum>1){
			/* Row i is not a singleton */
			L = S->FirstLayer;
			_L = cpreS->FirstLayer;
			j=0;
			while(L!=NULL && _L!=NULL){
				if(cur_abs->A[i][j]==1){
					_N=_L->FirstNode;
					while(_N!=NULL) {
						/* Take a node in cpreS layer j and search for a samed
						 * node in S layer j if so NewB[j]==0 */
						N=L->FirstNode;
						matched=false;
						while(N!=NULL){
							if(ist_equal_interval(_N->Info,N->Info))
								matched=true;
							N=N->Next;
						 }
						if (matched==false)
							NewB[j]=1;
						_N=_N->Next;
					}
				}
				L=L->Next;
				_L=_L->Next;
				++j;
			}
			for(sumB=0,j=0;j<cur_abs->nbConcreteV;sumB+=NewB[j++]);
			if(sumB==0) {
				/* No bound of row i are refined */
			} else {
				/* i is a non singleton row and some bound are refined */
				assert(sumB<=sum);
				if (sumB==sum) {
					/* all the bounds of row i are refined we remember it for
					 * later if we have no better split to make */
					/* We append one entry to Rows2Split, and we remember i in it */ 
					++Rows2Add;
					Rows2Split = (size_t *)xrealloc(Rows2Split,Rows2Add*sizeof(size_t));
					Rows2Split[Rows2Add-1]=i;
				} else {
					/* some (but not all) bounds of row i are refined we split
					 * row i according to NewB */
					/* we add exactly one row */
					retval->nbV=(cur_abs->nbV+1);
					retval->A=(integer16 **)xmalloc(retval->nbV*sizeof(integer16));
					retval->bound=(integer16 *)xmalloc(retval->nbV*sizeof(integer16));
					for(k=0;k<retval->nbV;++k){
						retval->A[k]=(integer16 *)xmalloc(cur_abs->nbConcreteV*sizeof(integer16));
						/* The new abstraction is a copy of the current one with a new line w/
						 * 0's. */
						for(j=0;j<cur_abs->nbConcreteV;++j)
							retval->A[k][j]=(k < cur_abs->nbV ? cur_abs->A[k][j] : 0);
						retval->bound[k]=1;
					}
					for(j=0;j<cur_abs->nbConcreteV;++j){
						if(NewB[j]==1) {
							retval->A[i][j] = 0;
							retval->A[retval->nbV-1][j]= 1;
						}
					}
					nl_added=true;
				}
			}
			/* Once a place is added we stop */
			if(nl_added==true)
				break;
		}
		/* we process the last row of cur_abs and no row has been added to retval  */
		if(i==cur_abs->nbV-1 && nl_added==false) {
			/* we add exactly Rows2Add rows */
			retval->nbV=(cur_abs->nbV+Rows2Add);
			/* we realloc memory for retval */
			retval->A=(integer16 **)xmalloc(retval->nbV*sizeof(integer16));
			retval->bound=(integer16 *)xmalloc(retval->nbV*sizeof(integer16));

			for(k=0;k<retval->nbV;++k){
				retval->A[k]=(integer16 *)xmalloc(cur_abs->nbConcreteV*sizeof(integer16));
				/* The new abstraction is a copy of the current one with for the Rows2Add last lines w/
				 * 0's. */
				for(j=0;j<cur_abs->nbConcreteV;++j)
					retval->A[k][j]= (k<cur_abs->nbV ? cur_abs->A[k][j] : 0);
				retval->bound[k]=1;
			}
			/* we split each of Rows2Split. Btw we have NewB == cur_abs->A[for each Rows2Split] */
			for(k=0;k<Rows2Add;++k){
				/* we compute the sum for row Rows2Split[k] */
				assert(Rows2Split[k]<cur_abs->nbV+k);
				for(sum=0,j=0;j<cur_abs->nbConcreteV;sum+=cur_abs->A[Rows2Split[k]][j++]);
				for(splitted=0,j=0;j<cur_abs->nbConcreteV;++j) {
					if(cur_abs->A[Rows2Split[k]][j]==1 && splitted < (integer16) sum/2 ) {
						retval->A[Rows2Split[k]][j] = 0;
						retval->A[cur_abs->nbV+k][j] = 1;
						++splitted;
					}
				}
			}
			printf("Row(s)");
			for(k=0;k<Rows2Add;++k)
				printf(" %d",Rows2Split[k]);
			printf(" splitted\n");

		}
	}
	if(Rows2Split!=NULL)
		xfree(Rows2Split);
	return retval;
}

ISTSharingTree *ist_symbolic_post_of_rules(ISTSharingTree * S, abstraction_t * abs, transition_system_t *t, int rule) 
{
	ISTSharingTree * result;
	ISTLayer * L;
	ISTNode * N;
	size_t i;
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
	/* If the IST is not empty, we apply the effect of the function */
	if (ist_is_empty(result) == false) {
		for (i = 0, L = result->FirstLayer; i < abs->nbV; i++, L = L->Next) {
			for(N = L->FirstNode;N != NULL;N=N->Next) {
					ist_assign_values_to_interval(N->Info, 
							N->Info->Left + t->transition[rule].cmd_for_place[i].delta,
							N->Info->Right + t->transition[rule].cmd_for_place[i].delta);
			}
		}	
	} 
	return result;
}

ISTSharingTree *ist_symbolic_post(ISTSharingTree * S, abstraction_t * abs, transition_system_t *t) {

	size_t i;
	ISTSharingTree *result, *tmp, *tmp2;
	
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
	ISTSharingTree *result, * tmp;
	ISTLayer * L;
	ISTNode * N;
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
		ist_normalize(result);
		tmp = ist_minimal_form(result);
		ist_dispose(result);
		result = tmp;
	}
	return result;
}

ISTSharingTree *ist_abstract_post(ISTSharingTree * S, abstraction_t * abs, transition_system_t *t) 
{
	size_t i;
	ISTSharingTree * result, * tmp, * tmp2;

	ist_new(&result);
	for(i=0;i< t->limits.nbr_rules;i++) {
		tmp = ist_abstract_post_of_rules(S,abs,t,i);
		if (ist_exact_subsumption_test(tmp,result) == false) {
			tmp2 = ist_union(tmp,result);
			ist_dispose(tmp);
			ist_dispose(result);
			result = ist_minimal_form(tmp2);
			ist_dispose(tmp2);
		} else 
			ist_dispose(tmp);
	}
	return result;
}

ISTSharingTree *ist_abstract_post_star(ISTSharingTree * initial_marking, abstraction_t * abs, transition_system_t *t) {
	ISTSharingTree * S, * tmp, * tmp2;

	S = ist_copy(initial_marking);
	while (true) {
		tmp = ist_abstract_post(S,abs,t);
		if (ist_exact_subsumption_test(tmp,S) == false) {		
			tmp2 = ist_union(tmp,S);
			ist_dispose(S);
			ist_dispose(tmp);
			S = ist_minimal_form(tmp2);
			ist_dispose(tmp2);
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
 * compute the adhoc pretild for one transition t
 */
ISTSharingTree *adhoc_place_pretild_rule(ISTSharingTree * S, transition_t *t) 
{
	ISTSharingTree * result = NULL;
	ISTSharingTree * temp;
	size_t i;
	boolean top;

	if (ist_is_empty(S) == false) {
		for (i=0, top = false; (i < ist_nb_layers(S)-1) && (top == false);i++) {
			if ((t->cmd_for_place[i].guard.Left > 0) && 
			(t->cmd_for_place[i].places_abstracted > 1))
				top = true;
		}
		if (top == false) {
			temp = ist_copy(S);
			ist_complement(temp,ist_nb_layers(S)-1);
			result = ist_pre_of_rule_plus_transfer(temp,t);
			ist_dispose(temp);
			ist_complement(result,ist_nb_layers(S)-1);
		} else {
			ist_new(&result);
			ist_complement(result,ist_nb_layers(S)-1);
		}
	} else 
		ist_new(&result);
	return result;	
}


/*
 * compute the adhoc pretild for all transitions
 */
ISTSharingTree *adhoc_pretild(ISTSharingTree *S, transition_system_t *t)
{
	ISTSharingTree *result, *temp1, *temp2;
	size_t i;

	ist_new(&result);
	ist_complement(result,ist_nb_layers(S)-1);
	for(i = 0; i < t->limits.nbr_rules; i++) {
		temp1 = adhoc_place_pretild_rule(S,&t->transition[i]);
		temp2 = ist_intersection(result,temp1);
		ist_dispose(result);
		ist_dispose(temp1);
		result= temp2;
	}
	return result;
}

void dispose_abstraction(abstraction_t *abs)
{
	size_t i;
	for(i=0;i<abs->nbV;++i)
		free(abs->A[i]);
	free(abs->bound);
	free(abs);
}



