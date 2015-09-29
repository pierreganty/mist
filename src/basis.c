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

   Copyright 2003, 2004, Pierre Ganty, 2007 Laurent Van Begin
 */


/*
 * The functions that we define in this file are the most basic ones that we
 * provide with the library.  To understand clearly how to play with ISTs, I
 * encourage the developper to read carefully these algorithms together with
 * the explaination of [Zam97] and [Gan02]
 */

#include "basis.h"
#include "error.h"
#include "xmalloc.h"
#include <stdlib.h>
#include <stdio.h>

inline boolean ist_is_empty(ST)
	ISTSharingTree *ST;
{
	return (ST->Root->FirstSon == NULL);
}

/*
 * Taken a IST 'ST', a vector of intervals 'Info' together with the lenght.
 * This function returns true if the LInfo first elements of Info are into the
 * IST 'ST'.
 */
boolean ist_is_member(ST, Info, LInfo)
	ISTSharingTree *ST;
	ISTInterval **Info;
	integer16 LInfo;
{
	boolean Result;
	integer16 i;
	ISTNode *node;

	Result = false;
	i = 0;
	node = ST->Root;
	while (node != NULL) {
		if (i >= LInfo) {
			node = ist_has_son_with_value(node, &IST_end_of_list);
			Result = (node != NULL);
			node = NULL;
		} else {
			node = ist_has_son_with_value(node, Info[i]);
			if (node != NULL)
				i++;
		}
	}
	return Result;
}


/*
 * This function is used by ist_copy defined below, take a look first at it
 * then go back to this function.
 */
static ISTNode *Copy(node, LINK)
	ISTNode *node;
	struct LOC_ist_method  *LINK;
{
	ISTSon *s;
	ISTNode *rnode;

	rnode = ist_create_node(node->Info);
	node->AuxI = ist_get_magic_number();
	if (ist_not_equal_interval(node->Info,&IST_end_of_list)) {
		LINK->rlayer = LINK->rlayer->Next;
		if (LINK->rlayer == NULL)
			LINK->rlayer = ist_add_last_layer(LINK->STR);
		s = node->FirstSon;
		while (s != NULL) {
			if (s->Son->AuxI == ist_get_magic_number())
				ist_add_son(rnode, s->Son->AuxP);
			else
				ist_add_son(rnode, Copy(s->Son, LINK));
			s = s->Next;
		}
		LINK->rlayer = LINK->rlayer->Previous;
	}
	node->AuxP = ist_add_node(LINK->rlayer, rnode);
	return (node->AuxP);
}


/* This function returns the copy of the IST provided in parameter */
ISTSharingTree *ist_copy(ST)
	ISTSharingTree *ST;
{
	struct LOC_ist_method  V;
	ISTSon *s;
	ISTNode *rchild;

	ist_new_magic_number();
	ist_new(&V.STR);
	if (!ist_is_empty(ST)) {
		V.rlayer = ist_add_last_layer(V.STR);
		s = ST->Root->FirstSon;
		while (s != NULL) {
			rchild = Copy(s->Son, &V);
			ist_add_son(V.STR->Root, rchild);
			s = s->Next;
		}
	}
	V.STR->NbElements = ST->NbElements;
	return V.STR;
}



/*
 * This function is used by ist_equal defined below, take a look first at it then
 * go back to this function.
 */
static boolean Equal(node1, node2)
	ISTNode *node1, *node2;
{
	boolean Result;
	ISTSon *s1, *s2;

	Result = true;
	node1->AuxI = ist_get_magic_number();
	node2->AuxI = ist_get_magic_number();
	if (ist_not_equal_interval(node1->Info,node2->Info) || (ist_number_of_sons(node1) != ist_number_of_sons(node2)))
		return false;
	s1 = node1->FirstSon;
	s2 = node2->FirstSon;
	while (s1 != NULL) {
		if (s1->Son->AuxI == ist_get_magic_number() && s2->Son->AuxI == ist_get_magic_number()) {
			s1 = s1->Next;
			s2 = s2->Next;
		} else {
			if (Equal(s1->Son, s2->Son)) {
				s1 = s1->Next;
				s2 = s2->Next;
			} else {
				Result = false;
				s1 = NULL;
			}
		}
	}
	return Result;
}


