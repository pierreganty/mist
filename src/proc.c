// vim:sw=4:ts=4
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

   Copyright 2003, Pierre Ganty
 */

#include "proc.h"
#include "xmalloc.h"
#include "error.h"
#include <stdlib.h>
#include <stdio.h>

/*
 * Internal define for memory allocation
 */

/*steps for memory booking*/
#define st_StepInfoMemory 20000
#define st_StepSonsMemory  15000
#define st_StepNodesMemory  5000
#define st_StepLayersMemory  1000
#define st_StepTreesMemory  500

/*bounds for memoization*/
#define st_MaxNbrMemos1  127
#define st_MaxNbrMemos2  127
#define st_MaxLoadMemos1  15000
#define st_MaxLoadMemos2  15000

typedef TMemo1 *TMemo1Table[st_MaxNbrMemos1][st_MaxNbrMemos1];
typedef TMemo2 *TMemo2Table[st_MaxNbrMemos2][st_MaxNbrMemos2];

/*
 * Here we defined all the variables by which we try to have an efficient management
 * of the system (e.g. the memory allocation).
 * Their initialization is done through ad hoc function (e.g. through ist_init_system).
 */
/* memories */
static ISTSharingTree *st_TreesMemory;
static ISTLayer *st_LayersMemory;
static ISTNode *st_NodesMemory;
static ISTSon *st_SonsMemory;
static ISTInterval *st_InfoMemory;
static ISTSharingTree *st_Disposed;
/* magic numbers */
static integer32 st_MagicNumber;
static integer8 st_Memo1Number, st_Memo2Number;
/* memoization */
static TMemo1 *(*st_Memo1Table)[st_MaxNbrMemos1];
static TMemo2 *(*st_Memo2Table)[st_MaxNbrMemos2];
/* statistics */
static integer16 st_NbrGetTreesMem, st_NbrGetLayersMem, st_NbrGetNodesMem,
	st_NbrGetSonsMem, st_NbrGetInfoMem;
static integer32 st_MaxMemos1, st_NbrMemos1, st_NbrPutMemo1, st_NbrGetMemo1,
	st_NbrClashes1, st_NbrSuccess1, st_MaxMemos2, st_NbrMemos2,
	st_NbrPutMemo2, st_NbrGetMemo2, st_NbrClashes2,
	st_NbrSuccess2;


inline void ist_dispose_info(interv)
	ISTInterval *interv;
{
	interv->next = st_InfoMemory;
	st_InfoMemory = interv;
}

inline void ist_dispose_son(son)
	ISTSon *son;
{

//	 A sanity check which is VERY TIME CONSUMING !!!
/*	ISTSon *ptr;
	ptr = st_SonsMemory;
	while (ptr != NULL) {
		if (ptr == son)
			err_msg("Error: son already disposed\n");
		ptr = ptr->Next;
	}
*/

	son->Next = st_SonsMemory;
	st_SonsMemory = son;
}

inline void ist_dispose_node(node)
	ISTNode *node;
{
//	 A sanity check which is VERY TIME CONSUMING !!!
/*	ISTNode *ptr;
	ptr = st_NodesMemory;
	while (ptr != NULL) {
		if (ptr == node)
			err_msg("Error: node already disposed\n");
		ptr = ptr->Next;
	}
*/

	if (ist_not_equal_interval(node->Info,&IST_beg_of_list) && ist_not_equal_interval(node->Info,&IST_end_of_list) && node->Info != NULL)
		ist_dispose_info(node->Info);
	node->Next = st_NodesMemory;
	st_NodesMemory = node;
}


inline void ist_dispose_layer(layer)
	ISTLayer *layer;
{
//	 A sanity check which is VERY TIME CONSUMING !!!
/*	ISTLayer *ptr;
	ptr = st_LayersMemory;
	while (ptr != NULL) {
		if (ptr == layer)
			err_msg("Error: layer already disposed\n");
		ptr = ptr->Next;
	}
*/


	layer->Next = st_LayersMemory;
	st_LayersMemory = layer;
}


inline void ist_dispose_tree(shar3)
	ISTSharingTree *shar3;
{
//	 A sanity check which is VERY TIME CONSUMING !!!
/*	ISTSharingTree *ptr;
	ptr = st_TreesMemory;
	while (ptr != NULL) {
		if (ptr == shar3)
			err_msg("Error: ist already disposed\n");
		ptr = ptr->Next;
	}
*/

	shar3->Next = st_TreesMemory;
	st_TreesMemory = shar3;
}


