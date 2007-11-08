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

   Copyright 2003, 2004, Pierre Ganty
 */


#ifndef __TRANSTREE_H
#define __TRANSTREE_H

#include "proc.h"
#include "interval.h"

/* Here i haven't yet apply the naming convention .... perhaps afterwards */
typedef struct TransSharingTree {
    struct TransLayer *FirstLayer, *LastLayer;
    struct TransNode *Root;
	integer32 NbElements;
    struct TransSharingTree *Next;
} TransSharingTree;

typedef struct TransLayer {
    struct TransNode *FirstNode, *LastNode;
    struct TransLayer *Previous, *Next;
} TransLayer;

typedef struct Transition_t {
    ISTInterval Guard;
    long Delta;
}Transition_t;

typedef struct TransNode {
    struct Transition_t Info;
    integer16 NbFathers;
    struct TransSon *FirstSon;
    integer32 AuxI;
    struct TransNode *AuxP, *Next;
    /*New field*/
    struct TransSon *BackRel, *Rel, *FirstFather;
    /*other new field*/
    integer32 Mark, MinUp, MinDown;
    /*computation of simulation firstmethod*/
    integer32 NbSons;
    /*computation of simulation second method*/
    struct TransSon *AuxP2;

} TransNode;

typedef struct TransSon {
    TransNode *Son;
    struct TransSon *Next;
} TransSon;




// Declaration of structures

/*  variables for ist_add: */
struct LOC_TransSTAdd {
    TransSharingTree *ST;
    Transition_t* Info;
    integer16 LInfo, depth;
    Transition_t v;
    TransLayer *layer;
    TransNode *nodep;
    boolean newelement;
} ;




TransSon *Trans_st_NewSon() ;
void Trans_st_DisposeSon(TransSon *son) ;
void Trans_st_AddSon(TransNode *node, TransNode *child) ;
TransNode *Trans_st_HasSon(TransNode *node, Transition_t value) ;
void Trans_st_CopySons(TransNode *orgnode, TransNode *tgtnode) ;
void Trans_st_ReplaceSon(TransNode *node, TransNode *oldchild, TransNode *newchild) ;
boolean Trans_st_SameSons(TransNode *node1, TransNode *node2) ;
void Trans_st_RemoveSons(TransNode *node) ;
TransNode *Trans_st_NewNode() ;
void Trans_st_DisposeNode(TransNode *node) ;
void Trans_st_RemoveNode(TransLayer *layer, TransNode *node) ;
TransNode *Trans_st_ExistsNode(TransLayer *layer, TransNode *node) ;
TransNode *Trans_st_AddNode(TransLayer *layer, TransNode *node) ;
TransNode *Trans_st_CreateNode(Transition_t value) ;
TransLayer *Trans_st_NewLayer() ;
void Trans_st_DisposeLayer(TransLayer *layer) ;
TransLayer *Trans_st_AddLasTransLayer(TransSharingTree *ST) ;
void Trans_st_DeleteLasTransLayer(TransSharingTree *ST) ;
/*
   For the memoization mecanism, we use the hashtable of ST with casting
   of pointers ... it works perfectly

   TMemo1 *Trans_st_GetMemoization1(TransNode *node1, TransNode *node2) ;
   void Trans_st_PutMemoization1(TransNode *node1, TransNode *node2, TransNode *result) ;
 */
TransSharingTree *Trans_st_NewTree() ;
void Trans_st_DisposeTree(TransSharingTree *shar3) ;
void Trans_st_DisposeShar3(TransSharingTree *ST) ;
void TransSTNew(TransSharingTree **ST) ;
void TransSTDispose(TransSharingTree *ST) ;
TransNode *Trans_Add(TransNode *node, boolean inlayer, boolean mustchange, struct LOC_TransSTAdd *LINK) ;
boolean TransSTAdd(TransSharingTree *ST_, Transition_t *Info_, integer16 LInfo_) ;



void TransSTWriteElem(Transition_t **path,long l);
void Trans_st_Write(TransNode *N,Transition_t **path,long i,long l);
void TransSTWrite(TransSharingTree *S,long l);
#endif 