boolean ist_equal(ST1, ST2)
	ISTSharingTree *ST1, *ST2;
{
	ist_new_magic_number();
	return (Equal(ST1->Root, ST2->Root));
}


/*
 * This function is used by ist_included defined below, take a look first at it then
 * go back to this function.
 */
static boolean Included(node1, node2)
	ISTNode *node1, *node2;
{
	boolean Result;
	ISTSon *s1;
	ISTNode *s2;

	Result = true;
	node1->AuxI = ist_get_magic_number();
	node2->AuxI = ist_get_magic_number();
	if (ist_not_equal_interval(node1->Info,node2->Info))
		return false;
	s1 = node1->FirstSon;
	while (s1 != NULL) {
		s2 = ist_has_son_with_value(node2, s1->Son->Info);
		if (s2 == NULL) {
			Result = false;
			s1 = NULL;
			break;
		}
		if (s1->Son->AuxI == ist_get_magic_number() && s2->AuxI == ist_get_magic_number())
			s1 = s1->Next;
		else if (Included(s1->Son, s2))
			s1 = s1->Next;
		else {
			Result = false;
			s1 = NULL;
		}
	}
	return Result;
}


/*
 * This function check if ST1 is included in ST2. By included we mean
 * a syntactic inclusion (w.r.t the labels of the nodes) not a semantic
 * one (i.e. work with intervals as set of values).
 * The semantic one is defined by the function ist_exact_subsumption_test.
 */
boolean ist_included(ST1, ST2)
	ISTSharingTree *ST1, *ST2;
{
	ist_new_magic_number();
	return (Included(ST1->Root, ST2->Root));
}

/*
 * LOC_ist_add is a structure used locally in ist_add. Most of the time algorithms are recursive.
 * To keep the code readable, instead of passing a plenty of parameters, we pass a data stucture
 * of this kind where all the parameters are wrapped in.
 */
struct LOC_ist_add {
	ISTSharingTree *ST;
	/* Info is a vector of pointer to ISTInterval */
	ISTInterval **Info;
	integer16 LInfo, depth;
	ISTInterval* v;
	ISTLayer *layer;
	ISTNode *nodep;
	boolean newelement;
} ;


/*
 * This function is used by ist_add defined below, take a look first at it then
 * go back to this function.
 */
static ISTNode *Add(node, inlayer, mustchange, LINK)
	ISTNode *node;
	boolean inlayer, mustchange;
	struct LOC_ist_add *LINK;
	/*
	 * inlayer told us if the node is already in the layer LINK->layer or not
	 * mustchange told us that we have to modify the sharing tree before adding the node 'node'
	 */
{
	ISTNode *son, *res;

	if (ist_equal_interval(node->Info,&IST_end_of_list)) {
		if (inlayer)
			return node;
		else
			return (ist_add_node(LINK->layer, node));
	} else {
		if (node->NbFathers > 1)
			mustchange = true;
		LINK->depth++;
		if (LINK->depth < LINK->LInfo)
			LINK->v = LINK->Info[LINK->depth];
		else
			LINK->v = &IST_end_of_list;
		LINK->layer = LINK->layer->Next;
		if (LINK->layer == NULL)
			LINK->layer = ist_add_last_layer(LINK->ST);
		son = ist_has_son_with_value(node, LINK->v);
		if (son == NULL) {
			LINK->newelement = true;
			res = Add(ist_create_node(LINK->v), false, false, LINK);
		} else
			res = Add(son, true, mustchange, LINK);
		LINK->layer = LINK->layer->Previous;
		if (res == son) {
			if (inlayer)
				return node;
			else
				return (ist_add_node(LINK->layer, node));
		} else {
			if (mustchange) {
				LINK->nodep = ist_create_node(node->Info);
				ist_copy_sons(node, LINK->nodep);
				node = LINK->nodep;
				inlayer = false;
			}
			if (son == NULL)
				ist_add_son(node, res);
			else {
				ist_replace_son(node, son, res);
				if (son->NbFathers == 0)
					ist_remove_node(LINK->layer->Next, son);
			}
			if (inlayer) {
				LINK->nodep = ist_exists_node(LINK->layer, node);
				if (LINK->nodep == NULL)
					return node;
				else
					return LINK->nodep;
			} else
				return (ist_add_node(LINK->layer, node));
		}
	}
}


