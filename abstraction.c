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
#include"xmalloc.h"
#include"checkup.h"
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

void dispose_abstraction(abstraction_t *abs)
{
	size_t i;
	if(abs->nbV > 0) {
		for(i=0;i<abs->nbV;++i)
			xfree(abs->A[i]);
		xfree(abs->bound);
	}
	xfree(abs);
}

void print_abstraction(abs)
	abstraction_t *abs;
{
	size_t i,j;
	printf("abs: %d variables.\n",abs->nbV);
	puts("abs: For the merging:");
	for(i=0;i<abs->nbV;++i){
		for(j=0;j<abs->nbConcreteV;++j)
			printf("%d",(int) abs->A[i][j]);
		puts("");
	}
	puts("abs: For the bounds:");
	for(i=0;i<abs->nbV;++i)
		printf("%d",(int) abs->bound[i]);
	puts("");

}

static void ist_add_variables(ISTSharingTree *S,integer16 nb_var) 
{
	size_t i;
	ISTLayer * L;
	ISTNode * N;
	ISTNode * N_tmp;
	ISTInterval * inter;

	inter = ist_build_interval(0,0);
	N = S->LastLayer->FirstNode;
	N->Info = inter;
	for (i=0;i< nb_var;i++) {
		L = ist_add_last_layer(S);
		N_tmp = ist_create_node(inter);
		ist_add_son(N,N_tmp);
		ist_add_node(L,N_tmp);
		N = N_tmp;
	}
	ist_dispose_info(S->LastLayer->FirstNode->Info);
	S->LastLayer->FirstNode->Info = &IST_end_of_list;
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


ISTSharingTree *ist_concretisation(ISTSharingTree *S, abstraction_t *abs)
{
	size_t i, j;
	ISTSharingTree * temp;
	ISTSharingTree * temp2;
	ISTSharingTree * result;
	ISTLayer * L;
	ISTNode * N;
	transition_t * t = (transition_t *)xmalloc(sizeof(transition_t));
	boolean * in_abs = (boolean *)xmalloc(abs->nbConcreteV*sizeof(boolean));
	integer16* mask;

	if (ist_is_empty(S) == false) {
		/* initialisation */
		for(i = 0; i < abs->nbConcreteV;i++)
			in_abs[i] = false;
	
		/* adding of concrete variables */
		temp = ist_copy(S);
		ist_add_variables(temp,abs->nbConcreteV);
		/*
		 * construction of the transfers that defines the mapping from concrete
		 * variables to abstract variables
		 */
		t->nbr_transfers = abs->nbV;	
		for(i=0;i< abs->nbV;i++) {
			t->transfers[i].target = i;
			t->transfers[i].origin = (integer16 *)xmalloc\
									 ((abs->nbConcreteV + abs->nbV)*sizeof(integer16)); 
			for(j=0;j < abs->nbV;j++)
				t->transfers[i].origin[j] = 0;
			for(j=0; j < abs->nbConcreteV;j++) {
				t->transfers[i].origin[abs->nbV + j] = abs->A[i][j];
				if (abs->A[i][j] != 0) {
					/* Place j is not ignored by the abstraction */
					in_abs[j] = true;
				} else t->transfers[i].origin[abs->nbV + j] = 0;
			}
		}
		/* Computation of the concrete values */
		temp2 = ist_pre_of_all_transfer(temp,t);
		ist_dispose(temp);
		for(i=0; i < abs->nbV;i++) 
			xfree(t->transfers[i].origin);
		xfree(t);	
		/* projection to only keep the concrete variables */
		mask = (integer16 *) xmalloc(
				(abs->nbConcreteV + abs->nbV+1) * sizeof(integer16));
		for(i = 0; i < abs->nbV;i++) 
			mask[i] = 0;
		/* i = abs->nbV */
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
		for(i = 0, L = result->FirstLayer; i < abs->nbConcreteV; i++,L=L->Next) {
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
						/* The new abstraction is a copy of the current one
						 * with a new line w/ 0's. */
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
				/* The new abstraction is a copy of the current one with for
				 * the Rows2Add last lines w/ 0's. We set bounds to 0 meaning
				 * that our refinement is built upon arbitrary criteria. */
				for(j=0;j<cur_abs->nbConcreteV;++j)
					retval->A[k][j]= (k<cur_abs->nbV ? cur_abs->A[k][j] : 0);
				retval->bound[k]=0;
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
		}
	}
	if(Rows2Split!=NULL)
		xfree(Rows2Split);
	return retval;
}


ISTSharingTree 
*ist_abstract_post_of_rules(ISTSharingTree *S, void (*approx)(ISTSharingTree
			*S, integer16 *b), integer16 *bound, transition_t *t) 
{
   ISTSharingTree *tmp;
   ISTSharingTree *res = ist_symbolic_post_of_rules(S,t);
   if (ist_is_empty(res) == false) {
      tmp = ist_downward_closure(res);
	  ist_dispose(res);
      if(approx)
		  approx(tmp,bound);
      ist_normalize(tmp);
	  res = ist_minimal_form(tmp);
	  ist_dispose(tmp);
   } 
   return res;
}

ISTSharingTree 
*ist_abstract_post(ISTSharingTree *S, void (*approx)(ISTSharingTree
			*S, integer16 *b), integer16 *bound, transition_system_t *t) 
{
	size_t i;
	ISTSharingTree *res, *tmp, *_tmp;

	ist_new(&res);
	for(i=0;i< t->limits.nbr_rules;i++) {
		tmp = ist_abstract_post_of_rules(S,approx,bound,&t->transition[i]);
		_tmp = ist_remove_subsumed_paths(tmp,S);
		ist_dispose(tmp);
		if (ist_is_empty(_tmp)==false) {
			tmp = ist_remove_subsumed_paths(S,_tmp);
			res = ist_union(tmp,_tmp);
			ist_dispose(tmp);
		}
		ist_dispose(_tmp);
	}
	return res;
}

/* Assume initial_marking is a downward closed marking and the ist is minimal */
ISTSharingTree 
*ist_abstract_post_star(ISTSharingTree *initial_marking, void
		(*approx)(ISTSharingTree *S, integer16* b), integer16 *bound,
		transition_system_t *t) 
{
	ISTSharingTree *S, *tmp, *_tmp;

	S = ist_copy(initial_marking);
	if(approx)
		approx(S,bound);
	ist_normalize(S);
	while (true) {
		tmp = ist_abstract_post(S,approx,bound,t);
		_tmp =  ist_remove_subsumed_paths(tmp,S);
		ist_dispose(tmp);
		if (ist_is_empty(_tmp)==false) {		
			tmp = ist_remove_subsumed_paths(S,_tmp);
			ist_dispose(S);
			S = ist_union(tmp,_tmp);
			ist_dispose(tmp);
			ist_dispose(_tmp);
		} else {
			ist_dispose(_tmp);
			break;
		}
	}
	return S;	
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
