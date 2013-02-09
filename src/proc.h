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

   Copyright 2003, 2004, Pierre Ganty
 */


#ifndef __PROC_H
#define __PROC_H

#include "def.h"
#include "interval.h"
#include <stddef.h>

/*
 * This file contains the definition of the IST data structure as well as very
 * elementary function to manipulate their components and functions to manage
 * the allocation of the memory. In [Zam97], you have all the explainations
 * about these low level functions.
 */

/*steps for memory booking*/
#define st_StepInfoMemory 20000
#define st_StepSonsMemory  15000
#define st_StepNodesMemory  5000
#define st_StepLayersMemory  1000
#define st_StepTreesMemory  500

/*bounds*/
#define st_MaxNbrMemos1  127
#define st_MaxNbrMemos2  127
#define st_MaxLoadMemos1  15000
#define st_MaxLoadMemos2  15000

/* Definition of the ISTSharingTree Data Structure */
typedef struct ISTSharingTree {
	struct ISTLayer *FirstLayer, *LastLayer;
	struct ISTNode *Root;
	integer32 NbElements;
	struct ISTSharingTree *Next;		/* So far, never used in our algorithms */
} ISTSharingTree;

typedef struct ISTLayer {
	struct ISTNode *FirstNode, *LastNode;
	struct ISTLayer *Previous, *Next;
} ISTLayer;

typedef struct ISTNode {
	/* The three following fields are part of the definition of the data-structure */
	ISTInterval *Info;
	integer16 NbFathers, NbSons;
	struct ISTSon *FirstSon, *FirstFather;
	struct ISTNode *Next;

	/* The follwing fields are used by peculiar algorithms or for peculiar purposes (e.g. AuxI for memoization) */
	integer32 AuxI, Mark;
	struct ISTNode *AuxP;
	struct ISTSon *BackRel, *Rel;
	integer32 MinUp, MinDown,MaxUp, MaxDown;
} ISTNode;

typedef struct ISTSon {
	ISTNode *Son;
	struct ISTSon *Next;
} ISTSon;


/*
 * Definition of the data structure used for the memoization mecanism.
 * The memoization mecanism is defined in the Zampunieris's thesis p.56
 */
typedef struct TMemo1 {
	integer8 op;
	ISTNode *n1, *n2, *r;
	struct TMemo1 *next;
} TMemo1;


typedef struct TMemo2 {
	integer8 op;
	ISTNode *n1, *n2, *r1, *r2;
	struct TMemo2 *next;
} TMemo2;



void ist_dispose_info(ISTInterval *interv);
void ist_dispose_son(ISTSon *son) ;
void ist_dispose_node(ISTNode *node) ;
void ist_dispose_layer(ISTLayer *layer) ;
void ist_dispose_tree(ISTSharingTree *shar3) ;
void ist_dispose_shar3(ISTSharingTree *ST) ;
void ist_dispose_shar3s() ;
void ist_book_info_memory() ;
void ist_get_info_memory() ;
void ist_book_sons_memory() ;
void ist_get_sons_memory() ;
void ist_book_nodes_memory() ;
void ist_get_nodes_memory() ;
void ist_book_layers_memory() ;
void ist_get_layers_memory() ;
void ist_book_trees_memory() ;
void ist_get_trees_memory() ;
ISTInterval *ist_new_info() ;
ISTSon *ist_new_son() ;
ISTNode *ist_new_node() ;
ISTLayer *ist_new_layer() ;
ISTSharingTree *ist_new_tree() ;
void ist_count_elements(ISTSharingTree *ST) ;
void ist_init_memory() ;
void ist_new_magic_number() ;
integer32 ist_get_magic_number() ;
void ist_init_system();
void ist_new(ISTSharingTree **ST) ;
void ist_dispose(ISTSharingTree *ST) ;
void ist_init_memoization() ;
void ist_empty_memoization1() ;
void ist_new_memo1_number() ;
void ist_empty_memoization2() ;
void ist_new_memo2_number() ;
void ist_put_memoization1(ISTNode *node1, ISTNode *node2, ISTNode *result) ;
TMemo1 *ist_get_memoization1(ISTNode *node1, ISTNode *node2) ;
void ist_put_memoization2(ISTNode *node1, ISTNode *node2, ISTNode *result1, ISTNode *result2) ;
TMemo2 *ist_get_memoization2(ISTNode *node1, ISTNode *node2) ;
void ist_add_son(ISTNode *node, ISTNode *child) ;
void ist_remove_son(ISTNode *node, ISTNode *child) ;
void ist_replace_son(ISTNode *node, ISTNode *oldchild, ISTNode *newchild) ;
void ist_copy_sons(ISTNode *orgnode, ISTNode *tgtnode) ;
boolean ist_has_son(ISTNode *Father, ISTNode *N) ;
ISTNode *ist_has_son_with_value(ISTNode *node, ISTInterval *value) ;
boolean ist_same_sons(ISTNode *node1, ISTNode *node2) ;
boolean ist_contains_sons(ISTNode *node, ISTNode *nodep) ;
void ist_remove_sons(ISTNode *node) ;
int ist_number_of_sons(ISTNode *node) ;
ISTNode *ist_create_node(ISTInterval* value) ;
void ist_remove_node(ISTLayer *layer, ISTNode *node) ;
ISTNode *ist_exists_node(ISTLayer *layer, ISTNode *node) ;
ISTNode *ist_add_node(ISTLayer *layer, ISTNode *node) ;
void ist_add_node_star(ISTLayer *layer, ISTNode *node) ;
ISTLayer *ist_add_first_layer(ISTSharingTree *ST) ;
ISTLayer *ist_add_last_layer(ISTSharingTree *ST) ;
void ist_remove_last_layer(ISTSharingTree *ST) ;

#endif