/*
 * ist_add takes in parameter an IST, a vector of intervals and a value <= size of that vector.
 * ist_add returns true if the new element is effectively added to the IST, false if
 * the element was already in the IST.
 */
boolean ist_add(ST_, Info_, LInfo_)
	ISTSharingTree *ST_;
	ISTInterval **Info_;
	integer16 LInfo_;
{
	struct LOC_ist_add V;
	ISTNode *son;

	V.ST = ST_;
	V.Info = Info_;
	V.LInfo = LInfo_;
	V.depth = 0;
	V.newelement = false;
	/* depth is the current pointer in the vector V.Info */
	if (V.depth < V.LInfo){
		V.v = V.Info[V.depth];
	} else {
		V.v = &IST_end_of_list;
	}
	son = ist_has_son_with_value(V.ST->Root, V.v);
	V.layer = V.ST->FirstLayer;
	if (V.layer == NULL){
		V.layer = ist_add_last_layer(V.ST);
	}
	if (son == NULL) {
		V.newelement = true;
		ist_add_son(V.ST->Root, Add(ist_create_node(V.v), false, false, &V));
	} else {
		Add(son, true, false, &V);
 	}
	if (V.newelement) {
		if (V.ST->NbElements >= 0)
			V.ST->NbElements++;
	}
	return V.newelement;
}

/*
 * This function is used by ist_union defined below, take a look first at it then
 * go back to this function.
 */
static ISTNode *Union(node1, node2, LINK)
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
		s2 = node2->FirstSon;
		while (s1 != NULL || s2 != NULL) {
			if (s1 == NULL) {
				if (s2->Son->AuxI == ist_get_magic_number())
					rchild = s2->Son->AuxP;
				else
					rchild = Copy(s2->Son, LINK);
				s2 = s2->Next;
			} else if (s2 == NULL) {
				if (s1->Son->AuxI == ist_get_magic_number())
					rchild = s1->Son->AuxP;
				else
					rchild = Copy(s1->Son, LINK);
				s1 = s1->Next;
			} else if (ist_equal_interval(s1->Son->Info,s2->Son->Info)) {
				/* s1	[	] 							*/
				/* s2	[	]						 	*/
				LINK->memo = ist_get_memoization1(s1->Son, s2->Son);
				if (LINK->memo != NULL)
					rchild = LINK->memo->r;
				else
					rchild = Union(s1->Son, s2->Son, LINK);
				s1 = s1->Next;
				s2 = s2->Next;
			} else if (ist_less_interval(s1->Son->Info,s2->Son->Info)) {
				/* s1	[				*/
				/* s2	    [				*/
				/* OR					*/
				/* s1	[	]			*/
				/* s2	[		]		*/
				if (s1->Son->AuxI == ist_get_magic_number())
					rchild = s1->Son->AuxP;
				else
					rchild = Copy(s1->Son, LINK);
				s1 = s1->Next;
			} else {
				/* s1	[			]	*/
				/* s2	[		]		*/
				/* OR				*/
				/* s1		[		*
				 * s2	[			*/

				if (s2->Son->AuxI == ist_get_magic_number())
					rchild = s2->Son->AuxP;
				else
					rchild = Copy(s2->Son, LINK);
				s2 = s2->Next;
			}

			if (rchild != NULL) {
				ist_add_son(rnode, rchild);
			}
		}
		LINK->rlayer = LINK->rlayer->Previous;
		rnode = ist_add_node(LINK->rlayer, rnode);
	}
	ist_put_memoization1(node1, node2, rnode);
	return rnode;
}


