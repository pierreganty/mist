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

#include <stdlib.h>
#include "initialmarking.h"
#include "transsystem.h"
#include "interval.h"
#include "basis.h"

void ist_is_initial_marking_reached_node(ValueNode, Node)
	ISTInterval *ValueNode;
	ISTNode *Node;
{
	ISTSon *Son;
	ISTInterval *is_reached;

	if (Node->Mark == ist_get_magic_number()){
		Son = Node->FirstSon;
		while (Son != NULL) {
			is_reached = ist_intersect_intervals(Son->Son->Info,ValueNode);
			if (is_reached != NULL) {
				Son->Son->Mark = ist_get_magic_number();
				ist_dispose_info(is_reached);
			}
			Son = Son->Next;
		}
	}
}


void ist_is_initial_marking_reached_layer(ValueLayer, Layer)
	ISTInterval *ValueLayer;
	ISTLayer *Layer;
{
  ISTNode *Node;

  Node = Layer->FirstNode;
  while (Node != NULL) {
    ist_is_initial_marking_reached_node(ValueLayer, Node);
    Node = Node->Next;
  }
}


/*
 * We assume S and InitialMarkingM have the same height 
 */
boolean ist_is_initial_marking_reached(InitialMarkingM, S)
	ISTSharingTree *InitialMarkingM, *S;
{
	ISTLayer *Layer;
	ISTNode *Node;
	size_t NuLayer, height;
	boolean Sol = false;
	ISTInterval  **InitialMarking;

	if (ist_is_empty(S) == false) {
		ist_new_magic_number();
		height = ist_nb_layers(S)-1;
		InitialMarking = ist_path2array(InitialMarkingM);
		S->Root->Mark = ist_get_magic_number();
		ist_is_initial_marking_reached_node(InitialMarking[0], S->Root);
		Layer = S->FirstLayer;
		NuLayer = 1;
		while (NuLayer < height) {
			ist_is_initial_marking_reached_layer(InitialMarking[NuLayer], Layer);
			Layer = Layer->Next;
			NuLayer++;
		}
		Node = S->LastLayer->Previous->FirstNode;
		while (Node != NULL) {
			if (Node->Mark == ist_get_magic_number())
				Sol = true;
			Node = Node->Next;
		}
	}
	return Sol;
}