void ist_dispose_shar3(ST)
	ISTSharingTree *ST;
{
	ISTLayer *layer, *nxlayer;
	ISTNode *node, *nxnode;
	ISTSon *son, *nxson;

	son = ST->Root->FirstSon;
	while (son != NULL) {
		nxson = son->Next;
		ist_dispose_son(son);
		son = nxson;
	}
	ist_dispose_node(ST->Root);
	layer = ST->FirstLayer;
	while (layer != NULL) {
		node = layer->FirstNode;
		while (node != NULL) {
			son = node->FirstSon;
			while (son != NULL) {
				nxson = son->Next;
				ist_dispose_son(son);
				son = nxson;
			}
			nxnode = node->Next;
			ist_dispose_node(node);
			node = nxnode;
		}
		nxlayer = layer->Next;
		ist_dispose_layer(layer);
		layer = nxlayer;
	}
	ist_dispose_tree(ST);
}


inline void ist_dispose_shar3s()
{
	ISTSharingTree *nxshar3;

	while (st_Disposed != NULL) {
		nxshar3 = st_Disposed->Next;
		ist_dispose_shar3(st_Disposed);
		st_Disposed = nxshar3;
	}
}


void ist_book_info_memory()
{
	integer16 i;
	ISTInterval *interv;

	st_NbrGetInfoMem++;
	for (i = 1; i <= st_StepInfoMemory; i++) {
		interv = (ISTInterval *)xmalloc(sizeof(ISTInterval));
		interv->next = st_InfoMemory;
		st_InfoMemory = interv;
	}
}

void ist_book_sons_memory()
{
	integer16 i;
	ISTSon *son;

	st_NbrGetSonsMem++;
	for (i = 1; i <= st_StepSonsMemory; i++) {
		son = (ISTSon *)xmalloc(sizeof(ISTSon));
		son->Next = st_SonsMemory;
		st_SonsMemory = son;
	}
}

void ist_get_info_memory()
{
	if (st_Disposed != NULL)
		ist_dispose_shar3s();
	if (st_InfoMemory == NULL)
		ist_book_info_memory();
}

void ist_get_sons_memory()
{
	if (st_Disposed != NULL)
		ist_dispose_shar3s();
	if (st_SonsMemory == NULL)
		ist_book_sons_memory();
}


void ist_book_nodes_memory()
{
	integer16 i;
	ISTNode *node;

	st_NbrGetNodesMem++;
	for (i = 1; i <= st_StepNodesMemory; i++) {
		node = (ISTNode *)xmalloc(sizeof(ISTNode));
		node->Next = st_NodesMemory;
		st_NodesMemory = node;
	}
}


void ist_get_nodes_memory()
{
	if (st_Disposed != NULL)
		ist_dispose_shar3s();
	if (st_NodesMemory == NULL)
		ist_book_nodes_memory();
}


void ist_book_layers_memory()
{
	integer16 i;
	ISTLayer *layer;

	st_NbrGetLayersMem++;
	for (i = 1; i <= st_StepLayersMemory; i++) {
		layer = (ISTLayer *)xmalloc(sizeof(ISTLayer));
		layer->Next = st_LayersMemory;
		st_LayersMemory = layer;
	}
}


void ist_get_layers_memory()
{
	if (st_Disposed != NULL)
		ist_dispose_shar3s();
	if (st_LayersMemory == NULL)
		ist_book_layers_memory();
}


void ist_book_trees_memory()
{
	integer16 i;
	ISTSharingTree *shar3;

	st_NbrGetTreesMem++;
	for (i = 1; i <= st_StepTreesMemory; i++) {
		shar3 = (ISTSharingTree *)xmalloc(sizeof(ISTSharingTree));
		shar3->Next = st_TreesMemory;
		st_TreesMemory = shar3;
	}
}


void ist_get_trees_memory()
{
	if (st_Disposed != NULL)
		ist_dispose_shar3s();
	if (st_TreesMemory == NULL)
		ist_book_trees_memory();
}