/*
 * This function returns the union of the ISTs ST1 and ST2.
 * Union  semantic (i.e. work with intervals as set of values) not syntactic (i.e. work with labels of nodes).
 */
ISTSharingTree *ist_union(ST1, ST2)
	ISTSharingTree *ST1, *ST2;
{
	struct LOC_ist_method  V;
	ISTSon *s1, *s2;
	ISTNode *rchild = NULL;

	ist_new(&V.STR);
	ist_new_magic_number();
	ist_new_memo1_number();
	s1 = ST1->Root->FirstSon;
	s2 = ST2->Root->FirstSon;
	if (s1 != NULL || s2 != NULL){
		/* The union of two empty sets is also an empty set */
		V.rlayer = ist_add_last_layer(V.STR);
		while (s1 != NULL || s2 != NULL) {
			/* Notice that this is a OR */
			if (s1 == NULL) {
				rchild = Copy(s2->Son, &V);
				s2 = s2->Next;
			} else if (s2 == NULL) {
				rchild = Copy(s1->Son, &V);
				s1 = s1->Next;
			} else if (ist_equal_interval(s1->Son->Info,s2->Son->Info))  {
				/* s1	[	] 							*/
				/* s2	[	]						 	*/
				rchild = Union(s1->Son, s2->Son, &V);
				s1 = s1->Next;
				s2 = s2->Next;
			} else if (ist_less_interval(s1->Son->Info,s2->Son->Info)) {
				/* s1	[
				   s2	    [
				   OR	 s1	[	]
				   s2	[		]		*/
				rchild = Copy(s1->Son, &V);
				s1 = s1->Next;
			} else if (ist_greater_interval(s1->Son->Info,s2->Son->Info)) {
				/* s1	[		]
				   s2	[	]
				   OR	 s1		[
				   s2	[			*/
				rchild = Copy(s2->Son, &V);
				s2 = s2->Next;
			}
			if (rchild != NULL)
				ist_add_son(V.STR->Root, rchild);
		}
		V.STR->NbElements = V.STR->Root->AuxI;
	}
	return V.STR;
}

/*
 * This function is used by ist_intersection defined below, take a look first at ist_add then
 * go back to this function.
 */
static ISTNode *Intersection(node1, node2, LINK)
	ISTNode *node1, *node2;
	struct LOC_ist_method  *LINK;
{
	/* We have to test all the combinations of nodes */
	ISTSon *s1, *s2;
	ISTNode *rnode, *rchild;

