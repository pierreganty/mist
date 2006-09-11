#include "concretisation.h"

		
void ist_add_variables(ISTSharingTree *S,int nb_var) {

	int i;
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


ISTSharingTree * ist_concretisation(ISTSharingTree *S, transition_system_t *sys,transition_system_t *abstract_sys, abstraction_t * abs){

	int i;
	int j;
	ISTSharingTree * temp;
	ISTSharingTree * temp2;
	ISTSharingTree * result;
	ISTLayer * L;
	ISTNode * N;
	transition_t * t = (transition_t *) xmalloc(sizeof(transition_t));
	boolean * in_abs = (boolean *)xmalloc( sys->limits.nbr_variables * sizeof(boolean));
	int * mask;

	/*
	 *
	 * initialisation
	 *
	 *
	 */
	for(i = 0; i < sys->limits.nbr_variables;i++) {
		in_abs[i] = false;
	}
	
	/*
	 * adding of concrete variables
	 *
	 */
	
	temp = ist_copy(S);
	ist_add_variables(temp,sys->limits.nbr_variables);
	temp2 = ist_copy(temp);
	/*
	 *
	 * construction of the transfers that defines the mapping from concrete
	 * variables to abstract variables
	 *
	 */
	t->nbr_transfers = abstract_sys->limits.nbr_variables;	
	for(i=0;i< abstract_sys->limits.nbr_variables;i++) {
		t->transfers[i].target = i;
		t->transfers[i].origin = (int *) xmalloc
			((sys->limits.nbr_variables + abstract_sys->limits.nbr_variables) * sizeof(int)); 
		for(j=0;j < abstract_sys->limits.nbr_variables;j++) {
			t->transfers[i].origin[j] = 0;
		}
		for(j=0; j < sys->limits.nbr_variables;j++) {
			if (abs->A[i][j] != 0) {
				t->transfers[i].origin[abstract_sys->limits.nbr_variables + j] = abs->A[i][j];
				in_abs[j] = true;
			}
		}
	}
	/*
	 * Computation of the abstract value
	 *
	 */
	
	temp2 = ist_pre_of_transfer(temp,t,sys->limits.nbr_variables + abstract_sys->limits.nbr_variables);
	ist_dispose(temp);
	for(i=0; i < abstract_sys->limits.nbr_variables;i++) 
		xfree(t->transfers[i].origin);
	xfree(t);	
	/*
	 * projection to only keep the concrete variables
	 *
	 */
	mask = (int *) xmalloc(
			(sys->limits.nbr_variables + abstract_sys->limits.nbr_variables+1) * sizeof(int));
	for(i = 0; i < abstract_sys->limits.nbr_variables;i++) 
		mask[i] = 0;
	for(i = abstract_sys->limits.nbr_variables; i < abstract_sys->limits.nbr_variables + sys->limits.nbr_variables; i++) 
		mask[i] = 1;
	mask[abstract_sys->limits.nbr_variables + sys->limits.nbr_variables] = 1;
	result = ist_projection(temp2,mask);
	ist_dispose(temp2);
	xfree(mask);
	/*
	 *assignment of variables not in abstraction
	 */
	for(i = 0, L = result->FirstLayer; i < sys->limits.nbr_variables; i++,L=L->Next) {
		if (in_abs[i] == false) {
			N = L->FirstNode;
			while (N != NULL) {
				ist_assign_values_to_interval(N->Info,0,INFINITY);
				N = N->Next;
			}
		}		
	}
	return result;
}