ISTInterval *ist_new_info()
{
	ISTInterval *Result;
	if (st_InfoMemory == NULL)
		ist_get_info_memory();
	Result = st_InfoMemory;
	st_InfoMemory = st_InfoMemory->next;
	return Result;
}

ISTSon *ist_new_son()
{
	ISTSon *Result;

	if (st_SonsMemory == NULL)
		ist_get_sons_memory();
	Result = st_SonsMemory;
	st_SonsMemory = st_SonsMemory->Next;
	return Result;
}


ISTNode *ist_new_node()
{
	ISTNode *Result;

	if (st_NodesMemory == NULL)
		ist_get_nodes_memory();
	Result = st_NodesMemory;
	st_NodesMemory = st_NodesMemory->Next;
	return Result;
}


ISTLayer *ist_new_layer()
{
	ISTLayer *Result;

	if (st_LayersMemory == NULL)
		ist_get_layers_memory();
	Result = st_LayersMemory;
	st_LayersMemory = st_LayersMemory->Next;
	return Result;
}


ISTSharingTree *ist_new_tree()
{
	ISTSharingTree *Result;

	if (st_TreesMemory == NULL)
		ist_get_trees_memory();
	Result = st_TreesMemory;
	st_TreesMemory = st_TreesMemory->Next;
	return Result;
}


void ist_init_memory()
{
	st_InfoMemory = NULL;
	st_NbrGetInfoMem = 0;
	ist_book_info_memory();
	st_SonsMemory = NULL;
	st_NbrGetSonsMem = 0;
	ist_book_sons_memory();
	st_NodesMemory = NULL;
	st_NbrGetNodesMem = 0;
	ist_book_nodes_memory();
	st_LayersMemory = NULL;
	st_NbrGetLayersMem = 0;
	ist_book_layers_memory();
	st_TreesMemory = NULL;
	st_NbrGetTreesMem = 0;
	ist_book_trees_memory();
	st_Disposed = NULL;
}


inline void ist_new_magic_number()
{
	st_MagicNumber++;
}

inline integer32 ist_get_magic_number()
{
	return st_MagicNumber;
}

void ist_init_system()
{
	ist_init_memory();
	ist_init_memoization();
	st_MagicNumber = -MaxInt32;
	st_Memo1Number = -MaxInt8;
	st_Memo2Number = -MaxInt8;
}

inline void ist_dispose(ST)
	ISTSharingTree *ST;
{

	if(ST != NULL){
		ST->Next = st_Disposed;
		st_Disposed = ST;
	}
}

void ist_new(ST)
	ISTSharingTree **ST;
{
	ISTNode *node;
	ISTSharingTree *WITH;

	*ST = ist_new_tree();
	node = ist_create_node(&IST_beg_of_list);
	WITH = *ST;
	WITH->FirstLayer = NULL;
	WITH->LastLayer = NULL;
	WITH->Root = node;
	WITH->NbElements = 0;
}

void ist_count_elements(ST)
	ISTSharingTree *ST;
{
	boolean stop;
	ISTLayer *layer;
	ISTNode *node;
	ISTSon *s;

	layer = ST->LastLayer;
	stop = false;
	while (!stop) {
		if (layer == NULL) {
			node = ST->Root;
			stop = true;
		} else
			node = layer->FirstNode;
		while (node != NULL) {
			if (ist_equal_interval(node->Info,&IST_end_of_list))
				node->AuxI = 1;
			else {
				node->AuxI = 0;
				s = node->FirstSon;
				while (s != NULL) {
					node->AuxI += s->Son->AuxI;
					s = s->Next;
				}
			}
			node = node->Next;
		}
		if (layer != NULL)
			layer = layer->Previous;
	}
	ST->NbElements = ST->Root->AuxI;
}


void ist_init_memoization()
{
	size_t i, j;

	st_Memo1Table = (TMemo1 *(*)[st_MaxNbrMemos1])xmalloc(sizeof(TMemo1Table));
	for (i = 0; i < st_MaxNbrMemos1; i++) {
		for (j = 0; j < st_MaxNbrMemos1; j++)
			st_Memo1Table[i][j] = NULL;
	}
	st_MaxMemos1 = 0;
	st_NbrMemos1 = 0;
	st_NbrPutMemo1 = 0;
	st_NbrGetMemo1 = 0;
	st_NbrClashes1 = 0;
	st_NbrSuccess1 = 0;
	st_Memo2Table = (TMemo2 *(*)[st_MaxNbrMemos2])xmalloc(sizeof(TMemo2Table));
	for (i = 0; i < st_MaxNbrMemos2; i++) {
		for (j = 0; j < st_MaxNbrMemos2; j++)
			st_Memo2Table[i][j] = NULL;
	}
	st_MaxMemos2 = 0;
	st_NbrMemos2 = 0;
	st_NbrPutMemo2 = 0;
	st_NbrGetMemo2 = 0;
	st_NbrClashes2 = 0;
	st_NbrSuccess2 = 0;
}


