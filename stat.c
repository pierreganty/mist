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

#include "stat.h"
#include "basis.h"
#include <stdlib.h>
#include <stdio.h>

static size_t ist_count_number_of_arcs_layer(Layer)
    ISTLayer *Layer;
{
	ISTNode *Node;
	size_t Sol;

	Node = Layer->FirstNode;
	Sol = 0;
	while (Node != NULL) {
		Sol += ist_number_of_sons(Node);
		Node = Node->Next;
	}
	return Sol;
}


static size_t ist_count_number_of_arcs(S)
    ISTSharingTree *S;
{
	size_t Sol;
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
    size_t NbArcs;

    NbArcs = ist_count_number_of_arcs(S);
    printf("Elems: %7d Nodes: %5d Arcs: %5d\n",
	    ist_nb_elements(S), ist_nb_nodes(S), NbArcs);
}

