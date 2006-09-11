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

   Copyright 2003, Pierre Ganty
 */

#include "precone.h"
#include "basis.h"
#include "minimizeinvarheuristic.h"
#include "minimize.h"
#include "transtree.h"
#include "listnode.h"
#include "remove.h"
#include "normalize.h"
#include "xmalloc.h"
#include "checkup.h"

#include <stdlib.h>
#include <stdio.h>


#ifdef TRANSFERT
/* 
 * All that part (fron #ifdef TRANSFERT ... #else) is only dedicated to sytem
 */
static boolean IsTarget(Place, transition)
	integer16 Place;
	transition_t *transition;
{
	size_t i;
	boolean Sol;
	Sol = false;
	i = 0;
	while ( i < transition->nbr_transfers  && !Sol){
		if (( transition->transfers[i].target) == Place){
			Sol = true;
		}
		++i;
	}
	return Sol;
}

static boolean IsOrigin(Place, transition)
	integer16 Place;
	transition_t *transition;
{

	boolean Sol;
	size_t i;
	Sol = false;
	i = 0;
	while ( i < transition->nbr_transfers && !Sol){
		if (transition->transfers[i].origin[Place] != 0){
			Sol = true;
		}
		++i;
	}
	return Sol;
}


static boolean KeepMarkingsSatisfyingPostCondition(S, transition)
	ISTSharingTree *S;
	transition_t *transition;
{
	ISTLayer *Layer;
	ISTNode *Node;
	ISTInterval *temp;
	size_t NuLayer;
	boolean modif = false;
	temp = ist_build_interval(0,0);
	Layer = S->FirstLayer;
	NuLayer = 0;
	while (Layer->Next != NULL){
		if (IsTarget(NuLayer,transition)==false && IsOrigin(NuLayer,transition) == true){
			Node = Layer->FirstNode;
			while (Node != NULL){
				if(ist_include_interval(Node->Info,temp)) {
					ist_assign_interval_to_interval(Node->Info,temp);
					modif = true;
				} else {
					ist_remove_sons(Node);
				}
				Node = Node->Next;
			}
		}
		Layer = Layer->Next;
		++NuLayer;
	}
	ist_remove_node_without_son(S);
	ist_remove_node_without_father(S);
	ist_dispose_info(temp);
	return modif;
}

static void SetNodesOfLayerToValue(Layer, Value)
	ISTLayer *Layer;
   	ISTInterval *Value;
{
	ISTNode* Node;
	Node = Layer->FirstNode;
	while (Node != NULL){
		ist_assign_interval_to_interval(Node->Info,Value);
		Node = Node->Next;
	}
}

static void ComputeOverApproximationOnLayerForValue(Layer, Value)
	ISTLayer *Layer;
   	ISTInterval *Value;
{
	/* 
	 * Here we have 'Value' that is the target value we are dealing with. In
	 * the target layer 'Layer' we have as many nodes as their previous value
	 * == 'Value' but now their value is equal to [0,0] or [0,\infty) according
	 * to the right bound of 'Value'. It's exactly the same situation in the
	 * origins layers.  The value of the right bound of nodes in target and
	 * origin layers is decided according to the value of the rightbound of the
	 * target 'Value'
	 */
	ISTNode *Node, *NewNode;
	ISTInterval *CurrentInterval, *Save, *Rightincr;
	ISTHeadListNode *listnode;
	long i,lenghtofinterv;
	ist_init_list_node(&listnode);
	lenghtofinterv =  (Value->Right == INFINITY ? INFINITY : (Value->Right - Value->Left));
	Rightincr = ist_build_interval(0,1);
	Save = ist_new_info();
	CurrentInterval = ist_new_info();
	Node = Layer->FirstNode;
	while (Node != NULL){
		Node->AuxP = NULL;
		/* We use AuxP to add all the new nodes as sons to the parents of
		 * 'Node' in the function : AdjustSonNode() */
		ist_assign_interval_to_interval(CurrentInterval,Node->Info);
		/* It's either [0,\infty) or [0,0] depending on the rightbound of
		 * 'Value'.  We deal the case of [0,?] (that is already in the layer)
		 * iff we play with intervals and not upper closed sets */
		if (lenghtofinterv != INFINITY){
			ist_assign_interval_to_interval(Save,CurrentInterval);
			for (i = 0; i< lenghtofinterv; ++i){
				ist_add_interval_to_interval(CurrentInterval,Rightincr);
				NewNode = ist_create_node(CurrentInterval);
				ist_copy_sons(Node,NewNode);
				ist_insert_list_node(listnode,NewNode); 
				NewNode->AuxP = Node->AuxP;
				Node->AuxP = NewNode;
			}
			ist_assign_interval_to_interval(CurrentInterval,Save);
		} 
		while (CurrentInterval->Left < Value->Left){
			ist_add_value_to_interval(CurrentInterval,1);
			NewNode = ist_create_node(CurrentInterval);
			ist_copy_sons(Node,NewNode);
			ist_insert_list_node(listnode,NewNode); 
			NewNode->AuxP = Node->AuxP;
			Node->AuxP = NewNode;
			if (lenghtofinterv != INFINITY){
				ist_assign_interval_to_interval(Save,CurrentInterval);
				for (i = 0; i< lenghtofinterv; ++i){
					ist_add_interval_to_interval(CurrentInterval,Rightincr);
					NewNode = ist_create_node(CurrentInterval);
					ist_copy_sons(Node,NewNode);
					ist_insert_list_node(listnode,NewNode); 
					NewNode->AuxP = Node->AuxP;
					Node->AuxP = NewNode;
				}
				ist_assign_interval_to_interval(CurrentInterval,Save);
			} 

		}
		Node = Node->Next;
	}
	NewNode = ist_remove_first_elem_list_node(listnode);
	while (NewNode != NULL){
		ist_add_node_star(Layer,NewNode);
		NewNode = ist_remove_first_elem_list_node(listnode);
	}
	ist_dispose_info(Rightincr);
	ist_dispose_info(CurrentInterval);
	ist_dispose_info(Save);
	xfree(listnode);
}