void ist_empty_memoization1()
{
	integer16 i, j;
	TMemo1 *memo, *memop;

	for (i = 0; i < st_MaxNbrMemos1; i++) {
		for (j = 0; j < st_MaxNbrMemos1; j++) {
			memo = st_Memo1Table[i][j];
			while (memo != NULL) {
				memop = memo->next;
				xfree(memo);
				memo = memop;
			}
			st_Memo1Table[i][j] = NULL;
		}
	}
	st_NbrMemos1 = 0;
}


void ist_new_memo1_number()
{
	if (st_NbrMemos1 > st_MaxLoadMemos1) {
		if (st_NbrMemos1 > st_MaxMemos1)
			st_MaxMemos1 = st_NbrMemos1;
		ist_empty_memoization1();
		st_Memo1Number = -MaxInt8;
	}
	st_Memo1Number++;
}


void ist_empty_memoization2()
{
	integer16 i, j;
	TMemo2 *memo, *memop;

	for (i = 0; i < st_MaxNbrMemos2; i++) {
		for (j = 0; j < st_MaxNbrMemos2; j++) {
			memo = st_Memo2Table[i][j];
			while (memo != NULL) {
				memop = memo->next;
				xfree(memo);
				memo = memop;
			}
			st_Memo2Table[i][j] = NULL;
		}
	}
	st_NbrMemos2 = 0;
}


void ist_new_memo2_number()
{
	if (st_NbrMemos2 > st_MaxLoadMemos2) {
		if (st_NbrMemos2 > st_MaxMemos2)
			st_MaxMemos2 = st_NbrMemos2;
		ist_empty_memoization2();
		st_Memo2Number = -MaxInt8;
	}
	st_Memo2Number++;
}


void ist_put_memoization1(node1, node2, result)
	ISTNode *node1, *node2, *result;
{
	integer32 h1, h2;
	TMemo1 *memo;

	h1 = (integer32)node1 % st_MaxNbrMemos1;
	h2 = (integer32)node2 % st_MaxNbrMemos1;
	memo = (TMemo1 *)xmalloc(sizeof(TMemo1));
	memo->n1 = node1;
	memo->n2 = node2;
	memo->r = result;
	memo->op = st_Memo1Number;
	memo->next = st_Memo1Table[h1][h2];
	if (st_Memo1Table[h1][h2] != NULL)
		st_NbrClashes1++;
	st_Memo1Table[h1][h2] = memo;
	st_NbrMemos1++;
	st_NbrPutMemo1++;
}


TMemo1 *ist_get_memoization1(node1, node2)
	ISTNode *node1, *node2;
{
	TMemo1 *Result;
	integer32 h1, h2;
	TMemo1 *memo, *WITH;

	h1 = (integer32)node1 % st_MaxNbrMemos1;
	h2 = (integer32)node2 % st_MaxNbrMemos1;
	Result = NULL;
	memo = st_Memo1Table[h1][h2];
	while (memo != NULL) {
		WITH = memo;
		if (WITH->n1 != node1) {
			memo = memo->next;
			continue;
		}
		if (WITH->n2 != node2) {
			memo = memo->next;
			continue;
		}
		if (WITH->op == st_Memo1Number) {
			Result = memo;
			st_NbrSuccess1++;
			memo = NULL;
		} else
			memo = memo->next;
	}
	st_NbrGetMemo1++;
	return Result;
}


