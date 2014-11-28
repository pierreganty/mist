// vim:sw=4:ts=4:cindent
/*
   This file is part of mist.

   mist is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   mist is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with mist; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   Copyright 2003, Pierre Ganty, 2006 Laurent Van Begin
 */

#include "predtrans.h"
#include "basis.h"
#include "minimizeinvarheuristic.h"
#include "minimize.h"
#include "listnode.h"
#include "remove.h"
#include "normalize.h"
#include "xmalloc.h"
#include "checkup.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>




static ISTNode *PostOfTransfer(ISTNode *node,ISTSharingTree *STR, ISTLayer * rlayer, ISTInterval val,int no_layer,integer16 *origin,integer16 target)
{
	ISTSon *s;
	ISTNode *rchild;
	ISTNode *rnode;
	TMemo1 *memo;
	ISTInterval inter;
	long temp;

	if (ist_equal_interval(node->Info,&IST_end_of_list))
		rnode = ist_add_node(rlayer, ist_create_node(&IST_end_of_list));
	else {
		/*
		 *if the layer is origin of the transfer
		 */
		if (origin[no_layer] == 1) {
			ist_add_interval_to_interval(&val,node->Info);
			ist_assign_values_to_interval(&inter,0,0);
			rnode = ist_create_node(&inter);
		} else
			rnode = ist_create_node(node->Info);

		rlayer = rlayer->Next;
		if (rlayer == NULL)
			rlayer = ist_add_last_layer(STR);

		for(s=node->FirstSon;s != NULL;s=s->Next){
			/*
			 * We store the interval, in a univoque way inside a 32 bit field.
			 * We take as asumption that -> Left and ->Right < 2^16.
			 */
			temp = (0x0000ffff & val.Left);
			temp =  (val.Right == INFINITY ?
					0x0000ffff : 0x0000ffff & val.Right) | (temp << 16);
			memo = ist_get_memoization1(s->Son, (ISTNode*)temp);

			if (memo != NULL)
				ist_add_son(rnode,memo->r);
			else {
				rchild = PostOfTransfer(s->Son,STR,rlayer,val,no_layer+1, origin, target);
				ist_add_son(rnode,rchild);
			}
		}
		rlayer = rlayer->Previous;
		if (no_layer == target)
			ist_add_interval_to_interval(rnode->Info,&val);
		rnode = ist_add_node(rlayer,rnode);
	}
	temp = (0x0000ffff & val.Left);
	temp =  (val.Right == INFINITY ?
			0x0000ffff : 0x0000ffff & val.Right) | (temp << 16);
	ist_put_memoization1(node, (ISTNode *)temp, rnode);
	return rnode;
}

ISTSharingTree *ist_post_of_transfer(S, transfers)
	ISTSharingTree *S;
	transfers_t *transfers;
{
	ISTSharingTree *STInt1;
	ISTLayer * rlayer;
	ISTSon * s;
	ISTInterval inter;

	ist_new(&STInt1);
	if (ist_is_empty(S) == false) {
		ist_new_memo1_number();
		rlayer = ist_add_last_layer(STInt1);
		ist_assign_values_to_interval(&inter,0,0);
		for(s = S->Root->FirstSon; s != NULL;s = s->Next)
			ist_add_son(STInt1->Root,PostOfTransfer(s->Son,STInt1,rlayer,inter,0,transfers->origin,transfers->target));
		ist_normalize(STInt1);
	}
	return STInt1;
}