static void RemoveNodeWithoutValue(S, Target, Value)
	ISTSharingTree *S;
   	integer16 Target;
  	ISTInterval *Value;
{
	ISTLayer *Layer;
	ISTNode *Node;
	size_t i;
	Layer = S->FirstLayer;
	for (i = 0;i < Target; ++i)
		Layer = Layer->Next;
	Node = Layer->FirstNode;
	while (Node != NULL){
		if (ist_equal_interval(Node->Info,Value) == false)
			ist_remove_sons(Node);
		Node = Node->Next;
	}
	ist_remove_node_without_son(S);
	ist_remove_node_without_father(S);
}

static void AdjustSonNode(Node)
	ISTNode *Node;
{
	/* 
	 * This fonction is useful when we generate the over approxmiation for the
	 * target value. In fact during this generation (AddValueLesserThan) we add
	 * new nodes, we have also to take care about their parents.  The use of
	 * the List is important, otherwise we will add several time the same son
	 * to the same node.
	 */ 
	ISTSon *Son;
	ISTNode *NewSon;
	ISTHeadListNode *ListSon;
	ist_init_list_node(&ListSon);
	Son = Node->FirstSon;
	while (Son != NULL ){
		NewSon = Son->Son->AuxP;
		while (NewSon != NULL) {
			ist_insert_list_node(ListSon,NewSon);
			NewSon = NewSon->AuxP;
		}
		Son = Son->Next;
	}
	NewSon = ist_remove_first_elem_list_node(ListSon);
	while (NewSon != NULL) {
		ist_add_son(Node,NewSon);
		NewSon = ist_remove_first_elem_list_node(ListSon);
	}
}

static void AdjustSonLayer(Layer)
	ISTLayer *Layer;
{
	ISTNode* Node;
	Node = Layer->FirstNode;
	while (Node != NULL){
		AdjustSonNode(Node);
		Node = Node->Next;
	}
}

static void ComputeOverApproximationForValue(S, Value, PlacesInTransfert)
	ISTSharingTree *S;
   	ISTInterval *Value;
   	integer16 *PlacesInTransfert;
{
	size_t i, nbr_variables; 
	ISTInterval *Temp;
	ISTLayer *Layer; 
	nbr_variables = ist_nb_layers(S)-1;
	Temp = ist_build_interval(0,(Value->Right == INFINITY) ? INFINITY :0);
	Layer = S->FirstLayer;
	for (i=0; i < nbr_variables; ++i){
		if (PlacesInTransfert[i] == true){
			/*
			 * With 'Value' we can set the values in the target layer and
			 * originS.  Si 'Value' a sa rightbound = \infty, alors on peut
			 * mettre l'infini partout a droite seul la leftbound compte. En
			 * revange, si la rightbound est finie il faut initialiser les
			 * layers origin et target a [0,0].
			 */
			SetNodesOfLayerToValue(Layer,Temp);
			ComputeOverApproximationOnLayerForValue(Layer,Value);
			if (i==0)
				AdjustSonNode(S->Root);
			else
				AdjustSonLayer(Layer->Previous);
		}
		Layer = Layer->Next;
	}
	ist_normalize(S);
	ist_dispose_info(Temp);
}