void ist_put_memoization2(node1, node2, result1, result2)
	ISTNode *node1, *node2, *result1, *result2;
{
	integer32 h1, h2;
	TMemo2 *memo;

	h1 = (integer32)node1 % st_MaxNbrMemos2;
	h2 = (integer32)node2 % st_MaxNbrMemos2;
	memo = (TMemo2 *)xmalloc(sizeof(TMemo2));
	memo->n1 = node1;
	memo->n2 = node2;
	memo->r1 = result1;
	memo->r2 = result2;
	memo->op = st_Memo2Number;
	memo->next = st_Memo2Table[h1][h2];
	if (st_Memo2Table[h1][h2] != NULL)
		st_NbrClashes2++;
	st_Memo2Table[h1][h2] = memo;
	st_NbrMemos2++;
	st_NbrPutMemo2++;
}


TMemo2 *ist_get_memoization2(node1, node2)
	ISTNode *node1, *node2;
{
	TMemo2 *Result;
	integer32 h1, h2;
	TMemo2 *memo, *WITH;

	h1 = (integer32)node1 % st_MaxNbrMemos2;
	h2 = (integer32)node2 % st_MaxNbrMemos2;
	Result = NULL;
	memo = st_Memo2Table[h1][h2];
	while (memo != NULL) {
		WITH = memo;
		if (WITH->n1 != node1) {
			memo = memo->next;
			continue;
		}
		if (WITH->n2 != node2) {
			memo = memo->next;
			continue;
		}
		if (WITH->op == st_Memo2Number) {
			Result = memo;
			st_NbrSuccess2++;
			memo = NULL;
		} else
			memo = memo->next;
	}
	st_NbrGetMemo2++;
	return Result;
}

void ist_add_son(node, child)
	ISTNode *node, *child;
{
	ISTSon *s, *sp, *sq;

	s = ist_new_son();
	s->Son = child;
	child->NbFathers++;
	/* We can place 's' in the head of the sons's list because it is empty */
	if (node->FirstSon == NULL) {
		s->Next = NULL;
		node->FirstSon = s;
		return;
	}
	/* We can place 's' before  node->FirstSon because it is logically before (w.r.t. the order that we defined on nodes) */
	if (ist_greater_or_equal_interval(node->FirstSon->Son->Info,s->Son->Info)){
		s->Next = node->FirstSon;
		node->FirstSon = s;
		return;
	}
	/* Otherwise we are in the genreral case. Moreover we know that our list contains at list one element */
	sp = node->FirstSon;
	sq = NULL;
	/* We look for the right place */
	while (sp != NULL && ist_less_interval(sp->Son->Info,s->Son->Info)) {
		/* We browse the list until we find an element  ist_greater_or_equal_interval or sp = NULL (if they are all lesser than 's') */
		sq = sp;
		sp = sp->Next;
	}
	if (sp != NULL) {
		s->Next = sp;
		sq->Next=s;
	} else {
		/* We insert in the tail of the list if necessary */
		s->Next = NULL;
		sq->Next=s;
	}
}



void ist_remove_son(node, child)
	ISTNode *node, *child;
{
	ISTSon *s, *sp;

	sp = NULL;
	s = node->FirstSon;
	while ( s!= NULL && s->Son != child) {
		sp = s;
		s = s->Next;
	}

	if (sp == NULL)
		node->FirstSon = s->Next;
	else
		sp->Next = s->Next;
	ist_dispose_son(s);
	child->NbFathers--;
}


void ist_replace_son(node, oldchild, newchild)
	ISTNode *node, *oldchild, *newchild;
{
	ISTSon *s;

	s = node->FirstSon;
	while (s != NULL) {
		if (s->Son == oldchild) {
			oldchild->NbFathers--;
			s->Son = newchild;
			newchild->NbFathers++;
			s = NULL;
		} else
			s = s->Next;
	}
}


void ist_copy_sons(orgnode, tgtnode)
	ISTNode *orgnode, *tgtnode;
{
	ISTSon *orgson, *tgtson, *lastson;

	orgson = orgnode->FirstSon;
	lastson = NULL;
	while (orgson != NULL) {
		tgtson = ist_new_son();
		tgtson->Son = orgson->Son;
		tgtson->Next = NULL;
		orgson->Son->NbFathers++;
		if (lastson == NULL)
			tgtnode->FirstSon = tgtson;
		else
			lastson->Next = tgtson;
		lastson = tgtson;
		orgson = orgson->Next;
	}
}

