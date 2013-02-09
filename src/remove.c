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

#include "remove.h"
#include "basis.h"
#include <stdlib.h>

void ist_remove_node_without_father_layer(Layer)
    ISTLayer *Layer;
{
    ISTNode *Node, *NextNode;

    Node = Layer->FirstNode;
    while (Node != NULL) {
	if (Node->NbFathers == 0) {
	    NextNode = Node->Next;
	    ist_remove_node(Layer, Node);
	    Node = NextNode;
	} else
	    Node = Node->Next;
    }
}


void ist_remove_node_without_father(S)
    ISTSharingTree *S;
{
	ISTLayer *Layer;

	if (ist_is_empty(S)==false) {
		Layer = S->FirstLayer;
		while (Layer != NULL) {
			ist_remove_node_without_father_layer(Layer);
			Layer = Layer->Next;
		}
	}
}


static void RemoveSonWithoutSonNode(Node)
    ISTNode *Node;
{
    ISTSon *Son, *SonNext;

    Son = Node->FirstSon;
    while (Son != NULL) {
	if (Son->Son->FirstSon == NULL) {
	    SonNext = Son->Next;
	    ist_remove_son(Node, Son->Son);
	    Son = SonNext;
	} else
	    Son = Son->Next;
    }
}


static void RemoveSonWithoutSonLayer(Layer)
    ISTLayer *Layer;
{
    ISTNode *Node;

    Node = Layer->FirstNode;
    while (Node != NULL) {
	RemoveSonWithoutSonNode(Node);
	Node = Node->Next;
    }
}


static void RemoveNodeWithoutSonLayer(Layer)
    ISTLayer *Layer;
{
    ISTNode *Node, *NodeNext;

    Node = Layer->FirstNode;
    while (Node != NULL) {
	if (Node->FirstSon == NULL) {
	    NodeNext = Node->Next;
	    ist_remove_node(Layer, Node);
	    Node = NodeNext;
	} else
	    Node = Node->Next;
    }
}


void ist_remove_node_without_son(S)
    ISTSharingTree *S;
{
	ISTLayer *Layer;

	if (ist_is_empty(S)==false) {
		Layer = S->LastLayer->Previous;
		while (Layer != NULL) {
			if (Layer->Previous != NULL)
				RemoveSonWithoutSonLayer(Layer->Previous);
			else
				RemoveSonWithoutSonNode(S->Root);
			RemoveNodeWithoutSonLayer(Layer);
			Layer = Layer->Previous;
		}
		if (S->LastLayer->FirstNode != NULL) {
			if (S->LastLayer->FirstNode->NbFathers == 0)
				ist_remove_node(S->LastLayer, S->LastLayer->FirstNode);
		}
	}
}