static ISTNode *IntersectionWithFormulaTransfert(node, PlacesInTransfer, MaxSum,  height, NuLayer, LINK)
	ISTNode *node;
	integer16 *PlacesInTransfer;
	ISTInterval *MaxSum;
	integer16 height;
	long  NuLayer;
	struct LOC_ist_method  *LINK;
{
	ISTSon *s1;
	ISTNode *rnode, *rchild;
	boolean stop;
	long temp;


	if (ist_equal_interval(node->Info,&IST_end_of_list)) {
		if (ist_equal_interval(MaxSum,LINK->intersect)) {
			rnode = ist_create_node(&IST_end_of_list);
			rnode = ist_add_node(LINK->rlayer, rnode);
		} else
			rnode = NULL;
	} else {
		rnode = ist_create_node(node->Info);
		LINK->rlayer = LINK->rlayer->Next;
		if (LINK->rlayer == NULL)
			LINK->rlayer = ist_add_last_layer(LINK->STR);
		s1 = node->FirstSon;
		stop = false;
		while (s1 != NULL && !stop) {
			if (NuLayer < height && PlacesInTransfer[NuLayer] == true){
				ist_add_interval_to_interval(LINK->intersect,s1->Son->Info);
				if (LINK->intersect->Left > MaxSum->Left ||
						(LINK->intersect->Left == MaxSum->Left &&
						 ist_greater_value(LINK->intersect->Right,MaxSum->Right))){
					stop = true;
				} else {
					/* 
					 * We store the interval, in a univoque way inside a 32 bit
					 * field.  We take as asumption that -> Left and ->Right <
					 * 2^16.
					 */
					temp = (0x0000ffff & LINK->intersect->Left);
					temp = (LINK->intersect->Right == INFINITY ? 0x0000ffff :
							0x0000ffff & LINK->intersect->Right) | (temp << 16);
					LINK->memo = ist_get_memoization1(s1->Son, (ISTNode*)temp);
					/* 
					 * Here, we play with fire ... I explain. We take as
					 * asumption that our values won't over 2^16 - 2 which is a
					 * reasonable asumption.  In the case of infinity we encode
					 * 0xffff as value. We take also as asumption that long is
					 * over 32 bit 
					 */
					if (LINK->memo != NULL){
						rchild = LINK->memo->r;
					} else
						rchild = IntersectionWithFormulaTransfert(s1->Son,
								PlacesInTransfer,MaxSum, height, NuLayer + 1, LINK);
					if (rchild != NULL)
						ist_add_son(rnode, rchild);
				}
				ist_sub_interval_to_interval(LINK->intersect,s1->Son->Info);
			} else {	
				temp = (0x0000ffff & LINK->intersect->Left);
				temp =  (LINK->intersect->Right == INFINITY ?  0x0000ffff :
						0x0000ffff & LINK->intersect->Right) | (temp << 16);
				LINK->memo = ist_get_memoization1(s1->Son, (ISTNode*)temp);
				if (LINK->memo != NULL){ 
					rchild = LINK->memo->r;
				} else
					rchild = IntersectionWithFormulaTransfert(s1->Son,
							PlacesInTransfer,MaxSum, height, NuLayer + 1, LINK);
				if (rchild != NULL)
					ist_add_son(rnode, rchild);
			}
			s1 = s1->Next;
		}
		LINK->rlayer = LINK->rlayer->Previous;
		if (rnode->FirstSon != NULL)
			rnode = ist_add_node(LINK->rlayer, rnode);
		else {
			ist_dispose_node(rnode);
			rnode = NULL;
		}
	}
	temp = (0x0000ffff & LINK->intersect->Left);
	temp =  (LINK->intersect->Right == INFINITY ?
			0x0000ffff : 0x0000ffff & LINK->intersect->Right) | (temp << 16);
	ist_put_memoization1(node, (ISTNode*)temp, rnode);
	return rnode;
}