boolean ist_has_son(Father, N)
	ISTNode *Father, *N;
{
	/* Returns TRUE if  N is a son of Father, FALSE otherwise */
	boolean found, ok;
	ISTSon *S;

	ok = true;
	found = false;
	S = Father->FirstSon;
	while (S != NULL && !found && ok) {
		if (S->Son == N)
			found = true;
		else if (ist_greater_interval(S->Son->Info,N->Info))
			ok = false;
		S = S->Next;
	}
	return found;
}



ISTNode *ist_has_son_with_value(node, value)
	ISTNode *node;
	ISTInterval *value;
	/* Search a son for node whose label is value */
{
	ISTNode *Result;
	ISTSon *s;

	Result = NULL;
	s = node->FirstSon;
	while (s != NULL && ist_less_interval(s->Son->Info,value)) {
		s = s->Next;
	}
	if (s != NULL){
		if (ist_equal_interval(s->Son->Info,value))
			Result = s->Son;
	}
	return Result;
}


boolean ist_same_sons(node1, node2)
	ISTNode *node1, *node2;
{
	boolean Result, stop;
	ISTSon *s1, *s2;

	Result = true;
	s1 = node1->FirstSon;
	s2 = node2->FirstSon;
	stop = false;
	while ( s1 != NULL && s2 != NULL && !stop ) {
		if (s1-> Son == s2->Son ) {
			s1 = s1->Next;
			s2 = s2->Next;
		} else
			stop = true;
	}
	if ( s1 != NULL || s2 != NULL)
		Result = false;
	return Result;
}


boolean ist_contains_sons(node, nodep)
	ISTNode *node, *nodep;
{
	boolean Result;
	ISTSon *s;

	Result = true;
	s = nodep->FirstSon;
	while (s != NULL) {
		if (ist_has_son_with_value(node, s->Son->Info) == s->Son)
			s = s->Next;
		else {
			Result = false;
			s = NULL;
		}
	}
	return Result;
}


void ist_remove_sons(node)
	ISTNode *node;
{
	ISTSon *s, *sn;

	s = node->FirstSon;
	while (s != NULL) {
		sn = s->Next;
		s->Son->NbFathers--;
		ist_dispose_son(s);
		s = sn;
	}
	node->FirstSon = NULL;
}


int ist_number_of_sons(node)
	ISTNode *node;
{
	int n;
	ISTSon *s;

	n = 0;
	s = node->FirstSon;
	while (s != NULL) {
		n++;
		s = s->Next;
	}
	return n;
}


ISTNode *ist_create_node(value)
	ISTInterval* value;
{
	ISTNode *node;

	node = ist_new_node();
	if (ist_not_equal_interval(value,&IST_end_of_list) && ist_not_equal_interval(value,&IST_beg_of_list))
		node->Info = ist_copy_interval(value);
	else
		node->Info = value;
	node->NbFathers = 0;
	/* For the computesim.c */
	node->NbSons = 0;
	node->FirstSon = NULL;
	if (ist_equal_interval(node->Info,&IST_end_of_list))
		node->AuxI = 1;
	else
		node->AuxI = 0;
	node->AuxP = NULL;
	/* Rel */
	node->Rel = NULL;
	node->FirstFather = NULL;
	node->BackRel = NULL;

	node->Next = NULL;
	return node;
}


void ist_remove_node(layer, node)
	ISTLayer *layer;
	ISTNode *node;
{
	ISTNode *nodep;

	if (layer->FirstNode == node) {
		if (layer->LastNode == node) {
			layer->FirstNode = NULL;
			layer->LastNode = NULL;
		} else
			layer->FirstNode = node->Next;
	} else {
		nodep = layer->FirstNode;
		while (nodep->Next != node)
			nodep = nodep->Next;
		if (layer->LastNode == node) {
			nodep->Next = NULL;
			layer->LastNode = nodep;
		} else
			nodep->Next = node->Next;
	}
	ist_remove_sons(node);
	ist_dispose_node(node);
}


