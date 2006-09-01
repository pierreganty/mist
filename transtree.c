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

#include "transtree.h"
#include "xmalloc.h"
#include <stdlib.h>
#include <stdio.h>

TransSon *Trans_st_NewSon(){
    /* xmalloc ... */
    TransSon *res = (TransSon *)xmalloc(sizeof(TransSon));
    return res;
}


void Trans_st_DisposeSon(TransSon *son){
    xfree(son);
} 

void Trans_st_AddSon(TransNode *node, TransNode *child)
    /* On peut optimiser en ne triant pas pdt l'insertion, pq parce que personne ne peut utiliser l'info triee c trop complique pour applyrule */
{
    TransSon *s, *sp, *sq;
    boolean stop;

    s = Trans_st_NewSon();
    s->Son = child;
    child->NbFathers++;
    /* On peut placer le child (= s) en tete de liste car la liste est vide */
    if (node->FirstSon == NULL) {
	s->Next = NULL;
	node->FirstSon = s;
	return;
    }
    /* On peut placer le child (= s) en tete car il est semantiquement avant  node->FirstSon */
    if (ist_greater_interval(&node->FirstSon->Son->Info.Guard,&s->Son->Info.Guard) 
	    || (ist_equal_interval(&node->FirstSon->Son->Info.Guard,&s->Son->Info.Guard)
	    && (node->FirstSon->Son->Info.Delta >= s->Son->Info.Delta))){
	s->Next = node->FirstSon;
	node->FirstSon = s;
	return;
    }

    /* sinon on est dans le cas tout a fait general, on sait cependant que notre liste contient au moins
       un element et que ce premier elem respecte ist_less_or_equal_interval(FirstSon,child) */
    sp = node->FirstSon;
    sq = NULL; 
    stop = false;
    /* On va tenter de se placer correctement dans la liste des sons */
    while (sp != NULL && !stop) {
	if (ist_greater_interval(&sp->Son->Info.Guard,&s->Son->Info.Guard)) 
	    stop = true;
	else {
	    if (ist_equal_interval(&sp->Son->Info.Guard,&s->Son->Info.Guard)) { 
		if (sp->Son->Info.Delta >= s->Son->Info.Delta)
		    stop = true;
	    } 
	    if (!stop){
		sq = sp;
		sp = sp->Next;
	    }
	}
    }
    if (sp != NULL) {
	s->Next = sp;
	sq->Next=s;
	 
    } else {
	/* On insere en fin de liste si necessaire */
	s->Next = NULL;
	sq->Next=s;
    }
}
 
TransNode *Trans_st_HasSon(TransNode *node, Transition value)
{
    TransNode *Result;
    TransSon *s;
    boolean stop;

    Result = NULL;
    s = node->FirstSon;
    stop = false;
    while (s != NULL && !stop) {
	if (ist_greater_interval(&s->Son->Info.Guard,&value.Guard))
	    s = NULL;
	else {
	    if (ist_equal_interval(&s->Son->Info.Guard,&value.Guard)) {
		if (s->Son->Info.Delta > value.Delta)
		    s = NULL;
		else {
		    if (s->Son->Info.Delta == value.Delta)
			stop = true;
		    else
			s = s->Next;
		}
	    } else 
		s = s->Next;
	}
    }
    if (stop == true)
	Result = s->Son;
    return Result;
}