ISTSharingTree *ist_intersection_with_formula_transfer(ST1, PlacesInTransfer, Value, height)
	ISTSharingTree *ST1;
	integer16 *PlacesInTransfer;
	ISTInterval *Value;
	integer16 height;
{
	struct LOC_ist_method  V;
	ISTSon *s1;
	ISTNode *rchild;
	boolean stop;

	ist_new(&V.STR);
	V.intersect = ist_build_interval(0,0);
	ist_new_memo1_number();
	V.rlayer = ist_add_last_layer(V.STR);
	s1 = ST1->Root->FirstSon;
	stop = false;
	while (s1 != NULL && !stop) {
		if (PlacesInTransfer[0] == true){
			ist_add_interval_to_interval(V.intersect,s1->Son->Info);
			if (V.intersect->Left > Value->Left || (V.intersect->Left == Value->Left
					   	&& ist_greater_value(V.intersect->Right,Value->Right))) {
				stop = true;
			} else {
				rchild = IntersectionWithFormulaTransfert(s1->Son,PlacesInTransfer,
							Value, height, 1L,&V);
				if (rchild != NULL) {
					ist_add_son(V.STR->Root, rchild);
				}
			}
			if (!stop){
				ist_sub_interval_to_interval(V.intersect,s1->Son->Info);
			}
		} else {
			rchild = IntersectionWithFormulaTransfert(s1->Son,PlacesInTransfer,
					Value, height, 1L,&V);
			if (rchild != NULL) {
				ist_add_son(V.STR->Root, rchild);
			}
		}
		s1 = s1->Next;
	}
	ist_dispose_info(V.intersect);
	return V.STR;
}



ISTSharingTree *ist_pre_of_transfer(S, transition, nbr_variables)
	ISTSharingTree *S;
	transition_t *transition;
	integer16 nbr_variables;
{


	printf("ist_pre_of_rule_plus_transfer: On rentre\n");
	ist_checkup(S);
	ist_write(S);
	printf("========================================\n");
	

	ISTLayer* Layer;
	ISTInterval *CurrentValue;
	ISTNode *Node;
	ISTSharingTree *Sol, *STInt1, *STInt2, *STInt3;
	integer16 *PlacesInTransfert;
	size_t i,j,TargetLayer, CurrentTarget;
	boolean stop;
	Sol = ist_copy(S);
	
	printf("copie de S\n");
	ist_checkup(Sol);
	ist_write(Sol);
	
	PlacesInTransfert = (integer16 *)xmalloc(nbr_variables*sizeof(integer16));
	for (i=0; i < transition->nbr_transfers; ++i){
		for (j=0; j < nbr_variables;++j){	
			PlacesInTransfert[j] = transition->transfers[i].origin[j];
		}
		/* We set to true all the variables that are involved in the transfers
		 */
		PlacesInTransfert[transition->transfers[i].target] = true;
		ist_new(&STInt3);
		Layer = Sol->FirstLayer;
		TargetLayer = transition->transfers[i].target;
		for (CurrentTarget = 0; CurrentTarget < TargetLayer; ++CurrentTarget)
			Layer = Layer->Next;

		Node = Layer->FirstNode;
		while(Node != NULL){
			CurrentValue = Node->Info;
			STInt1 = ist_copy(Sol);
			/*
			 * We remove from the target layer, all the nodes that do not have
			 * their value equal to target (i.e. CurrentValue).
			 */
			
			printf("avant RemoveNodeWithoutValue\n");
			ist_checkup(STInt1);
			ist_write(STInt1);
			
			ist_checkup(Sol);
			ist_write(Sol);
			
			RemoveNodeWithoutValue(STInt1,TargetLayer,CurrentValue);

			printf("apres RemoveNodeWithoutValue\n");
			ist_checkup(STInt1);
			ist_write(STInt1);
			
			/*
			 * We build the overapproximation of values that do not satisfy the
			 * transfer's equation.  We do it for all the layers that are
			 * involved in the current transfer.
			 */
			ComputeOverApproximationForValue(STInt1,CurrentValue,PlacesInTransfert);
			/*
			 * Starting from that overapproximation, we intersect with the
			 * formula to keep only the models of the transfer formula.  So we
			 * have generated all the possible decomposition of the sum.
			 */

			printf("apres ComputeOverApproximationForValue\n");
			ist_checkup(STInt1);
			ist_write(STInt1);
			
			STInt2 = ist_intersection_with_formula_transfer(STInt1,PlacesInTransfert,CurrentValue, nbr_variables);
			ist_dispose(STInt1);
			STInt1 = ist_union(STInt3,STInt2);
			ist_dispose(STInt3);
			STInt3 = STInt1;
			ist_dispose(STInt2);
			/* 
			 * It's not relevant to do the same computation on
			 * the same value so we go to the next different value 
			 */
			stop = false;
			Node = Node->Next;
			while (!stop){
				if (Node == NULL)
					stop = true;
				else {
					if (ist_not_equal_interval(Node->Info,CurrentValue))
						stop = true;
					else 
						Node = Node->Next;
				}
			}
		}
		ist_dispose(Sol);
		Sol = STInt3;
	}
	xfree(PlacesInTransfert);
	return Sol;
}