	if (ist_equal_interval(node1->Info,&IST_end_of_list))
		rnode = ist_add_node(LINK->rlayer, ist_create_node(&IST_end_of_list));
	else {
		/* We have an intersection */
		rnode = ist_create_node(LINK->intersect);
		ist_dispose_info(LINK->intersect);
		LINK->rlayer = LINK->rlayer->Next;
		if (LINK->rlayer == NULL)
			LINK->rlayer = ist_add_last_layer(LINK->STR);
		s1 = node1->FirstSon;
		while (s1 != NULL ) {
			s2 = node2->FirstSon;
			while ( s2 != NULL) {
				LINK->intersect = ist_intersect_intervals(s1->Son->Info,s2->Son->Info);
				if (LINK->intersect != NULL) {
					LINK->memo = ist_get_memoization1(s1->Son, s2->Son);
					if (LINK->memo != NULL){
						rchild = LINK->memo->r;
						/* We don't need it anymore, we have computed the result previously */
						ist_dispose_info(LINK->intersect);
					} else
						rchild = Intersection(s1->Son, s2->Son, LINK);
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
	ist_put_memoization1(node1, node2, rnode);
	return rnode;
}

/*
 * This function returns the intersection of the ISTs ST1 and ST2.
 * Intersection semantic (i.e. work with intervals as set of values) not syntactic (i.e. work with labels of nodes).
 */
ISTSharingTree *ist_intersection(ST1, ST2)
	ISTSharingTree *ST1, *ST2;
{
	struct LOC_ist_method  V;
	ISTSon *s1, *s2;
	ISTNode *rchild;
	boolean stop;

	ist_new(&V.STR);
	ist_new_memo1_number();
	V.rlayer = ist_add_last_layer(V.STR);

	s1 = ST1->Root->FirstSon;
	while (s1 != NULL ){
		s2 = ST2->Root->FirstSon;
		while ( s2 != NULL) {
			V.intersect = ist_intersect_intervals(s1->Son->Info,s2->Son->Info);
			if (V.intersect != NULL) {
				rchild = Intersection(s1->Son, s2->Son, &V);
				if (rchild != NULL)
					ist_add_son(V.STR->Root, rchild);
			}
			s2 = s2->Next;
		}
		s1 = s1->Next;
	}
	stop = false;
	while (!stop) {
		if (V.STR->LastLayer == NULL)
			stop = true;
		else {
			if (V.STR->LastLayer->FirstNode != NULL)
				stop = true;
			else
				ist_remove_last_layer(V.STR);
		}
	}
	V.STR->NbElements = V.STR->Root->AuxI;
	if(ist_is_empty(V.STR)==false) {
//		printf("ist_intersection before normilzation");
//		ist_stat(V.STR);
		ist_normalize(V.STR);
//		printf("ist_intersection after normilzation");
//		ist_stat(V.STR);
//		printf("initial ist\n");
//		ist_stat(ST1);
//		ist_stat(ST2);
	}
	return V.STR;
}

/*
 * This function is used by ist_minus defined below, take a look first at ist_add then
 * go back to this function.
 */
static ISTNode *Minus(node1, node2, LINK)
	ISTNode *node1, *node2;
	struct LOC_ist_method  *LINK;
{
	ISTSon *s1, *s2;
	ISTNode *rchild, *rnode;

	rnode = ist_create_node(node1->Info);
	LINK->rlayer = LINK->rlayer->Next;
	if (LINK->rlayer == NULL)
		LINK->rlayer = ist_add_last_layer(LINK->STR);
	s1 = node1->FirstSon;
	s2 = node2->FirstSon;
	while (s1 != NULL) {
		if (s2 == NULL) {
			if (s1->Son->AuxI == ist_get_magic_number())
				rchild = s1->Son->AuxP;
			else
				rchild = Copy(s1->Son, LINK);
			s1 = s1->Next;
		} else if (ist_equal_interval(s1->Son->Info,s2->Son->Info)){
			if (ist_equal_interval(s1->Son->Info,&IST_end_of_list))
				rchild = NULL;
			else {
				LINK->memo = ist_get_memoization1(s1->Son, s2->Son);
				if (LINK->memo != NULL)
					rchild = LINK->memo->r;
				else
					rchild = Minus(s1->Son, s2->Son, LINK);
			}
			s1 = s1->Next;
			s2 = s2->Next;
		} else if (ist_less_interval(s1->Son->Info,s2->Son->Info)) {
			if (s1->Son->AuxI == ist_get_magic_number())
				rchild = s1->Son->AuxP;
			else
				rchild = Copy(s1->Son, LINK);
			s1 = s1->Next;
		} else {
			rchild = NULL;
			s2 = s2->Next;
		}
		if (rchild != NULL) {
			ist_add_son(rnode, rchild);
		}
	}
	LINK->rlayer = LINK->rlayer->Previous;
	if (rnode->FirstSon != NULL)
		rnode = ist_add_node(LINK->rlayer, rnode);
	else {
		ist_dispose_node(rnode);
		rnode = NULL;
	}
	ist_put_memoization1(node1, node2, rnode);
	return rnode;
}


ISTSharingTree *ist_minus(ST1, ST2)
	ISTSharingTree *ST1, *ST2;
	/* Minus syntactic (i.e. work with labels of nodes) not semantic (i.e. work with intervals as set of values) */
{
	struct LOC_ist_method  V;
	ISTSon *s1, *s2;
	ISTNode *rchild;
	boolean stop;
	ISTSharingTree *WITH;

	ist_new(&V.STR);
	ist_new_magic_number();
	ist_new_memo1_number();
	if (ist_is_empty(ST1))
		return V.STR;
	V.rlayer = ist_add_last_layer(V.STR);
	s1 = ST1->Root->FirstSon;
	s2 = ST2->Root->FirstSon;
	while (s1 != NULL) {
		if (s2 == NULL) {
			rchild = Copy(s1->Son, &V);
			s1 = s1->Next;
		} else if (ist_equal_interval(s1->Son->Info,s2->Son->Info)) {
			rchild = Minus(s1->Son, s2->Son, &V);
			s1 = s1->Next;
			s2 = s2->Next;
		} else if (ist_less_interval(s1->Son->Info,s2->Son->Info)) {
			rchild = Copy(s1->Son, &V);
			s1 = s1->Next;
		} else {
			rchild = NULL;
			s2 = s2->Next;
		}
		if (rchild != NULL) {
			ist_add_son(V.STR->Root, rchild);
		}
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
	return V.STR;
}


int ist_nb_sons(ST)
	ISTSharingTree *ST;
{
	int  n;
	ISTLayer *layer;
	ISTNode *node;

	n = ist_number_of_sons(ST->Root);
	layer = ST->FirstLayer;
	while (layer != NULL) {
		node = layer->FirstNode;
		while (node != NULL) {
			n += ist_number_of_sons(node);
			node = node->Next;
		}
		layer = layer->Next;
	}
	return n;
}

int ist_nb_nodes(ST)
	ISTSharingTree *ST;
{
	int n;
	ISTLayer *layer;
	ISTNode *node;

	n = 1;
	layer = ST->FirstLayer;
	while (layer != NULL) {
		node = layer->FirstNode;
		while (node != NULL) {
			n++;
			node = node->Next;
		}
		layer = layer->Next;
	}
	return n;
}


int ist_nb_layers(ST)
	ISTSharingTree *ST;
{
	int n;
	ISTLayer *layer;

	n = 0;
	layer = ST->FirstLayer;
	while (layer != NULL) {
		n++;
		layer = layer->Next;
	}
	return n;
}


long ist_nb_elements(S)
	ISTSharingTree *S;
{
	ist_count_elements(S);
	return (S->NbElements);
}

/* See below void ist_write(S) */
static void STWriteElem(path, l)
	ISTInterval **path;
	integer16 l;
{
	integer16 i;

	if (l <= 0)
		return;
	printf("-<");
	for (i = 0; i < l-1; i++){
		printf("[%2ld", path[i]->Left);
		if(path[i]->Right==INFINITY){
			printf(", \u221E]");
		} else
			printf(",%2ld]", path[i]->Right);
	}
	printf(">-\n");
}

/* See below void ist_write(S) */
static void st_Write(N, path, i, l)
	ISTNode *N;
	ISTInterval **path;
	integer32 i, l;
{
	ISTSon *s;

	if (N == NULL) {
		err_msg("basic.c : st_Write : Found partial path\n");
		STWriteElem(path, l);
		return;
	}
	if (i >= l)
		STWriteElem(path, l);
	if (i > 0L && i < l)
		/* From 1 to l-1, from FirstLayer to LastLayer->Previous
		   because we do not care of Lastlayer */
		path[i-1] = N->Info;
	s = N->FirstSon;
	while (s != NULL) {
		st_Write(s->Son, path, i + 1, l);
		s = s->Next;
	}
}

/* This function prints all the element of the IST S */
void ist_write(S)
	ISTSharingTree *S;
{
	size_t l;
	/* A vector of pointer to intervals */
	ISTInterval **path;
	l = ist_nb_layers(S);
	if (l>=1) {
		path = (ISTInterval **)xmalloc((l-1)*sizeof(ISTInterval *));
		printf("<<< Printing the elements of the IST: %p\n",S);
		if (S != NULL)
			/* CAUTION : l is the number of layer */
			st_Write(S->Root, path, 0L, l);
		xfree(path);
		printf(">>>\n");
	} else
		puts("Empty IST");
}


/*
 * From the IST S, produce a vector of pointers pointing to the intervals
 * of the first element in S. Redundant w/ GiveMeAPath
 */
ISTInterval **ist_firstpath2array(S)
	ISTSharingTree *S;
{
	ISTNode *node;
	size_t i, length;
	ISTInterval **Sol;

	length = ist_nb_layers(S)-1;
	Sol = (ISTInterval **)xmalloc(length*sizeof(ISTInterval *));

	node=S->Root;
	for (i = 0; i < length; ++i) {
		Sol[i]=node->FirstSon->Son->Info;
		node=node->FirstSon->Son;
	}
	return Sol;
}

ISTHeadListNode *NoProject(ISTNode *node,ISTSharingTree *STR, ISTLayer *rlayer, int nlayer, integer16 *mask);
ISTNode *YesProject(ISTNode *node,ISTSharingTree *STR, ISTLayer *rlayer, int nlayer, integer16 *mask)
{
	ISTSon *s;
	ISTNode *rchild;
	ISTNode *rnode;
	ISTHeadListNode * list;
	ISTHeadListNode * list_tmp;
	TMemo1 *memo;

	if (ist_equal_interval(node->Info,&IST_end_of_list))
		rnode = ist_add_node(rlayer, ist_create_node(&IST_end_of_list));
	else {
		ist_init_list_node(&list_tmp);
		rnode = ist_create_node(node->Info);
		rlayer = rlayer->Next;
		if (rlayer == NULL)
			rlayer = ist_add_last_layer(STR);

		for(s=node->FirstSon;s != NULL;s=s->Next){
			if (mask[nlayer+1] == 0) {
				list = NoProject(s->Son,STR,rlayer,nlayer+1,mask);
				while(ist_is_empty_list_node(list) == false)
					ist_insert_list_node_without_redundancy(list_tmp,
							ist_remove_first_elem_list_node(list));
				xfree(list);
			} else {
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

ISTHeadListNode *NoProject(ISTNode *node,ISTSharingTree *STR, ISTLayer *rlayer, int nlayer, integer16 *mask)
{
	ISTSon *s;
	ISTNode *n;
	ISTHeadListNode *list, *list_tmp;
	ISTLayer *layer = rlayer;
	TMemo1 *memo;
	int l;

	ist_init_list_node(&list);
	ist_insert_list_node(list,node);
	l=nlayer;

	ist_new_magic_number();

	while(mask[l+1] == 0) {

//		printf("l=%d\n",l);

		ist_init_list_node(&list_tmp);
		for(n = ist_remove_first_elem_list_node(list); n != NULL;n = ist_remove_first_elem_list_node(list)) {
			for(s = n->FirstSon;s!=NULL;s=s->Next) {
				if (s->Son->AuxI != ist_get_magic_number()) {
					s->Son->AuxI = ist_get_magic_number();
					ist_insert_list_node(list_tmp,s->Son);
				}
			}
		}
		xfree(list);
		list = list_tmp;
		l++;
	}

	ist_init_list_node(&list_tmp);

	for(n = ist_remove_first_elem_list_node(list); n != NULL;n = ist_remove_first_elem_list_node(list)) {
		for(s = n->FirstSon;s!=NULL;s=s->Next) {
			memo = ist_get_memoization1(s->Son, s->Son);
			if (memo != NULL)
				ist_insert_list_node(list_tmp,memo->r);
			else
				ist_insert_list_node(list_tmp,YesProject(s->Son,STR,layer,l+1,mask));

		}
	}

	xfree(list);
	return list_tmp;
}


/*
 *
 * we make the assumption that we keep the last layer
 *
 */
ISTSharingTree *ist_projection(ISTSharingTree * S, integer16 *mask) {
	ISTSharingTree * STR;
	ISTLayer * rlayer;
	ISTSon * s;
	ISTHeadListNode * list;
	ISTHeadListNode * list_tmp;
	boolean noprojection;
	int i;

	ist_new(&STR);
	i=0;
	noprojection=true;
	while (i<ist_nb_layers(S)-1 && noprojection==true) {
		noprojection=(mask[i]==0) ? true : false;
		i++;
	}
	if (ist_is_empty(S) == false && noprojection==false) {
		ist_new_magic_number();
		ist_new_memo1_number();
		ist_init_list_node(&list_tmp);
		rlayer = ist_add_last_layer(STR);
		for(s = S->Root->FirstSon; s != NULL;s = s->Next) {
			if (mask[0] > 0)
				ist_add_son(STR->Root,YesProject(s->Son,STR,rlayer,0,mask));
			else {
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
		if(!ist_is_empty(STR))
			ist_normalize(STR);

	}
	return STR;
}

static ISTNode *Downward_closure(node, LINK)
	ISTNode *node;
	struct LOC_ist_method  *LINK;
{
	ISTSon *s;
	ISTNode *rnode;
	ISTInterval i;

	if (ist_not_equal_interval(node->Info,&IST_end_of_list)) {
		i.Left = 0;
		i.Right = node->Info->Right;
		rnode = ist_create_node(&i);
		node->AuxI = ist_get_magic_number();
		LINK->rlayer = LINK->rlayer->Next;
		if (LINK->rlayer == NULL)
			LINK->rlayer = ist_add_last_layer(LINK->STR);
		s = node->FirstSon;
		while (s != NULL) {
			if (s->Son->AuxI == ist_get_magic_number())
				ist_add_son(rnode, s->Son->AuxP);
			else
				ist_add_son(rnode, Downward_closure(s->Son, LINK));
			s = s->Next;
		}
		LINK->rlayer = LINK->rlayer->Previous;
	} else {
		rnode = ist_create_node(node->Info);
		node->AuxI = ist_get_magic_number();
	}
	node->AuxP = ist_add_node(LINK->rlayer, rnode);
	return (node->AuxP);
}


/* This function returns the copy of the IST provided in parameter */
ISTSharingTree *ist_downward_closure(ST)
	ISTSharingTree *ST;
{
	struct LOC_ist_method  V;
	ISTSon *s;
	ISTNode *rchild;

	ist_new_magic_number();
	ist_new(&V.STR);
	if (!ist_is_empty(ST)) {
		V.rlayer = ist_add_last_layer(V.STR);
		s = ST->Root->FirstSon;
		while (s != NULL) {
			rchild = Downward_closure(s->Son, &V);
			ist_add_son(V.STR->Root, rchild);
			s = s->Next;
		}
	}
	V.STR->NbElements = ST->NbElements;
	return V.STR;
}

long ist_nb_tuples(ST)
   ISTSharingTree *ST;
{
   ISTLayer *layer;
   ISTNode *node;
   ISTSon *s;
   int size_interval;

   if (ist_is_empty(ST))
       return 0;
   else {
       layer = ST->LastLayer;
       while (layer != NULL) {
           node = layer->FirstNode;
            while (node != NULL) {
                if (ist_equal_interval(node->Info,&IST_end_of_list))
                   node->AuxI = 1;
               else {
                   node->AuxI = 0;
                   s = node->FirstSon;
                   size_interval = ist_add_value(ist_sub_value(node->Info->Right,node->Info->Left),1);
                   while (s != NULL) {
                       node->AuxI += s->Son->AuxI * size_interval;
                       s = s->Next;
                   }
               }
               node = node->Next;
           }
           layer = layer->Previous;
       }
       ST->Root->AuxI = 0;
       s = ST->Root->FirstSon;
       while (s != NULL) {
           ST->Root->AuxI += s->Son->AuxI;
           s = s->Next;
       }
       return ST->Root->AuxI;
   }
}