void Trans_st_CopySons(TransNode *orgnode, TransNode *tgtnode) 
{
    TransSon *orgson, *tgtson, *lastson;

    orgson = orgnode->FirstSon;
    lastson = NULL;
    while (orgson != NULL) {
	tgtson = Trans_st_NewSon();
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

void Trans_st_ReplaceSon(TransNode *node, TransNode *oldchild, TransNode *newchild) 
{
    TransSon *s;

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

boolean Trans_st_SameSons(TransNode *node1, TransNode *node2) 
{
    boolean Result, stop;
    TransSon *s1, *s2;

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
void Trans_st_RemoveSons(TransNode *node) 
{
    TransSon *s, *sn;

    s = node->FirstSon;
    while (s != NULL) {
	sn = s->Next;
	s->Son->NbFathers--;
	Trans_st_DisposeSon(s);
	s = sn;
    }
    node->FirstSon = NULL;
}

TransNode *Trans_st_NewNode() {
/* xmalloc ... */
    TransNode *res = (TransNode *)xmalloc(sizeof(TransNode));
    return res;
}

void Trans_st_DisposeNode(TransNode *node) {
    xfree(node);
}

void Trans_st_RemoveNode(TransLayer *layer, TransNode *node) 
{
    TransNode *nodep;

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
    Trans_st_RemoveSons(node);
    Trans_st_DisposeNode(node);
}

TransNode *Trans_st_ExistsNode(TransLayer *layer, TransNode *node) 
{
    TransNode *Result, *nodep;

    if (layer->FirstNode == NULL)
	return NULL;
    if (ist_greater_interval(&layer->FirstNode->Info.Guard,&node->Info.Guard)
	    || (ist_equal_interval(&layer->FirstNode->Info.Guard,&node->Info.Guard)
	    && layer->FirstNode->Info.Delta > node->Info.Delta ))
	return NULL;
    if (ist_less_interval(&layer->LastNode->Info.Guard,&node->Info.Guard)
	    || (ist_equal_interval(&layer->LastNode->Info.Guard,&node->Info.Guard)
	    && layer->LastNode->Info.Delta < node->Info.Delta))
	return NULL;
    Result = NULL;

    nodep = layer->FirstNode;
    while ( nodep != NULL && Result == NULL ) {
	if (ist_greater_interval(&nodep->Info.Guard,&node->Info.Guard) 
	    || (ist_equal_interval(&nodep->Info.Guard,&node->Info.Guard)
	    && nodep->Info.Delta > node->Info.Delta ))
	    nodep = NULL;
	else if (ist_less_interval(&nodep->Info.Guard,&node->Info.Guard)
	    || (ist_equal_interval(&nodep->Info.Guard,&node->Info.Guard)
	    && nodep->Info.Delta < node->Info.Delta ))
	    nodep = nodep->Next;	
	else {
	    if(ist_equal_interval(&nodep->Info.Guard,&node->Info.Guard)) {
		if (nodep == node)
		    nodep = nodep->Next;
		else {
		    if (nodep->Info.Delta == node->Info.Delta 
			    && Trans_st_SameSons(nodep, node)) {
			Result = nodep;
			nodep = NULL;
		    } else
			if (nodep->Info.Delta <= node->Info.Delta)
			    nodep = nodep->Next;
			else
			    nodep = NULL;
		}
	    }
	}
    }
    return Result;
}
TransNode *Trans_st_AddNode(TransLayer *layer, TransNode *node) 
{
    TransNode  *nodep, *nodeq;
    boolean stop;
    /* Si le layer est vide, l'ajout est trivial */
    if (layer->FirstNode == NULL) {
	layer->FirstNode = node;
	layer->LastNode = node;
	return node;
    }
    /* On va voir si on ne peut pas placer node en tete de liste */
    if (ist_greater_interval(&layer->FirstNode->Info.Guard,&node->Info.Guard)
	    || (ist_equal_interval(&layer->FirstNode->Info.Guard,&node->Info.Guard)
	    && layer->FirstNode->Info.Delta > node->Info.Delta )){
	node->Next = layer->FirstNode;
	layer->FirstNode = node;
	return node;
    }  
    /* On va voir si on ne peut pas placer node en fin de liste */
    if (ist_less_interval(&layer->LastNode->Info.Guard,&node->Info.Guard)
	    || (ist_equal_interval(&layer->LastNode->Info.Guard,&node->Info.Guard)
	    && layer->LastNode->Info.Delta < node->Info.Delta)){
	layer->LastNode->Next = node;
	layer->LastNode = node;
	return node;
    }
    /* sinon on est dans le cas tout a fait general */
    nodep = layer->FirstNode;
    nodeq = NULL;
    stop = false;
    while (nodep != NULL && ist_less_or_equal_interval(&nodep->Info.Guard,&node->Info.Guard) && !stop){
	/* Attention au cas particulier evoquer plus bas */
	if (ist_equal_interval(&nodep->Info.Guard,&node->Info.Guard)) { 
	    if ( nodep->Info.Delta == node->Info.Delta){
		/* We check for the validity of the condition 6 of [DRV02] */
		if (Trans_st_SameSons(node, nodep)) {
		    Trans_st_RemoveSons(node);
		    Trans_st_DisposeNode(node);
		    return nodep;
		}
	    } else {
		if (nodep->Info.Delta > node->Info.Delta)
		    stop = true;
	    }
	}
	if (!stop) {
	    nodeq = nodep;
	    nodep = nodep->Next;
	}
    }
    node->Next = nodep;
    nodeq->Next = node;

    if (layer->LastNode->Next != NULL)
	    /* Dans le cas ou la liste contient une fin de nodep avec nodep.info = node.info mais not ist_same_sons */	
	layer->LastNode = node;
    return node; 
}
TransNode *Trans_st_CreateNode(Transition value) 
{
    TransNode *node;

    node = Trans_st_NewNode();
    node->Info.Guard = value.Guard;
    node->Info.Delta = value.Delta;
    node->NbFathers = 0;
    node->FirstSon = NULL;
    if (ist_equal_interval(&node->Info.Guard,&IST_end_of_list))
	/* Attention de ne pas faire l'appel avec value.Guard */
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
TransLayer *Trans_st_NewLayer() {
/* xmalloc ... */
    TransLayer* res = (TransLayer *)xmalloc(sizeof(TransLayer));
    return res;
}
void Trans_st_DisposeLayer(TransLayer *layer) {
    xfree(layer);
}
TransLayer *Trans_st_AddLastLayer(TransSharingTree *ST) 
{
    TransLayer *layer;

    layer = Trans_st_NewLayer();
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
void Trans_st_DeleteLastLayer(TransSharingTree *ST) 
{
    TransLayer *temp;

    temp = ST->LastLayer;
    if (ST->FirstLayer == ST->LastLayer) {
	ST->FirstLayer = NULL;
	ST->LastLayer = NULL;
    } else {
	ST->LastLayer->Previous->Next = NULL;
	ST->LastLayer = ST->LastLayer->Previous;
    }
    Trans_st_DisposeLayer(temp);
}

TransSharingTree *Trans_st_NewTree() {
/* Faire du xmalloc pour newtree */
    TransSharingTree *res = (TransSharingTree *)xmalloc(sizeof(TransSharingTree));
    return res;
}
void Trans_st_DisposeTree(TransSharingTree *shar3) {
    xfree(shar3);
}
void Trans_st_DisposeShar3(TransSharingTree *ST) 
{
    TransLayer *layer, *nxlayer;
    TransNode *node, *nxnode;
    TransSon *son, *nxson;

    son = ST->Root->FirstSon;
    while (son != NULL) {
	nxson = son->Next;
	Trans_st_DisposeSon(son);
	son = nxson;
    }
    Trans_st_DisposeNode(ST->Root);
    layer = ST->FirstLayer;
    while (layer != NULL) {
	node = layer->FirstNode;
	while (node != NULL) {
	    son = node->FirstSon;
	    while (son != NULL) {
		nxson = son->Next;
		Trans_st_DisposeSon(son);
		son = nxson;
	    }
	    nxnode = node->Next;
	    Trans_st_DisposeNode(node);
	    node = nxnode;
	}
	nxlayer = layer->Next;
	Trans_st_DisposeLayer(layer);
	layer = nxlayer;
    }
    Trans_st_DisposeTree(ST);
}

void TransSTNew(TransSharingTree **ST) 
{
    TransNode *node;
    TransSharingTree *WITH;
    Transition troot;

    troot.Delta = 0;
    troot.Guard = IST_beg_of_list;
    *ST = Trans_st_NewTree();
    node = Trans_st_CreateNode(troot);
    WITH = *ST;
    WITH->FirstLayer = NULL;
    WITH->LastLayer = NULL;
    WITH->Root = node;
    WITH->NbElements = 0;
}

void TransSTDispose(TransSharingTree *ST) {
    Trans_st_DisposeTree(ST);
}

 TransNode *Trans_Add(TransNode *node, boolean inlayer, boolean mustchange, struct LOC_TransSTAdd *LINK) 
{
    TransNode *son, *res;

    if (ist_equal_interval(&node->Info.Guard,&IST_end_of_list)) {
	if (inlayer)
	    return node;
	else
	    return (Trans_st_AddNode(LINK->layer, node));
    } else {
	if (node->NbFathers > 1)
	    mustchange = true;
	LINK->depth++;
	if (LINK->depth < LINK->LInfo)
	    LINK->v = LINK->Info[LINK->depth];
	else {
	    LINK->v.Delta = 0;
	    LINK->v.Guard = IST_end_of_list;
	}
	LINK->layer = LINK->layer->Next;
	if (LINK->layer == NULL)
	    LINK->layer = Trans_st_AddLastLayer(LINK->ST);
	son = Trans_st_HasSon(node, LINK->v);
	if (son == NULL) {
	    LINK->newelement = true;
	    res = Trans_Add(Trans_st_CreateNode(LINK->v), false, false, LINK);
	} else
	    res = Trans_Add(son, true, mustchange, LINK);
	LINK->layer = LINK->layer->Previous;
	if (res == son) {
	    if (inlayer)
		return node;
	    else
		return (Trans_st_AddNode(LINK->layer, node));
	} else {
	    if (mustchange) {
		LINK->nodep = Trans_st_CreateNode(node->Info);
		Trans_st_CopySons(node, LINK->nodep);
		node = LINK->nodep;
		inlayer = false;
	    }
	    if (son == NULL)
		Trans_st_AddSon(node, res);
	    else {
		Trans_st_ReplaceSon(node, son, res);
		if (son->NbFathers == 0)
		    Trans_st_RemoveNode(LINK->layer->Next, son);
	    }
	    if (inlayer) {
		LINK->nodep = Trans_st_ExistsNode(LINK->layer, node);
		if (LINK->nodep == NULL)
		    return node;
		else
		    return LINK->nodep;
	    } else
		return (Trans_st_AddNode(LINK->layer, node));
	}
    }
}

boolean TransSTAdd(TransSharingTree *ST_, Transition *Info_, integer16 LInfo_) 
{
	struct LOC_TransSTAdd V;
	TransNode *son, *res;

	V.ST = ST_;
	V.Info = Info_;
	V.LInfo = LInfo_;
	V.depth = 0;
	V.newelement = false;
	/* depth = a quelle composante j'en suis dans V.Info */
	if (V.depth < V.LInfo)
		V.v = V.Info[V.depth];
	else{
		/* Il faut construire explicitement l'interval */
		V.v.Delta = 0;
		V.v.Guard = IST_end_of_list;
	}
	son = Trans_st_HasSon(V.ST->Root, V.v);
	V.layer = V.ST->FirstLayer;
	if (V.layer == NULL)
		V.layer = Trans_st_AddLastLayer(V.ST);
	if (son == NULL) {
		V.newelement = true;
		Trans_st_AddSon(V.ST->Root, Trans_Add(Trans_st_CreateNode(V.v), false, false, &V));
	} else
		res = Trans_Add(son, true, false, &V);
	if (V.newelement) {
		if (V.ST->NbElements >= 0)   /*I do not remember in which case it could be < 0*/
			V.ST->NbElements++;
	}
	return V.newelement;
}



void TransSTWriteElem(path, l)
    Transition **path;
    long l;
{
    long i;

    if (l <= 0)
	return;
    printf("\n(");
    for (i = 0; i < l-1; i++){
    /* De 0 a l-2 = l-1 elements c'est ce qu'on a cfr 2277 */
	printf("[%3ld", path[i]->Guard.Left);
	printf(",%5ld]", path[i]->Guard.Right);
	printf("Delta = %3ld",path[i]->Delta);
    }  
    printf(" )\n");
}


void Trans_st_Write(N, path, i, l)
    TransNode *N;
    Transition **path;
    long i, l;
{
    TransSon *s;

    if (N == NULL) {
	printf("Error: Found partial path:\n");
	TransSTWriteElem(path, l);
	return;
    }
    if (i >= l)
	TransSTWriteElem(path, l);
    if (i > 0 && i < l)
	/* DE 1 a l-1, du FirstLayer a LastLayer->Previous
	parce qu'on s'en branle du lastlayer */
	path[i-1] = &N->Info;
    s = N->FirstSon;
    while (s != NULL) {
	// printf("NbFathers %12d\n", s->Son->NbFathers);
	Trans_st_Write(s->Son, path, i + 1, l);
	s = s->Next;
    }
}  /* st_Write */


void TransSTWrite(S,l)
    TransSharingTree *S;
    long l;
    /* l est le # de layers du CST, cet info n'est pas dans le CST ... faut la calculer 
       il y a  la fonction ist_nb_layers pour ca */
{
    /* A vector of pointer to intervals */
    Transition **path;
    path = (Transition **)xmalloc((l-1)*sizeof(Transition *));
    printf("<<< Impression des elements d'un TransST d'adr %p\n",S);
    if (S != NULL)
	Trans_st_Write(S->Root, path, 0L, l);
    xfree(path);
    printf(">>>Fin impression de %p\n",S);
}