ISTSharingTree *ist_pre_of_rule_plus_transfer(Prec, transition, nbr_variables)
	ISTSharingTree *Prec;
	transition_t *transition;
	integer16 nbr_variables;
{
	ISTSharingTree *STInt, *Temp;
	ISTNode *Node;
	ISTLayer *Layer;
	ISTInterval *intersect, *tau;
	size_t l;
	boolean modified = false;

	
	
	tau = ist_build_interval(0,INFINITY);
	STInt=ist_copy(Prec);
	if (transition->nbr_transfers > 0) 
		modified = KeepMarkingsSatisfyingPostCondition(STInt,transition); 
	if (ist_is_empty(STInt)== false){
		if (transition->nbr_transfers > 0) {
			if (modified == true ) {
				/* Because the KeepMarkingsSatisfyingPostCondition may violate
				 * the second condition */
				ist_adjust_second_condition(STInt);
			}
			Temp = ist_pre_of_transfer(STInt, transition, nbr_variables);
			ist_dispose(STInt);
			STInt = Temp;
		}
		if (ist_is_empty(STInt)== false ) {
			Layer=STInt->FirstLayer;
			l=0;
			while (Layer->Next != NULL ) {
				if (transition->cmd_for_place[l].delta  != 0 
						|| ist_not_equal_interval(&transition->cmd_for_place[l].guard, tau)){
					Node=Layer->FirstNode;
					while (Node!=NULL ) {
						ist_add_value_to_interval(Node->Info,-transition->cmd_for_place[l].delta);
						intersect = ist_intersect_intervals(Node->Info,&transition->cmd_for_place[l].guard);
						if (intersect == NULL)
							ist_remove_sons(Node);
						else {
							ist_dispose_info(Node->Info);
							Node->Info = intersect;
						}
						Node = Node->Next;

					}
				}
				Layer=Layer->Next;
				++l;
			}
			ist_remove_node_without_son(STInt);
			ist_remove_node_without_father(STInt);
		}
	}
	/* The result is not supposed to be in normal form */
	return STInt;
}
#else /* We deal with the transition tree (DEPRECATED) */
static ISTNode *PreOfRulesNode(node, nodetrans, LINK)
	ISTNode *node;
   	TransNode *nodetrans;
   	struct LOC_ist_method  *LINK;
{
	ISTSon *s1;
	TransSon *s2;
	ISTNode *rnode, *rchild;

	if (ist_equal_interval(node->Info,&IST_end_of_list))
		rnode = ist_add_node(LINK->rlayer, ist_create_node(&IST_end_of_list));
	else {
		rnode = ist_create_node(LINK->intersect);
		ist_dispose_info(LINK->intersect);
		LINK->rlayer = LINK->rlayer->Next;
		if (LINK->rlayer == NULL)
			LINK->rlayer = ist_add_last_layer(LINK->STR);
		s1 = node->FirstSon;
		while (s1 != NULL){
			s2 = nodetrans->FirstSon;
			while ( s2 != NULL) {
				ist_add_value_to_interval(s1->Son->Info,-s2->Son->Info.Delta);
				LINK->intersect = ist_intersect_intervals(s1->Son->Info,&s2->Son->Info.Guard);
				ist_add_value_to_interval(s1->Son->Info,s2->Son->Info.Delta);

				if (LINK->intersect != NULL) {
					LINK->memo = ist_get_memoization1(s1->Son,(ISTNode *) s2->Son);
					if (LINK->memo != NULL){
						rchild = LINK->memo->r;
						ist_dispose_info(LINK->intersect);
					} else
						rchild = PreOfRulesNode(s1->Son, s2->Son, LINK);
					if (rchild != NULL) 
						ist_add_son(rnode, rchild);
				}
				s2 = s2->Next;
			}
			s1 = s1->Next;
		}
		LINK->rlayer = LINK->rlayer->Previous;
		if (rnode->FirstSon != NULL)
			rnode = ist_add_node(LINK->rlayer, rnode);
		else {
			ist_dispose_node(rnode);
			rnode = NULL;
		}
	}
	ist_put_memoization1(node,(ISTNode *)nodetrans, rnode);
	return rnode;
}