/*
 * All that part (fron #ifdef TRANSFERT ... #else) is only dedicated to system
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
	ist_remove_node_without_father(S);
	ist_remove_node_without_son(S);
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
	ist_remove_node_without_father(S);
	ist_remove_node_without_son(S);
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

static void ComputeOverApproximationForValue(S, Value, Trans)
	ISTSharingTree *S;
   	ISTInterval *Value;
   	transfers_t *Trans;
{
	size_t i, nbr_variables;
	ISTInterval *Temp;
	ISTLayer *Layer;
	nbr_variables = ist_nb_layers(S)-1;
	Temp = ist_build_interval(0,(Value->Right == INFINITY) ? INFINITY :0);
	Layer = S->FirstLayer;
	for (i=0; i < nbr_variables; ++i){
		if (Trans->origin[i] == 1 || i == Trans->target){
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


static ISTNode *IntersectionWithFormulaTransfert(node, Trans, MaxSum,  height, NuLayer, LINK)
	ISTNode *node;
	transfers_t *Trans;
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
			if (NuLayer < height && (Trans->origin[NuLayer] == 1 || Trans->target == NuLayer)){
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
						rchild = IntersectionWithFormulaTransfert(s1->Son,Trans,MaxSum, height, NuLayer + 1, LINK);
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
					rchild = IntersectionWithFormulaTransfert(s1->Son, Trans,MaxSum, height, NuLayer + 1, LINK);
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


/* Assume ist_nb_layers(S)-1 == system->limits.nbr_variables */
ISTSharingTree *ist_intersection_with_formula_transfer(ST1, Trans, Value)
	ISTSharingTree *ST1;
	transfers_t *Trans;
	ISTInterval *Value;
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
		if (Trans->origin[0] == 1 || Trans->target == 0){
			ist_add_interval_to_interval(V.intersect,s1->Son->Info);
			if (V.intersect->Left > Value->Left || (V.intersect->Left == Value->Left\
						&& ist_greater_value(V.intersect->Right,Value->Right)))
				stop = true;
			else {
				rchild = IntersectionWithFormulaTransfert(s1->Son,Trans, Value, ist_nb_layers(ST1)-1, 1L,&V);
				if (rchild != NULL)
					ist_add_son(V.STR->Root, rchild);
			}
			if (!stop)
				ist_sub_interval_to_interval(V.intersect,s1->Son->Info);
		} else {
			rchild = IntersectionWithFormulaTransfert(s1->Son,Trans, Value, ist_nb_layers(ST1)-1, 1L,&V);
			if (rchild != NULL)
				ist_add_son(V.STR->Root, rchild);
		}
		s1 = s1->Next;
	}
	ist_dispose_info(V.intersect);
	return V.STR;
}



