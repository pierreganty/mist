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

   Copyright 2003, 2004, Pierre Ganty, 2015, Pedro Valero
 */

#include "checkup.h"
#include "basis.h"
#include "error.h"
#include <stddef.h>
#include <stdio.h>

static boolean first_cond_is_violated(ISTSharingTree *S);
static boolean second_cond_is_violated(ISTSharingTree *S);
static boolean same_pointer(ISTSon *S);
static boolean exists_irrellevant_info(ISTSharingTree *S);
static boolean exists_layers_not_ordered(ISTSharingTree *S);
static boolean exists_sons_not_ordered(ISTSharingTree *S);
static boolean has_father(ISTNode *N,ISTLayer *Layer);
static boolean exists_node_without_father_in_layer(ISTLayer *Layer);
static boolean exists_node_without_father(ISTSharingTree *S);
static boolean exists_son_twice(ISTSharingTree *S);
static boolean mismatch_in_counting_fathers(ISTSharingTree *S);

static boolean first_cond_is_violated(S)
    ISTSharingTree *S;
/* See point 5 Definition 7 p 5 of [DRV02] */
{
    boolean violated;
    ISTLayer *Layer;
    ISTSon *Son, *OtherSon;
    ISTNode *Node;
    integer32 num_layer;

    violated = false;
    Node = S->Root;
    Son = Node->FirstSon;
    while (Son != NULL && !violated) {
	OtherSon = Son->Next;
	while (OtherSon != NULL && !violated) {
	    if (ist_equal_interval(OtherSon->Son->Info,Son->Son->Info)) {
		violated = true;
		printf("In the first layer: Two equal sons with value[%4ld,%4ld]\n"
			, Son->Son->Info->Left,Son->Son->Info->Right);
	    } else
		OtherSon = OtherSon->Next;
	}
	Son = Son->Next;
    }
    if (!violated) {
	num_layer = 1;
	Layer = S->FirstLayer;
	while (Layer != NULL && !violated) {
	    Node = Layer->FirstNode;
	    while (Node != NULL && !violated) {
		Son = Node->FirstSon;
		while (Son != NULL && !violated) {
		    OtherSon = Son->Next;
		    while (OtherSon != NULL && !violated) {
			if (ist_equal_interval(OtherSon->Son->Info,Son->Son->Info)) {
			    violated = true;
			    printf("Two equal sons with value[%4ld,%4ld] at layer %4ld\n"
				    , Son->Son->Info->Left,Son->Son->Info->Right,num_layer);
			} else
			    OtherSon = OtherSon->Next;
		    }
		    Son = Son->Next;
		}
		Node = Node->Next;
	    }
	    Layer = Layer->Next;
	    num_layer++;
	}
    }
    return violated;
}


static boolean second_cond_is_violated(S)
/* See point 6 Definition 7 p 5 of [DRV02] */
    ISTSharingTree *S;
{
    boolean violated;
    ISTLayer *Layer;
    ISTNode *Node, *OtherNode;

    violated = false;
    Layer = S->FirstLayer;
    while (Layer != NULL && !violated) {
	Node = Layer->FirstNode;
	while (Node != NULL && !violated) {
	    OtherNode = Node->Next;
	    while (OtherNode != NULL && !violated) {
		if (ist_equal_interval(OtherNode->Info,Node->Info) && ist_same_sons(OtherNode, Node))
		    violated = true;
		else
		    OtherNode = OtherNode->Next;
	    }
	    Node = Node->Next;
	}
	Layer = Layer->Next;
    }
    return violated;
}


static boolean same_pointer(S)
    ISTSon *S;
{
    ISTSon *T;
    boolean stop;

    stop = false;
    while (S != NULL && !stop) {
	T = S->Next;
	while (T != NULL && !stop) {
	    if (T->Son == S->Son)
		stop = true;
	    else
		T = T->Next;

	}
	S = S->Next;
    }
    return stop;
}


static boolean exists_irrellevant_info(S)
    ISTSharingTree *S;
{
    ISTLayer *Layer;
    ISTNode *Node;
    boolean stop;

    stop = false;
    Layer = S->FirstLayer;
    while (Layer != S->LastLayer && !stop) {
	Node = Layer->FirstNode;
	while (Node != NULL && !stop) {
	    if (Node->Info->Left < 0
			|| (Node->Info->Right != INFINITY && Node->Info->Right < 0)
			|| (Node->Info->Right != INFINITY && Node->Info->Left > Node->Info->Right)){
		stop = true;
		printf("[%3ld,%3ld]",Node->Info->Left,Node->Info->Right);
	    }
	    Node = Node->Next;
	}
	Layer = Layer->Next;
    }
    return stop;
}

