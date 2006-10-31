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

abstraction_t *glb(abstraction_t *abs1, abstraction_t *abs2) 
{
	abstraction_t *retval;
	size_t i,j,k,rows,sum;
	assert(abs1->nbConcreteV==abs2->nbConcreteV);

	/* Count the number of sets in the resulting partition */
	rows=0;
	for (i=0;i<abs1->nbV;++i) {
		for (j=0;j<abs2->nbV;++j) {
			/* We look for an common place */
			for(k=0; k<abs1->nbConcreteV && abs1->A[i][k]+abs2->A[j][k]< 2; ++k);
			if(abs1->A[i][k]+abs2->A[j][k]>= 2)
				++rows;
				
		}
		
	}
	/* Memory allocation */
	retval=(abstraction_t *)xmalloc(sizeof(abstraction_t));
	/* We copy the number of places of the original system into the abstraction */
	retval->nbConcreteV=abs1->nbConcreteV;
	/* We start with a unique abstract place */
	retval->nbV=rows;
	retval->bound=(integer16 *)xmalloc(retval->nbV*sizeof(integer16));
	retval->A=(integer16 **)xmalloc(retval->nbV*sizeof(integer16));
	for(i=0;i<retval->nbV;++i)
		retval->A[i]=(integer16 *)xmalloc(retval->nbConcreteV*sizeof(integer16));
	/* Initial abstraction */
	for(i=0;i<retval->nbV;++i) {
		retval->bound[i]=1;
	}
	rows=0;
	for (i=0;i<abs1->nbV;++i) {
		for (j=0;j<abs2->nbV;++j) {
			for(k=0; k<abs1->nbConcreteV; ++k) {
				/* set to 1 if k is a common place of abs1[i] and abs2[j] */
				retval->A[rows][k]=(abs1->A[i][k]+abs2->A[j][k]>= 2 ? 1 : 0);
			}
			for(sum=0,k=0;k<retval->nbConcreteV;sum+=retval->A[rows][k++]);
			rows+=(sum>=1?1:0);
			assert(rows<=retval->nbV);
		}
	}
	return retval;
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

/* ASSUME row has at least two entries greater or equal than 1 */
static integer16 *refined_variables(integer16 *row, ISTSharingTree *S, ISTSharingTree *cpreS)
{
	size_t length, j;
	integer16 *NewB;
	length=ist_nb_layers(S)-1;
	ISTLayer *L,*_L;
	ISTNode *N,*_N;
	boolean matched;

	/* We allocate and initialize the value to return */
	NewB = (integer16 *)xmalloc(length*sizeof(integer16));
	for(j=0;j<length;NewB[j++]=0); 

	L = S->FirstLayer;
	_L = cpreS->FirstLayer;
	j=0;
	while(L!=NULL && _L!=NULL){
		if(row[j]==1){
			/* Does a new bound appear in cpreS ? */
			_N=_L->FirstNode;
			while(_N!=NULL) {
				/* Take a node in cpreS layer j and search for a samed
				 * node in S layer j if so NewB[j]==1 */
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
			/* Does a bound disappear in S ? */
			N=L->FirstNode;
			while(N!=NULL) {
				/* Take a node in cpreS layer j and search for a samed
				 * node in S layer j if so NewB[j]==1 */
				_N=L->FirstNode;
				matched=false;
				while(_N!=NULL){
					if(ist_equal_interval(_N->Info,N->Info))
						matched=true;
					_N=_N->Next;
				}
				if (matched==false)
					NewB[j]=1;
				N=N->Next;
			}
		}
		L=L->Next;
		_L=_L->Next;
		++j;
	}
	return NewB;
}

static abstraction_t *add_row(abstraction_t *abs, size_t nb_row, integer16 *NewB)
{

	size_t j,k;
	abstraction_t *retval;

	retval = (abstraction_t *)xmalloc(sizeof(abstraction_t));
	/* we add exactly one row  */
	retval->nbV=abs->nbV+1;
	retval->nbConcreteV = abs->nbConcreteV;
	retval->A=(integer16 **)xmalloc(retval->nbV*sizeof(integer16));
	retval->bound=(integer16 *)xmalloc(retval->nbV*sizeof(integer16));
	/* we copy abs into retval */
	for(k=0;k<retval->nbV;++k) {
		retval->bound[k]=1;
		retval->A[k]=(integer16 *)xmalloc(retval->nbConcreteV*sizeof(integer16));
		for(j=0;j<retval->nbConcreteV;++j)
			/* the added row is set to 0's. */
			retval->A[k][j]=(k < abs->nbV ? abs->A[k][j] : 0);
	}
	/* we split the nb_row according to NewB */
	if (NewB != NULL) {
		/* If NewB is NULL we do nothing */
		for(j=0;j<retval->nbConcreteV;++j){
			if(NewB[j]==1) {
				retval->A[nb_row][j] = 0;
				retval->A[retval->nbV-1][j]= 1;
			}
		}
	}
	return retval;
}


abstraction_t *refine_abs(cur_abs, S, cpreS, safe, sys)
	abstraction_t *cur_abs;
	ISTSharingTree *S, *cpreS, *safe;
	transition_system_t *sys;
{
	abstraction_t *refined_abs, *p;
	ISTSharingTree *cpre_t_S, *tmp, *_tmp;
	integer16 *NewB, sum, sumB;
	size_t i, j, k, splitted, nb_row;

	p=cur_abs;

	/* For each row i in the current abstraction */
	for(i=0;i<cur_abs->nbV;++i) {
		/* compute sum */
		for(sum=0,j=0;j<p->nbConcreteV;sum+=cur_abs->A[i][j++]);
		if(sum > 1){
			/* there are places to separate on row i */
			NewB = refined_variables(p->A[i],S,cpreS);
			for(sumB=0,j=0;j<p->nbConcreteV;sumB+=NewB[j++]);
			if (0 < sumB) {
				/* We have to refine this row */
				if (sumB < sum)	{
					/* CASE 1: Some but not all bounds are refined */
					refined_abs = add_row(p,i,NewB);
					/* to not modify cur_abs which is a in parameter */
					if (p!= cur_abs)
						dispose_abstraction(p);
					p = refined_abs;
				} 
				xfree(NewB);	
				if (sumB == sum) {
					/* CASE 2: All the bounds of row i are refined. */
					nb_row = p->nbV;
					/* We remember how many rows counts the abstraction p */
					for (k=0; nb_row==p->nbV && k<sys->limits.nbr_rules;++k){
						tmp = adhoc_pretild_rule(S,&sys->transition[k]);
						_tmp = ist_intersection(tmp,safe);
						ist_dispose(tmp);
						tmp = ist_downward_closure(_tmp);
						ist_normalize(tmp);
						ist_dispose(_tmp);
						cpre_t_S = ist_minimal_form(tmp);
						ist_dispose(tmp);
						assert(ist_checkup(cpre_t_S)==true);
						/* This time using cpre_t_S */
						NewB = refined_variables(p->A[i],S,cpre_t_S);
						for(sumB=0,j=0;j<p->nbConcreteV;sumB+=NewB[j++]);
						if (0 < sumB && sumB < sum)	{
							/* Some but not all bounds are refined */
							refined_abs = add_row(p,i,NewB);
							/* to not modify cur_abs which is a in parameter */
							if (p!= cur_abs)
								dispose_abstraction(p);
							p = refined_abs;
							/* recompute sum since p->A[i] has been modified */
							for(sum=0,j=0;j<p->nbConcreteV;sum+=p->A[i][j++]);
						}
						xfree(NewB);
						ist_dispose(cpre_t_S);
					}
					if (nb_row == p->nbV) {
						/* CASE 2a: No clue about separating places of row i.
						 * So, we carry on an arbitrary split of row i */
						refined_abs=add_row(p,i,NULL);
						/* refined_abs is p w/ one row more */
						for(splitted=0,j=0;j<refined_abs->nbConcreteV;++j) {
							if(refined_abs->A[i][j]==1 && splitted < (integer16) sum/2 ) {
								refined_abs->A[i][j] = 0;
								refined_abs->A[refined_abs->nbV-1][j] = 1;
								++splitted;
							}
						}
						/* to not modify cur_abs which is a in parameter */
						if (p!= cur_abs)
							dispose_abstraction(p);
						p = refined_abs;
					} else
						printf("%d rows added by the trans/trans \
								refinement\n",p->nbV-nb_row);
				}
			} else
				/* We do nothing but release the NewB array. */
				xfree(NewB);
		}
	}
	if (p==cur_abs) {
		/* So far no refinenement has been proposed. So we split a non singleton row */
		for(i=0;i<cur_abs->nbV;++i) {
			/* compute sum */
			for(sum=0,j=0;j<cur_abs->nbConcreteV;sum+=cur_abs->A[i][j++]);
			if(sum > 1) {
				p=add_row(cur_abs,i,NULL);
				/* we add a row and we split row i into two sets. */
				for(splitted=0,j=0;j<p->nbConcreteV;++j) {
					if(p->A[i][j]==1 && splitted < (integer16) sum/2 ) {
						p->A[i][j] = 0;
						p->A[p->nbV-1][j] = 1;
						++splitted;
					}
				}
			}
		}
	}
	/* A sanity check */
	for(i=0;i<p->nbV;++i) {
		for(sum=0,j=0;j<p->nbConcreteV;sum+=p->A[i][j++]);
		assert(sum>0);
	}
	return p;
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
ISTSharingTree *adhoc_pretild_rule(ISTSharingTree *S, transition_t *t) 
{
	ISTSharingTree *result = NULL;
	ISTSharingTree *temp;
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
		temp1 = adhoc_pretild_rule(S,&t->transition[i]);
		temp2 = ist_intersection(result,temp1);
		ist_dispose(result);
		ist_dispose(temp1);
		result= temp2;
	}
	return result;
}