ISTSharingTree *ist_pre_of_all_transfer(S, transition)
	ISTSharingTree *S;
	transition_t *transition;
{
	ISTLayer* Layer;
	ISTInterval *CurrentValue;
	ISTNode *Node;
	ISTSharingTree *Sol, *STInt1, *STInt2, *STInt3;
	size_t i, TargetLayer, CurrentTarget;
	boolean stop;
	Sol = ist_copy(S);

	for (i=0; i < transition->nbr_transfers; ++i){
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
			RemoveNodeWithoutValue(STInt1,TargetLayer,CurrentValue);
			/*
			 * We build the overapproximation of values that do not satisfy the
			 * transfer's equation.  We do it for all the layers that are
			 * involved in the current transfer.
			 */
			ComputeOverApproximationForValue(STInt1,CurrentValue,&transition->transfers[i]);
			/*
			 * Starting from that overapproximation, we intersect with the
			 * formula to keep only the models of the transfer formula.  So we
			 * have generated all the possible decomposition of the sum.
			 */
			STInt2 = ist_intersection_with_formula_transfer(STInt1,&transition->transfers[i],CurrentValue);
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
	return Sol;
}

ISTSharingTree * ist_pre_of_all_transfer_for_concretisation(ISTSharingTree * S,transition_t * transition)
{
    ISTLayer* Layer;
    ISTInterval *CurrentValue;
    ISTNode *Node;
    ISTSharingTree *Sol, *STInt1, *STInt2, *STInt3;
    size_t i,j,TargetLayer, CurrentTarget;
    boolean stop;
    Sol = ist_copy(S);

    for (i=0; i < transition->nbr_transfers; ++i){
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
            RemoveNodeWithoutValue(STInt1,TargetLayer,CurrentValue);
            /*
             * We build the overapproximation of values that do not satisfy the
             * transfer's equation.  We do it for all the layers that are
             * involved in the current transfer.
             */
	    ComputeOverApproximationForValue(STInt1,CurrentValue,&transition->transfers[i]);
            /*
             * Starting from that overapproximation, we intersect with the
             * formula to keep only the models of the transfer formula.  So we
             * have generated all the possible decomposition of the sum.
             */
            STInt2 = ist_intersection_with_formula_transfer(STInt1,&transition->transfers[i],CurrentValue);
            ist_dispose(STInt1);
            /* we call ist_intersection_with_formula_transfer which
             * considers the source places only in order the dismiss cases where tokens did
             * not move from the target of tokens
             */
            for(j=0;transition->transfers[i].origin[j]==0;++j);
            transition->transfers[i].target=j;
            STInt1 = ist_intersection_with_formula_transfer(STInt2,&transition->transfers[i],CurrentValue);
            transition->transfers[i].target=TargetLayer; // restore the target layer as it was
            ist_dispose(STInt2);
            STInt2 = STInt1;

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
    return Sol;
}


ISTSharingTree *ist_symbolic_pre_of_rule(Prec, transition)
   ISTSharingTree *Prec;
   transition_t *transition;
{
	ISTSharingTree *STInt;
	ISTNode *Node;
	ISTLayer *Layer;
	ISTInterval *inter, *tau;
	size_t l;
	boolean stop;

	tau = ist_build_interval(0,INFINITY);
	STInt=ist_copy(Prec);
	if (ist_is_empty(STInt)== false) {
		Layer=STInt->FirstLayer;
		l=0;
		while (Layer->Next != NULL) {
			if (transition->cmd_for_place[l].delta  != 0){
				Node=Layer->FirstNode;
				while (Node!=NULL ) {
					ist_sub_value_to_interval(Node->Info,transition->cmd_for_place[l].delta);
					Node = Node->Next;

				}
			}
			Node=Layer->FirstNode;
			while(Node !=NULL) {
				inter = ist_intersect_intervals(&transition->cmd_for_place[l].guard,Node->Info);
				if (inter == NULL)
					ist_remove_sons(Node);
				else {
					ist_dispose_info(Node->Info);
					Node->Info = inter;
				}
				Node = Node->Next;
			}
			Layer=Layer->Next;
			++l;
		}
		ist_remove_node_without_father(STInt);
		ist_remove_node_without_son(STInt);
	}
	/* Remove empty layers if any */
	stop = false;
	while (!stop) {
		if (STInt->LastLayer == NULL)
			stop = true;
		else {
			if (STInt->LastLayer->FirstNode != NULL)
				stop = true;
			else
				ist_remove_last_layer(STInt);
		}
	}
	ist_dispose_info(tau);
	if (!ist_is_empty(STInt))
		ist_normalize(STInt);
	return STInt;
}


/* The result is not supposed to be in normal form */
ISTSharingTree *ist_pre_of_rule_plus_transfer(Prec, transition)
	ISTSharingTree *Prec;
	transition_t *transition;
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
		modified=KeepMarkingsSatisfyingPostCondition(STInt,transition);
	if (ist_is_empty(STInt)== false){
		if (transition->nbr_transfers > 0) {
			if (modified == true )
				/* Because the KeepMarkingsSatisfyingPostCondition may violate
				 * the second condition */
				ist_adjust_second_condition(STInt);
			Temp = ist_pre_of_all_transfer(STInt, transition);
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
			ist_remove_node_without_father(STInt);
			ist_remove_node_without_son(STInt);
		}
	}
	/* The result is not supposed to be in normal form */
	return STInt;
}

static ISTNode *PreOfRulesNode(node, nodetrans, LINK)
	ISTNode *node;
   	ISTNode *nodetrans;
   	struct LOC_ist_method  *LINK;
{
	ISTSon *s1, *s2;
	ISTNode *rnode, *rchild;
	integer32 delta;

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
				/* Set delta */
				delta=s2->Son->Info->Right;
				/* Modify node */
				ist_sub_value_to_interval(s1->Son->Info,delta);
				/* Set guard */
				s2->Son->Info->Right=INFINITY;
				LINK->intersect = ist_intersect_intervals(s1->Son->Info,s2->Son->Info);
				/* Restore node */
				ist_add_value_to_interval(s1->Son->Info,delta);
				/* Restore delta */
				s2->Son->Info->Right=delta;

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
 * Where the first argument is interpreted as follows, each path
 * gives a transition where for each place the left bound gives
 * the guard value and the right bound gives the delta value
 */
ISTSharingTree *ist_pre_of_rules(IST_trans_tree, prec)
	ISTSharingTree *IST_trans_tree;
	ISTSharingTree *prec;
{
	struct LOC_ist_method  V;
	ISTSharingTree *copy_prec;
	ISTSon *s1, *s2;
	ISTNode *rchild;
	boolean stop;
	integer32 delta;

	ist_new(&V.STR);
	ist_new_memo1_number();
	copy_prec = ist_copy(prec);
	V.rlayer = ist_add_last_layer(V.STR);
	s1 = copy_prec->Root->FirstSon;
	while (s1 != NULL ){
		/* We should play with tree_of_transitions */
		s2 = IST_trans_tree->Root->FirstSon;
		while ( s2 != NULL) {
			/* Set delta */
			delta=s2->Son->Info->Right;
			/* Modify node */
			ist_sub_value_to_interval(s1->Son->Info,delta);
			/* Set guard */
			s2->Son->Info->Right=INFINITY;
			V.intersect = ist_intersect_intervals(s1->Son->Info,s2->Son->Info);
			/* Restore node */
			ist_add_value_to_interval(s1->Son->Info,delta);
			/* Restore delta */
			s2->Son->Info->Right=delta;
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
				ist_remove_last_layer(V.STR);
		}
	}
	V.STR->NbElements = V.STR->Root->AuxI;
	return V.STR;
}


static ISTNode *PostOfRulesNode(node, nodetrans, LINK)
	ISTNode *node;
   	ISTNode *nodetrans;
   	struct LOC_ist_method  *LINK;
{
	ISTSon *s1, *s2;
	ISTNode *rnode, *rchild;
	integer32 delta;

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
				/* Set delta */
				delta=s2->Son->Info->Right;
				/* Set guard */
				s2->Son->Info->Right=INFINITY;
				LINK->intersect = ist_intersect_intervals(s1->Son->Info,s2->Son->Info);
				/* Restore node */
				ist_add_value_to_interval(LINK->intersect,delta);
				/* Restore delta */
				s2->Son->Info->Right=delta;

				if (LINK->intersect != NULL) {
					LINK->memo = ist_get_memoization1(s1->Son,(ISTNode *) s2->Son);
					if (LINK->memo != NULL){
						rchild = LINK->memo->r;
						ist_dispose_info(LINK->intersect);
					} else
						rchild = PostOfRulesNode(s1->Son, s2->Son, LINK);
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
 * Where the first argument is interpreted as follows, each path
 * gives a transition where for each place the left bound gives
 * the guard value and the right bound gives the delta value
 */
ISTSharingTree *ist_post_of_rules(IST_trans_tree, succ)
	ISTSharingTree *IST_trans_tree;
	ISTSharingTree *succ;
{
	struct LOC_ist_method  V;
	ISTSharingTree *copy_succ;
	ISTSon *s1, *s2;
	ISTNode *rchild;
	boolean stop;
	integer32 delta;

	ist_new(&V.STR);
	ist_new_memo1_number();
	copy_succ = ist_copy(succ);
	V.rlayer = ist_add_last_layer(V.STR);
	s1 = copy_succ->Root->FirstSon;
	while (s1 != NULL ){
		/* We should play with tree_of_transitions */
		s2 = IST_trans_tree->Root->FirstSon;
		while ( s2 != NULL) {
			/* Set delta */
			delta=s2->Son->Info->Right;
			/* Set guard */
			s2->Son->Info->Right=INFINITY;
			V.intersect = ist_intersect_intervals(s1->Son->Info,s2->Son->Info);
			/* Restore node */
			ist_add_value_to_interval(V.intersect,delta);
			/* Restore delta */
			s2->Son->Info->Right=delta;
			if (V.intersect != NULL) {
				/* We compute dcl o post !!!! */
				V.intersect->Left=0;
				rchild = PostOfRulesNode(s1->Son, s2->Son, &V);
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
				ist_remove_last_layer(V.STR);
		}
	}
	V.STR->NbElements = V.STR->Root->AuxI;
	if (ist_is_empty(V.STR)==false)
		ist_normalize(V.STR);
	return V.STR;
}

ISTSharingTree *ist_pre_pruned_wth_inv_and_prev_iterates(prec, reached_elem, system)
	ISTSharingTree *prec, *reached_elem;
	transition_system_t *system;
{
	ISTSharingTree *pre_of_ith_rule, *temp;
	ISTSharingTree *pre_until_ith_rule;
	size_t i;
	ist_new(&pre_until_ith_rule);
	i = 0;
	while (i < system->limits.nbr_rules && (ist_is_empty(prec) == false)){
		pre_of_ith_rule = ist_pre_of_rule_plus_transfer(prec, &system->transition[i]);
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
		ist_dispose(pre_of_ith_rule);
		++i;
	}
	return pre_until_ith_rule;
}

ISTSharingTree *ist_pre(prec, system)
	ISTSharingTree *prec;
	transition_system_t *system;
{
	ISTSharingTree *pre_of_ith_rule, *temp;
	ISTSharingTree *pre_until_ith_rule;
	size_t i=0;
	ist_new(&pre_until_ith_rule);
	while (i < system->limits.nbr_rules && (ist_is_empty(prec) == false)){
		pre_of_ith_rule = ist_pre_of_rule_plus_transfer(prec, &system->transition[i]);
		if (ist_is_empty(pre_of_ith_rule) == false){
			ist_normalize(pre_of_ith_rule);

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
		ist_dispose(pre_of_ith_rule);
		++i;
	}
	return pre_until_ith_rule;
}

ISTSharingTree *ist_enumerative_pre_transition(ISTSharingTree *backward_p, transition_system_t *system, size_t transition)
{
	ISTSharingTree *res;
	ISTSon **path;
	ISTInterval **tuple;
	ISTInterval *intersect;
	size_t i, k, l;
	boolean stop;

	assert(ist_nb_layers(backward_p)-1==system->limits.nbr_variables);

	ist_new(&res);
	/* Allocation of memory */
	path = (ISTSon **)xmalloc((system->limits.nbr_variables)*sizeof(ISTSon *));
	tuple = (ISTInterval **)xmalloc((system->limits.nbr_variables)*sizeof(ISTInterval *));
	for (i = 0; i < system->limits.nbr_variables; ++i)
		tuple[i] = ist_new_info();
	/*
	 * Now, we will enumerate all the elems of backward_p
	 * for each compute the post for all the rules and add
	 * them into res
	 */

	/* We initialize path[0] with the first elem */
	path[0]= backward_p->Root->FirstSon;
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
			/* We will compute the reversed effect of 'transition'
 			 * (ommitting the transfer) */
			for (k = 0; k < system->limits.nbr_variables; ++k)
				ist_sub_value_to_interval(tuple[k],system->transition[transition].cmd_for_place[k].delta);
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
			if (!stop){
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
/* No transfer */
ISTSharingTree *ist_enumerative_pre(backward_p, system)
	ISTSharingTree *backward_p;
	transition_system_t *system;
{
	ISTSharingTree *res;
	ISTSon **path;
	ISTInterval **tuple;
	ISTInterval *intersect;
	size_t i, j, k, l;
	boolean stop;

	assert(ist_nb_layers(backward_p)-1==system->limits.nbr_variables);

	ist_new(&res);
	/* Allocation of memory */
	path = (ISTSon **)xmalloc((system->limits.nbr_variables)*sizeof(ISTSon *));
	tuple = (ISTInterval **)xmalloc((system->limits.nbr_variables)*sizeof(ISTInterval *));
	for (i = 0; i < system->limits.nbr_variables; ++i)
		tuple[i] = ist_new_info();
	/*
	 * Now, we will browse all the elems of backward_p
	 * for each compute the post for all the rules and add
	 * them into res. The browsing is derecursified.
	 */

	/* We initialize path[0] with the first elem */
	path[0]= backward_p->Root->FirstSon;
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
			/* We have a new tuple */
			for (j = 0; j < system->limits.nbr_rules; ++j){
				for (k = 0; k < system->limits.nbr_variables; ++k)
					ist_assign_interval_to_interval(tuple[k],path[k]->Son->Info);
				/*
				 * We will compute the reversed effect of each transition (ommitting the transfer)
				 */
				for (k = 0; k < system->limits.nbr_variables; ++k)
					ist_sub_value_to_interval(tuple[k],system->transition[j].cmd_for_place[k].delta);
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

ISTSharingTree *ist_enumerative_post(forward_p, system)
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

	assert(ist_nb_layers(forward_p)-1==system->limits.nbr_variables);

	ist_new(&res);
	/* Allocation of memory */
	path = (ISTSon **)xmalloc((system->limits.nbr_variables)*sizeof(ISTSon *));
	tuple = (ISTInterval **)xmalloc((system->limits.nbr_variables)*sizeof(ISTInterval *));
	for (i = 0; i < system->limits.nbr_variables; ++i)
		tuple[i] = ist_new_info();
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
							if (system->transition[j].transfers[k].origin[l]  == 1){
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

ISTSharingTree *ist_enumerative_post_transition(forward_p, system, transition)
	ISTSharingTree *forward_p;
	transition_system_t *system;
	size_t transition;
{
	ISTSharingTree *res;
	ISTSon **path;
	ISTInterval **tuple;
	ISTInterval *intersect;
	ISTInterval tokens_transfered;
	size_t i, k, l;
	boolean stop;

	assert(ist_nb_layers(forward_p)-1==system->limits.nbr_variables);

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
			      if (system->transition[transition].transfers[k].origin[l]  == 1){
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

ISTSharingTree *ist_symbolic_post_of_rules(ISTSharingTree *S, transition_t *t)
{
	ISTSharingTree * result;
	ISTLayer * L;
	ISTNode * N;
	size_t i, nbV;
	nbV=ist_nb_layers(S)-1;
	ISTInterval **g = (ISTInterval **)xmalloc(nbV*sizeof(ISTInterval *));
	ISTSharingTree *G;

	for(i = 0;i < nbV;i++)
		g[i] = ist_copy_interval(&t->cmd_for_place[i].guard);
	ist_new(&G);
	ist_add(G,g,nbV);
	for(i= 0;i< nbV;i++)
		ist_dispose_info(g[i]);
	xfree(g);

	result = ist_intersection(S,G);
	ist_dispose(G);
	/* If the IST is not empty, we apply the effect of the function */
	if (ist_is_empty(result) == false) {
		for (i=0, L = result->FirstLayer; i < nbV; i++, L = L->Next) {
			for(N = L->FirstNode;N != NULL;N=N->Next)
					ist_add_value_to_interval(N->Info,t->cmd_for_place[i].delta);
		}
	}
	return result;
}

ISTSharingTree *ist_symbolic_post(ISTSharingTree * S, transition_system_t *t) {

	size_t i;
	ISTSharingTree *result, *tmp, *tmp2;

	assert(ist_nb_layers(S)-1==t->limits.nbr_variables);
	ist_new(&result);
	for(i=0;i< t->limits.nbr_rules;i++) {
		tmp = ist_symbolic_post_of_rules(S,&t->transition[i]);
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