static boolean exists_layers_not_ordered(S)
    ISTSharingTree *S;
{
    ISTLayer *Layer;
    ISTNode *Node, *M;
    boolean stop;

    stop = false;
    Layer = S->FirstLayer;
    while (Layer != S->LastLayer && !stop) {
	Node = Layer->FirstNode;
	M = Node->Next;
	while (M != NULL && !stop) {
	    if (ist_greater_interval(Node->Info,M->Info)){
		stop = true;
		printf("n-->[%3ld,%3ld]\t n+1-->[%3ld,%3ld]\n"
			,Node->Info->Left,Node->Info->Right,M->Info->Left,M->Info->Right);
	    }
	    Node = M;
	    M = M->Next;
	}
	Layer = Layer->Next;
    }
    return stop;
}


static boolean exists_sons_not_ordered(S)
    ISTSharingTree *S;
{
    ISTLayer *Layer;
    ISTNode *Node;
    ISTSon *Son, *OSon;
    boolean stop;

    stop = false;
    Layer = S->FirstLayer;
    while (Layer != S->LastLayer && !stop) {
	Node = Layer->FirstNode;
	while (Node != NULL && !stop) {
	    Son = Node->FirstSon;
	    while (Son != NULL && !stop) {
		OSon = Son->Next;
		while (OSon != NULL && !stop) {
		    if (ist_greater_interval(Son->Son->Info,OSon->Son->Info))
			stop = true;
		    else
			OSon = OSon->Next;
		}
		Son = Son->Next;
	    }
	    Node = Node->Next;
	}
	Layer = Layer->Next;
    }
    return stop;
}


static boolean has_father(N, Layer)
    ISTNode *N;
    ISTLayer *Layer;
{
    ISTNode *Q;
    ISTSon *S;
    boolean found;

    found = false;
    Q = Layer->FirstNode;
    while (Q != NULL && !found) {
	S = Q->FirstSon;
	while (S != NULL && !found) {
	    if (S->Son == N)
		found = true;
	    else
		S = S->Next;
	}
	Q = Q->Next;
    }
    return found;
}


static boolean exists_node_without_father_in_layer(Layer)
    ISTLayer *Layer;
{
    ISTNode *Node;
    boolean stop;

    stop = false;
    Node = Layer->FirstNode;
    while (Node != NULL && !stop) {
	if (!has_father(Node, Layer->Previous))
	    stop = true;
	else
	    Node = Node->Next;
    }
    return stop;
}

static boolean exists_node_without_father(S)
    ISTSharingTree *S;
{
    ISTLayer *Layer;
    boolean stop;

    stop = false;
    Layer = S->FirstLayer->Next;
    while (Layer != S->LastLayer && !stop) {
	if (exists_node_without_father_in_layer(Layer))
	    stop = true;
	else {
	    Layer = Layer->Next;
	}
    }
    return stop;
}

static boolean exists_node_without_son(S)
    ISTSharingTree *S;
{
    ISTLayer *Layer;
    ISTNode *Node;
    boolean stop;

    stop = false;
    Layer = S->FirstLayer;
    while (Layer != S->LastLayer && !stop) {
	Node = Layer->FirstNode;
	while (Node != NULL && !stop) {
	    if (Node->FirstSon == NULL)
		stop = true;
	    Node = Node->Next;
	}
	Layer = Layer->Next;
    }
    return stop;
}


static boolean exists_son_twice(S)
    ISTSharingTree *S;
{
    ISTLayer *Layer;
    ISTNode *Node;
    boolean stop;

    stop = false;
    Layer = S->FirstLayer;
    while (Layer != S->LastLayer && !stop) {
	Node = Layer->FirstNode;
	while (Node != NULL && !stop) {
	    if (same_pointer(Node->FirstSon))
		stop = true;
	    else
		Node = Node->Next;
	}
	Layer = Layer->Next;
    }
    return stop;
}



