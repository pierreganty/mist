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

   Copyright 2003, Pierre Ganty. Copyright 2006, Laurent Van Begin 
 */

#include "minimize.h"
#include <stdlib.h>
#include "father_mangmt.h"
#include "computesim.h"
#include "normalize.h"
#include "basis.h" 
#include "determinize.h"
#include "complement.h"
//----- For Debbugging ----------
#include "assert.h"
#include <stdio.h>
#include "ist_to_ps.h"
//----- For Debbugging ----------

static ISTNode *who_subsumes(node1, node2, LINK)
	ISTNode *node1, *node2;
	struct LOC_ist_method  *LINK;
{
	ISTSon *s1, *s2;
	ISTNode *rnode, *rchild;

	if (ist_equal_interval(node2->Info,&IST_end_of_list))
		rnode = ist_add_node(LINK->rlayer, ist_create_node(&IST_end_of_list));
	else {
		rnode = ist_create_node(node2->Info);
		LINK->rlayer = LINK->rlayer->Next;
		if (LINK->rlayer == NULL)
			LINK->rlayer = ist_add_last_layer(LINK->STR);
		s1 = node1->FirstSon;
		while (s1 != NULL) {
			s2 = node2->FirstSon;
			while (s2 != NULL) {
				if (ist_include_interval(s2->Son->Info,s1->Son->Info)) {
					/* s1 \subset s2 */
					LINK->memo = ist_get_memoization1(s1->Son, s2->Son);
					if (LINK->memo != NULL)
						rchild = LINK->memo->r;
					else
						rchild = who_subsumes(s1->Son, s2->Son, LINK);
					if (rchild != NULL)
						ist_add_son(rnode, rchild);
					s2 = s2->Next;
				} else {
					if (ist_less_interval(s2->Son->Info,s1->Son->Info))
						s2 = s2->Next;
					else
						s2 = NULL;
				}
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
	ist_put_memoization1(node1, node2, rnode);
	return rnode;
}

/* ist_compute_who_subsumes do the same as ist_compute_subsumed_paths but they return different
 * tree: ist_compute_who_subsumes returns the part of ST2 that subsumes some paths of ST1,
 * ist_compute_subsumed_paths returns the part of ST1 that is subsumed by ST2.
 */

ISTSharingTree *ist_compute_who_subsumes(ST1, ST2)
	ISTSharingTree *ST1, *ST2;
{
	struct LOC_ist_method  V;
	ISTSon *s1, *s2;
	ISTNode *rchild;
	boolean stop;
	ISTSharingTree *WITH;

	ist_new(&V.STR);
	ist_new_memo1_number();
	V.rlayer = ist_add_last_layer(V.STR);
	s1 = ST1->Root->FirstSon;
	while (s1 != NULL) {
		s2 = ST2->Root->FirstSon;
		while (s2 != NULL) {
			if (ist_include_interval(s2->Son->Info,s1->Son->Info)) {
				/* s1 \subset s2 */
				rchild = who_subsumes(s1->Son, s2->Son, &V);
				if (rchild != NULL)
					ist_add_son(V.STR->Root, rchild);
				s2 = s2->Next;
			} else {
				if (ist_less_interval(s2->Son->Info,s1->Son->Info))
					s2 = s2->Next;
				else
					s2 = NULL;
			}
		}
		s1 = s1->Next;
	}
	WITH = V.STR;
	stop = false;
	while (!stop) {
		if (WITH->LastLayer == NULL) {
			stop = true;
			break;
		}
		if (WITH->LastLayer->FirstNode != NULL)
			stop = true;
		else
			ist_remove_last_layer(V.STR);
	}
	if (ist_is_empty(V.STR) == false)
		ist_normalize(V.STR);
	return V.STR;
}

static ISTNode *SubsumedPaths(node1, node2, LINK)
	ISTNode *node1, *node2;
	struct LOC_ist_method  *LINK;
{
	ISTSon *s1, *s2;
	ISTNode *rnode, *rchild;

	if (ist_equal_interval(node1->Info,&IST_end_of_list))
		rnode = ist_add_node(LINK->rlayer, ist_create_node(&IST_end_of_list));
	else {
		rnode = ist_create_node(node1->Info);
		LINK->rlayer = LINK->rlayer->Next;
		if (LINK->rlayer == NULL)
			LINK->rlayer = ist_add_last_layer(LINK->STR);
		s1 = node1->FirstSon;
		while (s1 != NULL) {
			s2 = node2->FirstSon;
			while (s2 != NULL) {
				if (ist_include_interval(s2->Son->Info,s1->Son->Info)) {
					/* s1 \subset s2 */
					LINK->memo = ist_get_memoization1(s1->Son, s2->Son);
					if (LINK->memo != NULL)
						rchild = LINK->memo->r;
					else
						rchild = SubsumedPaths(s1->Son, s2->Son, LINK);
					if (rchild != NULL)
						ist_add_son(rnode, rchild);
					s2 = s2->Next;
				} else {
					if (ist_less_interval(s2->Son->Info,s1->Son->Info))
						s2 = s2->Next;
					else
						s2 = NULL;
				}
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
	ist_put_memoization1(node1, node2, rnode);
	return rnode;
}

ISTSharingTree *ist_compute_subsumed_paths(ST1, ST2)
	ISTSharingTree *ST1, *ST2;
{
	struct LOC_ist_method  V;
	ISTSon *s1, *s2;
	ISTNode *rchild;
	boolean stop;
	ISTSharingTree *WITH;

	ist_new(&V.STR);
	ist_new_memo1_number();
	V.rlayer = ist_add_last_layer(V.STR);
	s1 = ST1->Root->FirstSon;
	while (s1 != NULL) {
		s2 = ST2->Root->FirstSon;
		while (s2 != NULL) {
			if (ist_include_interval(s2->Son->Info,s1->Son->Info)) {
				/* s1 \subset s2 */
				rchild = SubsumedPaths(s1->Son, s2->Son, &V);
				if (rchild != NULL)
					ist_add_son(V.STR->Root, rchild);
				s2 = s2->Next;
			} else {
				if (ist_less_interval(s2->Son->Info,s1->Son->Info))
					s2 = s2->Next;
				else
					s2 = NULL;
			}
		}
		s1 = s1->Next;
	}

	WITH = V.STR;
	stop = false;
	while (!stop) {
		if (WITH->LastLayer == NULL) {
			stop = true;
			break;
		}
		if (WITH->LastLayer->FirstNode != NULL)
			stop = true;
		else
			ist_remove_last_layer(V.STR);
	}
	if (ist_is_empty(V.STR) == false)
		ist_normalize(V.STR);
	return V.STR;
}


ISTSharingTree *ist_remove_subsumed_paths(S, T)
	ISTSharingTree *S, *T;
{
	ISTSharingTree *Subsumed, *Sol;

	Subsumed = ist_compute_subsumed_paths(S, T);
	if (ist_is_empty(Subsumed) == false) {
		Sol = ist_minus(S, Subsumed);
		ist_dispose(Subsumed);
	} else {
		ist_dispose(Subsumed);
		Sol = ist_copy(S);
	}
	return Sol;
}




static ISTNode *SubsumedPathsWithinTree(node1, node2, DifferentNode, LINK)
	ISTNode *node1, *node2;
	boolean DifferentNode;
	struct LOC_ist_method  *LINK;
{
	ISTSon *s1, *s2;
	ISTNode *rnode, *rchild;
	boolean NewDifferentNode;

	if (ist_equal_interval(node1->Info,&IST_end_of_list)) {
		if (DifferentNode == true)
			rnode = ist_add_node(LINK->rlayer, ist_create_node(&IST_end_of_list));
		else
			rnode = NULL;
	} else {
		rnode = ist_create_node(node1->Info);
		LINK->rlayer = LINK->rlayer->Next;
		if (LINK->rlayer == NULL)
			LINK->rlayer = ist_add_last_layer(LINK->STR);
		s1 = node1->FirstSon;
		while (s1 != NULL) {
			s2 = node2->FirstSon;
			while (s2 != NULL) {
				if (ist_include_interval(s2->Son->Info,s1->Son->Info)) {
					/* s1 \subset s2 */
					if (s1->Son != s2->Son)
						NewDifferentNode = true;
					else
						NewDifferentNode = DifferentNode;
					if (NewDifferentNode == true)
						LINK->memo = ist_get_memoization1(s1->Son, s2->Son);
					else
						LINK->memo = ist_get_memoization1(s1->Son,NULL);
					if (LINK->memo != NULL)
						rchild = LINK->memo->r;
					else
						rchild = SubsumedPathsWithinTree(s1->Son, s2->Son, NewDifferentNode, LINK);
					if (rchild != NULL)
						ist_add_son(rnode, rchild);
					s2 = s2->Next;
				} else {
					if (ist_less_or_equal_interval(s2->Son->Info,s1->Son->Info))
						s2 = s2->Next;
					else
						s2 = NULL;
				}
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
	if (DifferentNode == true)
		ist_put_memoization1(node1, node2, rnode);
	else
		ist_put_memoization1(node1,NULL, rnode);
	return rnode;
}

ISTSharingTree *ist_compute_subsumed_paths_within_tree(S)
	ISTSharingTree *S;
{
	struct LOC_ist_method  V;
	ISTSon *s1, *s2;
	ISTNode *rchild;
	boolean stop;
	ISTSharingTree *WITH;

	ist_new(&V.STR);

	ist_new_memo1_number();
	V.rlayer = ist_add_last_layer(V.STR);
	s1 = S->Root->FirstSon;
	while (s1 != NULL) {
		s2 = S->Root->FirstSon;
		while (s2 != NULL) {
			if (ist_include_interval(s2->Son->Info,s1->Son->Info)) {
				/* s1 \subset s2 */ 
				if (s1->Son != s2->Son)
					rchild = SubsumedPathsWithinTree(s1->Son, s2->Son, true, &V);
				else
					rchild = SubsumedPathsWithinTree(s1->Son, s2->Son, false, &V);
				if (rchild != NULL)
					ist_add_son(V.STR->Root, rchild);
				s2 = s2->Next;
			} else {
				if (ist_less_or_equal_interval(s2->Son->Info,s1->Son->Info))
					s2 = s2->Next;
				else
					s2 = NULL;
			}
		}
		s1 = s1->Next;
	}

	WITH = V.STR;

	stop = false;
	while (!stop) {
		if (WITH->LastLayer == NULL) {
			stop = true;
		}else {
			if (WITH->LastLayer->FirstNode != NULL)
				stop = true;
			else
				ist_remove_last_layer(V.STR);
		}
	}
	WITH->NbElements = WITH->Root->AuxI;

	if (ist_is_empty(V.STR) == false)
		ist_normalize(V.STR);

	return V.STR;
}


ISTSharingTree *ist_minimal_form(S)
	ISTSharingTree *S;
{
	ISTSharingTree *Sol, *Subsumed;

	Subsumed = ist_compute_subsumed_paths_within_tree(S);
	if (ist_is_empty(Subsumed) == false)
		Sol = ist_minus(S, Subsumed);
	else
		Sol = ist_copy(S);
	ist_dispose(Subsumed);
	return Sol;
}

static void replace_node_correctly_in_layer(Layer,N)
	ISTLayer *Layer;
	ISTNode *N;
{
	ISTNode *p;
	ISTNode *q;

	q = NULL;
	p = Layer->FirstNode;
	while (p != NULL && p != N ){
		q = p;
		p = p->Next;
	}
	if (q == NULL){
		Layer->FirstNode =  Layer->FirstNode->Next;
	} else {
		q->Next = p->Next;
	}
	if (Layer->LastNode == N){
		Layer->LastNode = q;
	}
	/* Now N is extracted from the list, we reinsert it */
	ist_add_node_star(Layer,N);
	/* 
	 * We use ist_add_node_star and not ist_add_node because
	 * it's possible to obtain 2 nodes on the same layer with
	 * the same value and the same sons. However we will readjust
	 * afterwards with ist_adjust_second_condition_sons_fathers, take care that ist_adjust_second_condition
	 * will destroy the AuxP pointers.
	 */
}

static void replace_node_correctly_in_sonlist(Father_of_N,N)
	ISTNode *Father_of_N;
	ISTNode *N;
{
	ISTSon *p;
	ISTSon *q;

	q = NULL;
	p = Father_of_N->FirstSon;
	while (p != NULL && p->Son != N ){
		q = p;
		p = p->Next;
	}
	if (q == NULL){
		Father_of_N->FirstSon =  Father_of_N->FirstSon->Next;
	} else {
		q->Next = p->Next;
	}
	ist_dispose_son(p);
	/* ist_add_son will increment the number of Fathers, so we have to decrement it before */
	--N->NbFathers;

	/* Now N is extracted from the list, we reinsert it */
	ist_add_son(Father_of_N,N);
	/*
	 * We deal only with Son's of Father_of_N and not with Father of N so we use ist_add_son
	 * and not ist_add_son_father
	 */
}

static void replace_node_correctly_in_fatherlist(N,Son_of_N)
	ISTNode *N;
	ISTNode *Son_of_N;
{
	ISTSon *p;
	ISTSon *q;

	q = NULL;
	p = Son_of_N->FirstFather;
	while (p != NULL && p->Son != N ){
		q = p;
		p = p->Next;
	}
	if (q == NULL){
		Son_of_N->FirstFather =  Son_of_N->FirstFather->Next;
	} else {
		q->Next = p->Next;
	}
	ist_dispose_son(p);
	/* ist_add_father will increment the number of Sons, so we have to decrement it before */
	--N->NbSons;

	/* Now N is extracted from the list, we reinsert it */
	ist_add_father(N,Son_of_N);

}



static boolean LocalEdgeRemovalSonSide(ISTLayer *Layer){
	ISTNode *Node;
	ISTSon *NodeSon, *NodeSonNext, *SimulNode, *SimulSon, *Simul;
	boolean HasPruned;

	HasPruned = false;
	Node = Layer->FirstNode;
	while (Node != NULL) {
		SimulNode = Node->BackRel;
		while (SimulNode != NULL) {
			SimulSon = SimulNode->Son->FirstSon;
			while (SimulSon != NULL) {
				NodeSon = Node->FirstSon;
				while (NodeSon != NULL) {
					NodeSonNext = NodeSon->Next;
					Simul = NodeSon->Son->Rel;
					while (Simul != NULL) {
						if (Simul->Son == SimulSon->Son &&
								(Node != SimulNode->Son || NodeSon->Son != SimulSon->Son)) {
							ist_remove_son_father(Node, NodeSon->Son);
							HasPruned = true;
						}
						Simul = Simul->Next;
					}
					NodeSon = NodeSonNext;
				}
				SimulSon = SimulSon->Next;
			}
			SimulNode = SimulNode->Next;
		}
		Node = Node->Next;
	}
	return HasPruned;
}

static boolean LocalEdgeRemovalFatherSide(ISTLayer *Layer){
	ISTNode *Node;
	ISTSon *NodeFather, *NodeFatherNext, *SimulNode, *SimulFather, *Simul;
	boolean HasPruned;

	HasPruned = false;
	Node = Layer->FirstNode;
	while (Node != NULL) {
		SimulNode = Node->Rel;
		while (SimulNode != NULL) {
			SimulFather = SimulNode->Son->FirstFather;
			while (SimulFather != NULL) {
				NodeFather = Node->FirstFather;
				while (NodeFather != NULL) {
					NodeFatherNext = NodeFather->Next;
					Simul = NodeFather->Son->BackRel;
					while (Simul != NULL) {
						if (Simul->Son == SimulFather->Son &&
								(Node != SimulNode->Son || NodeFather->Son != SimulFather->Son)) {
							ist_remove_son_father(NodeFather->Son, Node);
							HasPruned = true;
						}
						Simul = Simul->Next;
					}
					NodeFather = NodeFatherNext;
				}
				SimulFather = SimulFather->Next;
			}
			SimulNode = SimulNode->Next;
		}
		Node = Node->Next;
	}
	return HasPruned;
}

static void MarkInterestingNodesBySonWay(ISTNode *N,ISTLayer *L){
	ISTNode *q;
	ISTSon *SonN, *RelSonN, *FatherRelSonN;

	/*
	 * For a very special case, if here we mark nothing, if after we call BuildListOfInterestingNodesS
	 * It is possible that BuildListOfInterestingNodesS work on ancient marked nodes from the last call to MarkInterestingNodesBySonWay
	 */
	ist_new_magic_number();
	/* First we initialize the counters */
	q = L->FirstNode;
	while (q != NULL){
		q->Mark = 0;
		q = q->Next;
	}
	/* Then we count, we avoid couting twice a son by using the ist_get_magic_number() */
	SonN = N->FirstSon;
	while (SonN != NULL){
		ist_new_magic_number();
		RelSonN = SonN->Son->Rel;
		while (RelSonN != NULL){
			FatherRelSonN = RelSonN->Son->FirstFather;
			while (FatherRelSonN != NULL){
				if (FatherRelSonN->Son->AuxI != ist_get_magic_number())  {
					FatherRelSonN->Son->AuxI = ist_get_magic_number();
					++FatherRelSonN->Son->Mark;
				}
				FatherRelSonN = FatherRelSonN->Next;
			}
			RelSonN = RelSonN->Next;
		}
		SonN = SonN->Next;
	}
}



static void MarkInterestingNodesByFatherWay(ISTNode *N,ISTLayer *L){
	ISTNode *q;
	ISTSon *FatherN, *BackRelFatherN, *SonBackRelFatherN;
	/*
	 * For a very special case, if here we mark nothing, if after we call BuildListOfInterestingNodesF
	 * It is possible that BuildListOfInterestingNodesF work on ancient marked nodes from the last call to MarkInterestingNodesByFatherWay
	 */
	ist_new_magic_number();
	/* First we initialize the counters */
	q = L->FirstNode;
	while (q != NULL){
		q->Mark = 0;
		q = q->Next;
	}
	/* Then we count, we avoid couting twice a Father by using the ist_get_magic_number() */
	FatherN = N->FirstFather;
	while (FatherN != NULL){
		ist_new_magic_number();
		BackRelFatherN = FatherN->Son->BackRel;
		while (BackRelFatherN != NULL){
			SonBackRelFatherN = BackRelFatherN->Son->FirstSon;
			while (SonBackRelFatherN != NULL){
				if (SonBackRelFatherN->Son->AuxI != ist_get_magic_number())  {
					SonBackRelFatherN->Son->AuxI = ist_get_magic_number();
					++SonBackRelFatherN->Son->Mark;
				}
				SonBackRelFatherN = SonBackRelFatherN->Next;
			}
			BackRelFatherN = BackRelFatherN->Next;
		}
		FatherN = FatherN->Next;
	}
}

static ISTNode *BuildListOfInterestingNodesS(ISTNode *N,ISTLayer *L){
	ISTNode *p,*q;

	p = NULL;
	q = L->FirstNode;
	while (q != NULL) {
		if (q->AuxI == ist_get_magic_number() 
				&& q->Mark == N->NbSons && q != N){
			q->AuxP = p;
			p = q;
		}
		q = q->Next;
	}
	return p;
}

static ISTNode *BuildListOfInterestingNodesF(ISTNode *N,ISTLayer *L){
	ISTNode *p,*q;

	p = NULL;
	q = L->FirstNode;
	while (q != NULL) {
		if (q->AuxI == ist_get_magic_number() 
				&& q->Mark == N->NbFathers && q != N){
			q->AuxP = p;
			p = q;
		}
		q = q->Next;
	}
	return p;
}


static boolean IsIncluded(ISTNode* N,ISTNode *p){
	boolean res, fixpoint;
	ISTInterval  *inclusion;
	ISTNode *q, *r;

	res = false;

	inclusion = NULL;
	q = p;
	while (q != NULL && inclusion == NULL){
		if (q->AuxI == ist_get_magic_number() ){
			inclusion = ist_intersect_intervals(q->Info,N->Info);
			r = q;
			/* We remind the last position where we made an ist_intersect_intervals. */
		}
		q = q->AuxP;
	}

	if (inclusion != NULL){
		/*
		 * We have at least one element of the list that intersect N->Info 
		 * We saved his position in r.
		 */
		ist_assign_interval_to_interval(inclusion,r->Info);
		/* In the sequel we don't have to consider this element anymore. */
		r->Mark = 1;

		fixpoint = false;
		r = p;
		while (r != NULL && !fixpoint){
			fixpoint = true;
			q = p;
			while ( q!= NULL){
				if (q->AuxI == ist_get_magic_number() && q->Mark == 0){
					if (ist_convex_union(inclusion,q->Info)){
						q->Mark = 1;
						fixpoint = false;
					}
				}
				q = q->AuxP;
			}
			r = r->AuxP;
		}
		if (ist_include_interval(inclusion,N->Info))
			res = true;
		ist_dispose_info(inclusion);
	}
	return res;	
}

static ISTInterval* ComputeExtendedInfo(ISTNode *N,ISTNode *p){
	ISTInterval *extendedvalue;
	ISTNode *q, *r;
	boolean fixpoint;


	extendedvalue = ist_copy_interval(N->Info);

	fixpoint = false;
	r = p;
	while (r != NULL && !fixpoint){
		q = p;
		fixpoint = true;
		while (q!= NULL){
			if (q->AuxI == ist_get_magic_number() && q->Mark == N->NbFathers){
				/*
				 * Inside the list the only relevants nodes are the ones
				 * with q->AuxI == ist_get_magic_number() (marked by the last father of N)
				 * with q->Mark == N->NbFathers (marked by all the others fathers of N)
				 * In fact we consider only the nodes that sons simulates all the sons of N
				 * and that fathers simulates all the fathers of N
				 */
				if (ist_convex_union(extendedvalue,q->Info)){
					/* 
					 * This node is now inside the convex union
					 * in the future, we don't have to consider it anymore
					 * we flag it to 0 ... 0 is ok because if q->AuxI == ist_get_magic_number()
					 * q->Mark >= 1
					 */
					q->Mark = 0;
					fixpoint = false;
				}
			}
			q = q->AuxP;
		}
		r = r->AuxP;
	}
	if (ist_equal_interval(extendedvalue,N->Info)){
		ist_dispose_info(extendedvalue);
		extendedvalue =  NULL;
	}
	return extendedvalue;	
}

/* 
 * This function is the conclusion after our long list of static function. So that function
 * prune our IST, following the information provided by the simulation relation.
 * I encourage the developper to take a look at [Gan02] to understand deeply how we exploit the simulation relation.
 */
boolean ist_prune_within_tree_sim_based(S)
	ISTSharingTree *S;
{

	ISTLayer *Layer;
	ISTNode *N, *p;
	ISTSon *ParOf_N, *ParOf_N_Next, *SimOf_ParOf_N, *NodeInIntersect;
	ISTInterval *res;
	boolean HasPruned;

	HasPruned = false;
	Layer = S->LastLayer->Previous;
	while (Layer != NULL){
		if (LocalEdgeRemovalSonSide(Layer))
			HasPruned = true; 
		if (LocalEdgeRemovalFatherSide(Layer)) 
			HasPruned = true; 

		N = Layer->FirstNode;
		while (N != NULL){
			MarkInterestingNodesBySonWay(N,Layer);
			p = BuildListOfInterestingNodesS(N,Layer);
			ParOf_N = N->FirstFather;
			while (ParOf_N != NULL){
				ParOf_N_Next = ParOf_N->Next;
				ist_new_magic_number();
				SimOf_ParOf_N = ParOf_N->Son->BackRel;
				while (SimOf_ParOf_N != NULL){
					NodeInIntersect = SimOf_ParOf_N->Son->FirstSon;
					while (NodeInIntersect != NULL ){
						NodeInIntersect->Son->AuxI = ist_get_magic_number();
						NodeInIntersect->Son->Mark = 0;
						NodeInIntersect = NodeInIntersect->Next; 
					}
					SimOf_ParOf_N = SimOf_ParOf_N->Next;
				}
				if (IsIncluded(N,p)){
					ist_remove_son_father(ParOf_N->Son,N);
					HasPruned = true;
				}		  
				ParOf_N = ParOf_N_Next;
			}

			MarkInterestingNodesByFatherWay(N,Layer);
			p = BuildListOfInterestingNodesF(N,Layer);
			ParOf_N = N->FirstSon;
			while (ParOf_N != NULL){
				ParOf_N_Next = ParOf_N->Next;
				ist_new_magic_number();
				SimOf_ParOf_N = ParOf_N->Son->Rel;
				while (SimOf_ParOf_N != NULL){
					NodeInIntersect = SimOf_ParOf_N->Son->FirstFather;
					while (NodeInIntersect != NULL ){
						NodeInIntersect->Son->AuxI = ist_get_magic_number();
						NodeInIntersect->Son->Mark = 0;
						NodeInIntersect = NodeInIntersect->Next; 
					}
					SimOf_ParOf_N = SimOf_ParOf_N->Next;
				}
				if (IsIncluded(N,p)){
					ist_remove_son_father(N,ParOf_N->Son);
					HasPruned = true;
				}		  
				ParOf_N = ParOf_N_Next;
			}
			MarkInterestingNodesBySonWay(N,Layer);
			p = BuildListOfInterestingNodesS(N,Layer);
			MarkInterestingNodesByFatherWay(N,Layer);

			res = ComputeExtendedInfo(N,p);
			if (res != NULL ){
				/*
				 * Here, we will change the information inside the node. 
				 * After modifying, we have to reinsert N at its right place
				 * in the list of node its layer, in the list of sons of its fathers
				 * in the list of fathers of its sons ...
				 */
				ist_dispose_info(N->Info);
				N->Info = res;
				replace_node_correctly_in_layer(Layer,N);
				ParOf_N = N->FirstFather;
				while ( ParOf_N != NULL){
					replace_node_correctly_in_sonlist(ParOf_N->Son,N);
					ParOf_N = ParOf_N->Next;
				}

				ParOf_N = N->FirstSon;
				while (ParOf_N != NULL){
					replace_node_correctly_in_fatherlist(N,ParOf_N->Son);
					ParOf_N = ParOf_N->Next;
				}
				HasPruned = true;
			}
			N = N->Next;
		}
		Layer = Layer->Previous;
	}
	return HasPruned;
}


/*minimize the ST S*/
void ist_minimal_form_sim_based(S)
	ISTSharingTree *S;
{
	boolean Modified;

	ist_construct_fathers_info(S);
	Modified = true;
	while (Modified == true) {
		ComputeBackwardSimulation(S);
		ComputeForwardSimulation(S);
		Modified = ist_prune_within_tree_sim_based(S);
		/* 
		 * We have to normalize. Why ? e.g. : 
		 * We have two nodes n and n' issued from the root node
		 * there is no forward simulation relation between each other
		 * We extend the information such that n->Info == n'->Info.
		 * There is still no forward simulation but the first syntactic
		 * conditions doesn't hold anymore. First we clean the sharing tree,
		 * otherwise ist_adjust_first_condition_sons_fathers dump!
		 */
		ist_dispose_node_without_son(S);
		ist_dispose_node_without_father(S);
		ist_normalize_sons_fathers(S);
	}
	if (ist_is_empty(S) == false)
		ist_dispose_fathers_info(S);
}

boolean ist_exact_subsumption_test(T,S)
	ISTSharingTree *T, *S;
{
	boolean res;
	ISTSharingTree *S_bis, *intersec, *temp;

	if (ist_is_empty(T) == false) {
		S_bis = ist_copy(S);	
		ist_complement(S_bis,ist_nb_layers(T)-1);
		//-------FOR Debugging{----------
		temp = ist_intersection(S_bis,S);
		assert(ist_is_empty(temp));
		ist_dispose(temp);
		//-------FOR Debugging}----------
		intersec = ist_intersection(S_bis,T);
		res = ist_is_empty(intersec);
		ist_dispose(S_bis);
		ist_dispose(intersec);
	} else 
		res = true;

	return res;
}