/*
 * OUT OF ORDER : To update, we have to pass the system->tree_of_transitions
 */
ISTSharingTree *ist_pre_of_rules (prec)
	ISTSharingTree *prec;
{
	struct LOC_ist_method  V;
	ISTSharingTree *copy_prec;
	ISTSon *s1;
	TransSon *s2;
	ISTNode *rchild;
	boolean stop;

	ist_new(&V.STR);
	ist_new_memo1_number();
	copy_prec = ist_copy(prec);
	V.rlayer = ist_add_last_layer(V.STR);
	s1 = copy_prec->Root->FirstSon;
	while (s1 != NULL ){
		/* We should play with tree_of_transitions */
		s2 = IST_trans_tree->Root->FirstSon;
		while ( s2 != NULL) {
			ist_add_value_to_interval(s1->Son->Info,-s2->Son->Info.Delta);
			V.intersect = ist_intersect_intervals(s1->Son->Info,&s2->Son->Info.Guard);
			ist_add_value_to_interval(s1->Son->Info,s2->Son->Info.Delta);
			if (V.intersect != NULL) {
				rchild = PreOfRulesNode(s1->Son, s2->Son, &V);
				if (rchild != NULL) 
					ist_add_son(V.STR->Root, rchild);
			}
			s2 = s2->Next;
		}
		s1 = s1->Next;
	}

	stop = false;
	while (!stop) {
		if (V.STR->LastLayer == NULL) {
			stop = true;
		}else {
			if (V.STR->LastLayer->FirstNode != NULL)
				stop = true;
			else
				ist_delete_last_layer(V.STR);
		}
	}
	V.STR->NbElements = V.STR->Root->AuxI;
	return V.STR;
}
#endif


ISTSharingTree *ist_pre_cone(prec, reached_elem, system)
	ISTSharingTree *prec, *reached_elem;
	transition_system_t *system;
{
#ifdef TRANSFERT
	ISTSharingTree *pre_of_ith_rule, *temp;
	ISTSharingTree *pre_until_ith_rule;
	size_t i;
	ist_new(&pre_until_ith_rule);
	i = 0;
	while (i < system->limits.nbr_rules && (ist_is_empty(prec) == false)){
		pre_of_ith_rule = ist_pre_of_rule_plus_transfer(prec, &system->transition[i], system->limits.nbr_variables);
		if (ist_is_empty(pre_of_ith_rule) == false){ 
			ist_normalize(pre_of_ith_rule);
			ist_remove_with_invar_heuristic(pre_of_ith_rule, i, system);

			if (ist_is_empty(pre_of_ith_rule) == false){ 
				temp = ist_remove_subsumed_paths(pre_of_ith_rule,reached_elem);
				ist_dispose(pre_of_ith_rule);
				pre_of_ith_rule = temp;

				if ((ist_is_empty(pre_of_ith_rule) == false) && (ist_is_empty(pre_until_ith_rule) == false)){ 
					temp = ist_remove_subsumed_paths(pre_of_ith_rule,pre_until_ith_rule);
					ist_dispose(pre_of_ith_rule);
					pre_of_ith_rule = temp;

					if (ist_is_empty(pre_of_ith_rule) == false){ 
						temp = ist_remove_subsumed_paths(pre_until_ith_rule,pre_of_ith_rule);
						ist_dispose(pre_until_ith_rule);
						pre_until_ith_rule = temp;
					}
				}
				if (ist_is_empty(pre_of_ith_rule) == false){ 
					temp = ist_minimal_form(pre_of_ith_rule);
					ist_dispose(pre_of_ith_rule);
					pre_of_ith_rule = temp;
					temp = ist_union(pre_of_ith_rule,pre_until_ith_rule);
					ist_dispose(pre_until_ith_rule);
					pre_until_ith_rule=temp;

				}
			}

		}
		++i;
	}
	return pre_until_ith_rule;

#else
	ISTSharingTree *raw_pre, *copy_prec;
	copy_prec = ist_copy(prec);
	/* DEPRECATED, to make functionnal */
	raw_pre = ist_pre_of_rules(copy_prec);
	if (ist_is_empty(raw_pre) == false) {
		ist_normalize(raw_pre);
		ist_remove_with_all_invar_heuristic(raw_pre);
		/*
		 * pruning with the old frontier,the new frontier 
		 * and the set of the elements reached at the preceding iterations
		 */
		if (ist_is_empty(raw_pre) == false) {
			/*remove from the frontier markings subsumed by the new frontier*/
			temp = ist_remove_subsumed_paths(copy_prec, raw_pre);
			ist_dispose(copy_prec);
			copy_prec = temp;
			if ((ist_is_empty(reached_elem) == false) & (ist_is_empty(raw_pre) == false)) {
				temp = ist_remove_subsumed_paths(raw_pre, reached_elem);
				ist_dispose(raw_pre);
				raw_pre = temp;
			}
		}
		if (ist_is_empty(raw_pre) == false){
			temp = ist_minimal_form(raw_pre);
			ist_dispose(raw_pre);
			raw_pre = temp;
		}
	}
	return raw_pre;
#endif 
}