ISTNode *ist_exists_node(layer, node)
	ISTLayer *layer;
	ISTNode *node;
{
	ISTNode *Result, *nodep;

	if (layer->FirstNode == NULL)
		return NULL;
	if (ist_greater_interval(layer->FirstNode->Info,node->Info))
		return NULL;
	if (ist_less_interval(layer->LastNode->Info,node->Info))
		return NULL;
	Result = NULL;

	nodep = layer->FirstNode;
	while ( nodep != NULL && Result == NULL ) {
		if (ist_greater_interval(nodep->Info,node->Info))
			nodep = NULL;
		else if (ist_less_interval(nodep->Info,node->Info))
			nodep = nodep->Next;
		else {
			if (nodep == node)
				nodep = nodep->Next;
			else {
				if (ist_same_sons(nodep, node)) {
					Result = nodep;
					nodep = NULL;
				} else
					nodep = nodep->Next;
			}
		}
	}
	return Result;
}


inline ISTNode *ist_add_node(layer, node)
	ISTLayer *layer;
	ISTNode *node;
{
	ISTNode  *nodep, *nodeq;
	if (layer->FirstNode == NULL) {
		layer->FirstNode = node;
		layer->LastNode = node;
	} else if (ist_greater_interval(layer->FirstNode->Info,node->Info)){
		node->Next = layer->FirstNode;
		layer->FirstNode = node;
	} else if (ist_less_interval(layer->LastNode->Info,node->Info)){
		layer->LastNode->Next = node;
		layer->LastNode = node;
	} else {
		nodep = layer->FirstNode;
		nodeq = NULL;
		while (nodep != NULL && ist_less_or_equal_interval(nodep->Info,node->Info)){
			if (ist_equal_interval(nodep->Info,node->Info)){
				if (ist_same_sons(node, nodep)) {
					ist_remove_sons(node);
					ist_dispose_node(node);
					return nodep;
				}
			}
			nodeq = nodep;
			nodep = nodep->Next;
		}
		node->Next = nodep;
		nodeq->Next = node;
		if (layer->LastNode->Next != NULL)
			layer->LastNode = node;
	}
	return node;
}

void ist_add_node_star(layer, node)
	ISTLayer *layer;
	/*
	 * Same as ist_add_node but do not care at respecting
	 * the rules imposed by the definition of an IST.
	 */
	ISTNode *node;
{
	ISTNode *nodep, *nodeq;

	if (layer->FirstNode == NULL) {
		layer->FirstNode = node;
		layer->LastNode = node;
	} else if (ist_greater_or_equal_interval(layer->FirstNode->Info,node->Info)) {
		node->Next = layer->FirstNode;
		layer->FirstNode = node;
	} else if (ist_less_or_equal_interval(layer->LastNode->Info,node->Info)) {
		layer->LastNode->Next = node;
		layer->LastNode = node;
	} else {
		nodep = layer->FirstNode;
		nodeq = NULL;
		while (nodep != NULL && ist_less_interval(nodep->Info,node->Info)){
			nodeq = nodep;
			nodep = nodep->Next;
		}
		node->Next = nodep;
		nodeq->Next = node;
	}
}


ISTLayer *ist_add_first_layer(ST)
	ISTSharingTree *ST;
{
	ISTLayer *layer;

	layer = ist_new_layer();
	layer->FirstNode = NULL;
	layer->LastNode = NULL;
	layer->Previous = NULL;
	layer->Next = ST->FirstLayer;
	if (ST->FirstLayer == NULL) {
		ST->FirstLayer = layer;
		ST->LastLayer = layer;
	} else {
		ST->FirstLayer->Previous = layer;
		ST->FirstLayer = layer;
	}
	return layer;
}


ISTLayer *ist_add_last_layer(ST)
	ISTSharingTree *ST;
{
	ISTLayer *layer;

	layer = ist_new_layer();
	layer->FirstNode = NULL;
	layer->LastNode = NULL;
	layer->Previous = ST->LastLayer;
	layer->Next = NULL;
	if (ST->LastLayer == NULL) {
		ST->FirstLayer = layer;
		ST->LastLayer = layer;
	} else {
		ST->LastLayer->Next = layer;
		ST->LastLayer = layer;
	}
	return layer;
}


void ist_remove_last_layer(ST)
	ISTSharingTree *ST;
{
	ISTLayer *temp;

	temp = ST->LastLayer;
	if (ST->FirstLayer == ST->LastLayer) {
		ST->FirstLayer = NULL;
		ST->LastLayer = NULL;
	} else {
		ST->LastLayer->Previous->Next = NULL;
		ST->LastLayer = ST->LastLayer->Previous;
	}
	ist_dispose_layer(temp);
}
