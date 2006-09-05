#include "concretisation.h"

ISTHeadListNode * NoProject(ISTNode * node,ISTSharingTree * STR, ISTLayer * rlayer, int nlayer,int* mask);
ISTNode *  YesProject(ISTNode * node,ISTSharingTree * STR, ISTLayer * rlayer, int nlayer,int* mask) {
	ISTSon *s;
	ISTNode *rchild;
	ISTNode *rnode;
	ISTHeadListNode * list;
	ISTHeadListNode * list_tmp;
	TMemo1 *memo;

	
	printf("YesProject: entrée\n");
	
	if (ist_equal_interval(node->Info,&IST_end_of_list))
		rnode = ist_add_node(rlayer, ist_create_node(&IST_end_of_list));
	else {
		ist_init_list_node(&list_tmp);
		rnode = ist_create_node(node->Info);
		rlayer = rlayer->Next;
		if (rlayer == NULL) 
			rlayer = ist_add_last_layer(STR);
		
		for(s=node->FirstSon;s != NULL;s=s->Next){
			printf("YesProject: traitement fils\n");
			printf("YesProject: layer suivant: %d\n",nlayer+1);
			if (mask[nlayer+1] == 0) {
				
				printf("YesProject: On va faire un NoProject\n");

				list = NoProject(s->Son,STR,rlayer,nlayer+1,mask);
				
				while(ist_is_empty_list_node(list) == false)
					ist_insert_list_node_without_redundancy(list_tmp,
							ist_remove_first_elem_list_node(list));
				xfree(list);
			} else {
				printf("YesProject: On va faire un YesProject\n");
				memo = ist_get_memoization1(s->Son, s->Son);
				if (memo != NULL)
					rchild = memo->r;
				else 
					rchild = YesProject(s->Son,STR,rlayer,nlayer+1,mask);
				ist_add_son(rnode,rchild);
				
			}
		}
		/*if the next layer is not projected, we add the new sons */
		if (mask[nlayer+1] == 0) {
			for(rchild = ist_remove_first_elem_list_node(list_tmp); rchild != NULL;
					rchild = ist_remove_first_elem_list_node(list_tmp)) {
				ist_add_son(rnode,rchild);
			}
		}
		rlayer = rlayer->Previous;
		rnode = ist_add_node(rlayer,rnode);
		xfree(list_tmp);
	}
	ist_put_memoization1(node,node,rnode);
	return rnode;	  
}

    
ISTHeadListNode * NoProject(ISTNode * node,ISTSharingTree * STR, ISTLayer * rlayer, int nlayer,int* mask) {

	ISTSon *s;
	ISTNode *rchild;
	ISTHeadListNode * list;
	ISTHeadListNode * list_temp;
	TMemo1 *memo;

	printf("NoProject: entrée\n");
	
	ist_init_list_node(&list);

	printf("NoProject: pour tout fils\n");
	
	for(s = node->FirstSon; s != NULL; s = s->Next) {

		printf("NoProject: traitement d'un fils\n");
		printf("NoProject: layer: %d\n",nlayer);
		
		if(mask[nlayer+1] > 0) {

			printf("NoProject: fils projeté\n");
		
			memo = ist_get_memoization1(s->Son, s->Son);

			printf("NoProject: on regarde si le fils a déjà été traité\n");

			if (memo != NULL)
				ist_insert_list_node(list,memo->r);
			else 
				ist_insert_list_node(list,YesProject(s->Son,STR,rlayer,nlayer+1,mask));
		} else {

			printf("NoProject: fils non-projeté\n");
			
			list_temp = NoProject(s->Son,STR,rlayer,nlayer+1,mask);
			for(rchild = ist_remove_first_elem_list_node(list_temp) ; rchild != NULL; 
					rchild = ist_remove_first_elem_list_node(list_temp)) 
				ist_insert_list_node(list,rchild);
			xfree(list_temp);
		}
	}
	return list;
}


/*
 *
 * we make the assumption that we keep the last layer
 *
 */
ISTSharingTree * ist_projection(ISTSharingTree * S, int *mask) {
	ISTSharingTree * STR;
	ISTLayer * rlayer;
	ISTSon * s;
	ISTHeadListNode * list;
	ISTHeadListNode * list_tmp;
	
	ist_new(&STR);
	if (ist_is_empty(S) == false) {
		ist_new_magic_number();
		ist_new_memo1_number();
		ist_init_list_node(&list_tmp);
		rlayer = ist_add_last_layer(STR);
		for(s = S->Root->FirstSon; s != NULL;s = s->Next) {
			printf("Premier fils du root\n");
			if (mask[0] > 0)
				ist_add_son(STR->Root,YesProject(s->Son,STR,rlayer,0,mask));
			else {
				printf("ist_projection: On ne projette pas la premiere variable\n");
				list = NoProject(s->Son,STR,rlayer,0,mask);

				while (ist_is_empty_list_node(list) == false) {
					ist_insert_list_node_without_redundancy(list_tmp,
							ist_remove_first_elem_list_node(list));
				}
				xfree(list);
			}
		}
		/*if the first variable is projected, we add sons to root*/
		while (ist_is_empty_list_node(list_tmp) == false) {
			ist_add_son(STR->Root,ist_remove_first_elem_list_node(list_tmp));
		}
		
		xfree(list_tmp);
		ist_normalize(STR);
	}
	return STR;
}
		
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
	transition_t * t = (transition_t *) xmalloc(sizeof(transition_t));
	int * mask;

	printf("ist_concretisation:on rentre\n");
	
	/*
	 * adding of concrete variables
	 *
	 */
	
	temp = ist_copy(S);

	ist_add_variables(temp,sys->limits.nbr_variables);

	printf("apres add_variables\n");
	ist_write(temp);
	ist_checkup(temp);

	printf("une copie de temp=\n");
	temp2 = ist_copy(temp);
	ist_checkup(temp2);
	ist_write(temp2);
	ist_write(temp);
	ist_checkup(temp);
	
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
			t->transfers[i].origin[abstract_sys->limits.nbr_variables + j] = abs->A[i][j];
		}
	}


	printf("nb_transfers = %d\n",t->nbr_transfers);
	for(i=0;i< abstract_sys->limits.nbr_variables;i++) {
		printf("t->transfers[%d].target = %d\n",i,t->transfers[i].target);
		for(j=0;j < abstract_sys->limits.nbr_variables +sys->limits.nbr_variables ;j++) {
			printf("t->transfers[%d].origin[%d] = %d",i,j,t->transfers[i].origin[j]);
		}
	}
	
	printf("ist_concretisation: construction de l'abstraction\n");
	ist_write(temp);
	ist_checkup(temp);
	/*
	 * Computation of the abstract value
	 *
	 */
	
	temp2 = ist_pre_of_transfer(temp,t,sys->limits.nbr_variables + abstract_sys->limits.nbr_variables);

	printf("sys->limits.nbr_variables + abstract_sys->limits.nbr_variables = %d\n",sys->limits.nbr_variables + abstract_sys->limits.nbr_variables);
	printf("ist_concretisation: on a fait le pre\n");
	ist_write(temp);
	ist_write(temp2);
	ist_checkup(temp);
	ist_checkup(temp2);

	printf("On dispose\n");	
	ist_dispose(temp);
	printf("1)\n");
	for(i=0; i < abstract_sys->limits.nbr_variables;i++) 
		xfree(t->transfers[i].origin);
	printf("2)\n");
	xfree(t);	

	printf("on a fini de disposer\n");

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

	printf("on a fait la projection\n");
	
	ist_dispose(temp2);
	xfree(mask);
	
	return result;
}