ISTSharingTree *ist_post(forward_p, system)
	ISTSharingTree *forward_p;
	transition_system_t *system;
{
	ISTSharingTree *res;
	ISTSon **path;
	ISTInterval **tuple;
	ISTInterval *intersect;
	ISTInterval tokens_transfered;
	size_t i, j, k, l;
	boolean stop;

	ist_new(&res);
	/* Allocation of memory */
	path = (ISTSon **)xmalloc((system->limits.nbr_variables)*sizeof(ISTSon *));
	tuple = (ISTInterval **)xmalloc((system->limits.nbr_variables)*sizeof(ISTInterval *));
	for (i = 0; i < system->limits.nbr_variables; ++i){
		tuple[i] = ist_new_info();
	}
	/* 
	 * Now, we will browse all the elems of forward_p
	 * for each compute the post for all the rules and add
	 * them into res. The browsing is derecursified.
	 */

	/* We initialize path[0] with the first elem */
	path[0]= forward_p->Root->FirstSon;
	i = 0;
	while (path[0] != NULL){
		if (path[i] == NULL){
			--i;
			path[i] = path[i]->Next;
		} else {
			++i;
			if (i < system->limits.nbr_variables){
				/* We take care of not going out of the vector */
				path[i] = path[i-1]->Son->FirstSon;
			}
		}

		if (i == system->limits.nbr_variables){
			/* We have a new tuple, we apply the post on it */
			for (j = 0; j < system->limits.nbr_rules; ++j){
				for (k = 0; k < system->limits.nbr_variables; ++k)
					ist_assign_interval_to_interval(tuple[k],path[k]->Son->Info);
				/*
				 * We will compute effect of each transition and their
				 * transfers associated
				 */
				k = 0;
				stop = false;
				while (k < system->limits.nbr_variables && !stop){
					intersect = ist_intersect_intervals(&system->transition[j].cmd_for_place[k].guard, tuple[k]);
					if (intersect != NULL){
						ist_assign_interval_to_interval(tuple[k],intersect);
						ist_dispose_info(intersect);
					} else {
						stop = true;
					}
					++k;
				}
				if (!stop){
					/* If we can apply the rule on the tuple */
					for (k = 0; k < system->limits.nbr_variables; ++k) 
						/* then apply it  */
						ist_add_value_to_interval(tuple[k],system->transition[j].cmd_for_place[k].delta);

					/* Apply transfers associated to this rule */
					for (k = 0; k < system->transition[j].nbr_transfers; ++k){
						ist_assign_values_to_interval(&tokens_transfered,0,0);
						for (l = 0; l < system->limits.nbr_variables; ++l){
							if (system->transition[j].transfers[k].origin[l]  == true){
								ist_add_interval_to_interval(&tokens_transfered,tuple[l]);
								/* Post condition of the trasnfer */
								ist_assign_values_to_interval(tuple[l],0,0);
							}
						}
						ist_add_interval_to_interval(tuple[system->transition[j].transfers[k].target], &tokens_transfered);
					}
					/* We add this tuple in the tree containing the post */
					ist_add(res,tuple,system->limits.nbr_variables);
					/* We reload PATH in tuple to apply a new rule on this tuple */
					for (l = 0;l < system->limits.nbr_variables; ++l){
						ist_assign_interval_to_interval(tuple[l],path[l]->Son->Info);
					}
				}
			}
			/* We continue to browse all the tuple ... */
			--i;
			path[i] = path[i]->Next;
		}
	}

	/* Clean desallocation */
	for (i = 0; i < system->limits.nbr_variables; ++i)
		ist_dispose_info(tuple[i]);
	xfree(tuple);
	xfree(path);

	return res;
}