static boolean mismatch_in_counting_fathers(S)
    ISTSharingTree *S;
{
    ISTLayer *Layer, *NLayer;
    ISTNode *CurN, *OtherN;
    ISTSon *CurS;
    boolean stop;
    integer32 count;

    CurN = S->FirstLayer->FirstNode;
    stop = false;
    while (CurN != NULL && !stop) {
	count = 0;
	CurS = S->Root->FirstSon;
	while (CurS != NULL) {
	    if (CurS->Son == CurN)
		count++;
	    CurS = CurS->Next;
	}
	if (count != CurN->NbFathers)
	    stop = true;
	else
	    CurN = CurN->Next;
    }
    if (!stop){
	Layer = S->FirstLayer;
	NLayer = Layer->Next;
	while (Layer != S->LastLayer && !stop) {
	    CurN = NLayer->FirstNode;
	    while (CurN != NULL && !stop) {
		OtherN = Layer->FirstNode;
		count = 0;
		while (OtherN != NULL) {
		    CurS = OtherN->FirstSon;
		    while (CurS != NULL) {
			if (CurS->Son == CurN)
			    count++;
			CurS = CurS->Next;
		    }
		    OtherN = OtherN->Next;
		}
		if (count != CurN->NbFathers) {
		    stop = true;
		} else
		    CurN = CurN->Next;
	    }
	    Layer = Layer->Next;
	    NLayer = Layer->Next;
	}
    }
    return stop;
}


boolean ist_checkup(S)
    ISTSharingTree *S;
{
	boolean retval=true;
	if(ist_is_empty(S)==false) {
		if(S->Root->Info!=&IST_beg_of_list && S->LastLayer->FirstNode->Info!=&IST_end_of_list){
			err_msg("Error: Root or Sink node mislabelled!\n");
			retval=false;
		}
		if (first_cond_is_violated(S)) {
			err_msg("Error: First Cond violated!\n");
			retval=false;
		}
		if (second_cond_is_violated(S)) {
			err_msg("Error: Second Cond violated!\n");
			retval=false;
		}
		if (exists_layers_not_ordered(S)) {
			err_msg("Error: Non-ordered nodes in a layer!\n");
			retval=false;
		}
		if (exists_irrellevant_info(S)) {
			err_msg("Error: Irrelevant info in node!\n");
			retval=false;
		}
		if (exists_sons_not_ordered(S)) {
			err_msg("Error: Non-ordered sons!\n");
			retval=false;
		}
		if (exists_node_without_son(S)) {
			err_msg("Error: Node without sons!\n");
			retval=false;
		}
		if (exists_node_without_father(S)) {
			err_msg("Error: Son without father!\n");
			retval=false;
		}
		if (exists_son_twice(S)) {
			err_msg("Error: A node with two pointers to the same node!\n");
			retval=false;
		}
		if (mismatch_in_counting_fathers(S)) {
			err_msg("Error: NbFathers not handled properly!\n");
			retval=false;
		}
	}
	ist_stat(S);
	return retval;
}



static int ist_count_number_of_arcs_layer(Layer)
    ISTLayer *Layer;
{
	ISTNode *Node;
	int Sol;

	Node = Layer->FirstNode;
	Sol = 0;
	while (Node != NULL) {
		Sol += ist_number_of_sons(Node);
		Node = Node->Next;
	}
	return Sol;
}


static int ist_count_number_of_arcs(S)
    ISTSharingTree *S;
{
	int Sol;
	ISTLayer *Layer;

	Sol = ist_number_of_sons(S->Root);
	Layer = S->FirstLayer;
	while (Layer != S->LastLayer) {
		Sol += ist_count_number_of_arcs_layer(Layer);
		Layer = Layer->Next;
	}
	return Sol;
}


/*print statistics about the sharing tree S*/
void ist_stat(S)
    ISTSharingTree *S;
{
    int NbArcs;

    NbArcs = ist_count_number_of_arcs(S);
    printf("Elems: %7ld Nodes: %5d Arcs: %5d Layers: %2d\n",
	    ist_nb_elements(S), ist_nb_nodes(S), NbArcs, ist_nb_layers(S));
}

void ist_stat_plot(S, f)
    ISTSharingTree *S;
    FILE *f;
{
    if(f!=NULL) fprintf(f, "%7ld",ist_nb_elements(S));
}
