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

   Copyright 2006, Laurent Van Begin
 */
#include "concretisation.h"

		
void ist_add_variables(ISTSharingTree *S,integer16 nb_var) {

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
				if (abs->A[i][j] != 0) {
					t->transfers[i].origin[abs->nbV + j] = abs->A[i][j];
					in_abs[j] = true;
				}
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