ISTSharingTree *ist_post_transition(forward_p, system, transition)
	ISTSharingTree *forward_p;
	transition_system_t *system;
	int transition;
{
	ISTSharingTree *res;
	ISTSon **path;
	ISTInterval **tuple;
	ISTInterval *intersect;
	ISTInterval tokens_transfered;
	size_t i, k, l;
	boolean stop;

	ist_new(&res);
	/* Allocation of memory */
	path = (ISTSon **)xmalloc((system->limits.nbr_variables)*sizeof(ISTSon *));
	tuple = (ISTInterval **)xmalloc((system->limits.nbr_variables)*sizeof(ISTInterval *));
	for (i = 0; i < system->limits.nbr_variables; ++i){
		tuple[i] = ist_new_info();
	}
	/* 
	 * Now, we will enumerate all the elems of forward_p
	 * for each compute the post for all the rules and add
	 * them into res
	 */

	/* We initialize path[0] with the first elem */
	path[0]= forward_p->Root->FirstSon;
	i = 0;
	while (path[0] != NULL){
		if (path[i] == NULL){
			--i;
			path[i] = path[i]->Next;
		} else {
			++i;
			if (i < system->limits.nbr_variables){
				/* We take care of not going out of the vector */
				path[i] = path[i-1]->Son->FirstSon;
			}
		}

		if ( i == system->limits.nbr_variables){
			/* We have a new tuple, we apply the post on it */
			
			for (k = 0; k < system->limits.nbr_variables; ++k){
			   ist_assign_interval_to_interval(tuple[k],path[k]->Son->Info);
			}
			/*
			 * We will compute effect of each transition and their transfers
			 * associated
			 */
			k = 0;
			stop = false;
			while (k < system->limits.nbr_variables&& !stop){
			  intersect = ist_intersect_intervals(&system->transition[transition].cmd_for_place[k].guard, tuple[k]);
			  if (intersect != NULL){
			    ist_assign_interval_to_interval(tuple[k],intersect);
			    ist_dispose_info(intersect);
			  } else {
			    stop = true;
			  }
			    ++k;
			}
			/* Check if we can apply the rule on the tuple */
			if (!stop){
			  /* It means that we can apply the rule to this tuple */
			  for (k = 0; k < system->limits.nbr_variables; ++k) {
			    /* Apply the rule  */
			    ist_add_value_to_interval(tuple[k],system->transition[transition].cmd_for_place[k].delta);
			  }

			  /* Apply transfers associated to this rule */
			  for (k = 0; k < system->transition[transition].nbr_transfers; ++k){
			    ist_assign_values_to_interval(&tokens_transfered,0,0);
			    for (l = 0; l < system->limits.nbr_variables; ++l){
			      if (system->transition[transition].transfers[k].origin[l]  == true){
				ist_add_interval_to_interval(&tokens_transfered,tuple[l]);
				ist_assign_values_to_interval(tuple[l],0,0);
			      }
			    }
			    ist_add_interval_to_interval(tuple[system->transition[transition].transfers[k].target], &tokens_transfered);
			  }
			  /* We add this tuple in the tree containing the post */
			  ist_add(res,tuple,system->limits.nbr_variables);
			  /* We reload PATH in tuple to apply a new rule on this tuple */
			  for (l = 0;l < system->limits.nbr_variables; ++l){
			    ist_assign_interval_to_interval(tuple[l],path[l]->Son->Info);
			  }
			}
			/* We continue to browse all the tuple ... */
			--i;
			path[i] = path[i]->Next;
		}
	}

	for (i = 0; i < system->limits.nbr_variables; ++i){
		ist_dispose_info(tuple[i]);
	}
	xfree(tuple);
	xfree(path);

	return res;
}
