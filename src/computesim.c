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

#include "computesim.h"
#include "basis.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <time.h>

#define MatriceLenght 						7000

/* Matrix used for the computation of the simulation relation */
typedef struct BigStructure {
	long BiggestValueUsed;
	long MaxDimUsed;
	long Tab[MatriceLenght][MatriceLenght] ;
}BigStructure;

/* We declare it locally */
static BigStructure Matrice;


static void InitMatriceSimulation()
{
	Matrice.BiggestValueUsed = 0;
	Matrice.MaxDimUsed = 1;
}


/*initialize th 0 all the element i,j of the matrice used to compute backward*/
/*and forward simulation with i and j in [DimMin,DimMax] */
static void InitMatrice(DimMin, DimMax)
	long DimMin, DimMax;
{
	size_t Abs, Ord;
	for (Abs = DimMin ; Abs < DimMax; Abs++) {
		for (Ord = DimMin; Ord < DimMax; Ord++)
			Matrice.Tab[Abs][Ord] = 0;
	}
}

/*initialize temporary data inside the nodes of Layer*/
static integer16 InitializeNodeFromLayer(Layer)
	ISTLayer *Layer;
{
	ISTNode *NodeLayer;
	long NbNode;

	NodeLayer = Layer->FirstNode;
	NbNode = 0;
	while (NodeLayer != NULL) {
		NbNode++;
		NodeLayer->AuxI = NbNode;
		NodeLayer->Mark = 0;
		NodeLayer = NodeLayer->Next;
	}
	return NbNode;
}

void DisposeRel(N)
	ISTNode *N;
{
	ISTSon *s, *sn;

	s = N->Rel;
	while (s != NULL) {
		sn = s->Next;
		ist_dispose_son(s);
		s = sn;
	}
	N->Rel = NULL;
}

static void WriteRel(L, P)
	ISTSon *L;
	ISTNode *P;
{
	while (L != NULL) {
		printf("[%12ld,%12ld]", L->Son->Info->Left,L->Son->Info->Right);
		if (L->Son == P)
			printf(" Itself\n");
		else
			printf(" \n");
		L = L->Next;
	}
}

void STWriteRel(S)
	ISTSharingTree *S;
{
	ISTNode *Node;
	ISTLayer *Layer;
	long d;

	Layer = S->FirstLayer;
	d = 1;

	while (Layer != S->LastLayer) {
		printf("----- Depth %12ld ----- \n", d);
		Node = Layer->FirstNode;
		while (Node != NULL) {
			printf("Node [%12ld,%12ld] simulated by \n", Node->Info->Left,Node->Info->Right);
			WriteRel(Node->Rel, Node);
			Node = Node->Next;
		}
		Layer = Layer->Next;
		d++;
	}
	printf("Root simulated by \n");
	WriteRel(S->Root->Rel, S->Root);
}

static void AddNodeToRelNotOrdered(Node, RelNode)
	ISTNode *Node, *RelNode;
{
	ISTSon *NewElem;

	NewElem = ist_new_son();
	NewElem->Son = RelNode;
	NewElem->Next = Node->Rel;
	Node->Rel = NewElem;
}


static void AddNodeToBackRelNotOrdered(Node, BackRelNode)
	ISTNode *Node, *BackRelNode;
{
	ISTSon *NewElem;

	NewElem = ist_new_son();
	NewElem->Son = BackRelNode;
	NewElem->Next = Node->BackRel;
	Node->BackRel = NewElem;
}

static void AddNodeToRel(node, relnode)
	ISTNode *node, *relnode;
{
	ISTSon *s, *sp;
	boolean stop;

	s = ist_new_son();
	s->Son = relnode;
	if (node->Rel == NULL) {
		s->Next = NULL;
		node->Rel = s;
		return;
	}
	if (ist_greater_or_equal_interval(node->Rel->Son->Info,s->Son->Info)) {
		s->Next = node->Rel;
		node->Rel = s;
		return;
	}
	sp = node->Rel;
	stop = false;
	while (!stop) {
		if (sp->Next == NULL) {
			stop = true;
		}else {
			if (ist_greater_or_equal_interval(sp->Next->Son->Info,s->Son->Info))
				stop = true;
			else
				sp = sp->Next;
		}
	}
	s->Next = sp->Next;
	sp->Next = s;
}


static void AddNodeToBackRel(node, relnode)
	ISTNode *node, *relnode;
{
	ISTSon *s, *sp;
	boolean stop;

	s = ist_new_son();
	s->Son = relnode;
	if (node->BackRel == NULL) {
		s->Next = NULL;
		node->BackRel = s;
		return;
	}
	if (ist_greater_interval(node->BackRel->Son->Info,s->Son->Info)) {
		s->Next = node->BackRel;
		node->BackRel = s;
		return;
	}
	sp = node->BackRel;
	stop = false;
	while (!stop) {
		if (sp->Next == NULL) {
			stop = true;
		}else {
			if (ist_greater_interval(sp->Next->Son->Info,s->Son->Info))
				stop = true;
			else
				sp = sp->Next;
		}
	}
	s->Next = sp->Next;
	sp->Next = s;
}


void STWriteBackRel(S)
	ISTSharingTree *S;
{
	ISTNode *Node;
	ISTLayer *Layer;
	long d;

	Layer = S->FirstLayer;
	d = 1;

	while (Layer != S->LastLayer) {
		printf("----- Depth %12ld ----- \n", d);
		Node = Layer->FirstNode;
		while (Node != NULL) {
			printf("Node [%12ld,%12ld] simulated by \n", Node->Info->Left,Node->Info->Right);
			WriteRel(Node->BackRel, Node);
			Node = Node->Next;
		}
		Layer = Layer->Next;
		d++;
	}
	printf("Root simulated by \n");
	WriteRel(S->Root->BackRel, S->Root);
}


void DisposeBackRel(N)
	ISTNode *N;
{
	ISTSon *s, *sn;

	s = N->BackRel;
	while (s != NULL) {
		sn = s->Next;
		ist_dispose_son(s);
		s = sn;
	}
	N->BackRel = NULL;
}


void DisposeInfoRel(S)
	ISTSharingTree *S;
{
	ISTLayer *Layer;
	ISTNode *N;

	DisposeRel(S->Root);
	Layer = S->FirstLayer;
	while (Layer != NULL) {
		N = Layer->FirstNode;
		while (N != NULL) {
			DisposeRel(N);
			N = N->Next;
		}
		Layer = Layer->Next;
	}
}


void DisposeInfoBackRel(S)
	ISTSharingTree *S;
{
	ISTLayer *Layer;
	ISTNode *N;

	DisposeBackRel(S->Root);
	Layer = S->FirstLayer;
	while (Layer != NULL) {
		N = Layer->FirstNode;
		while (N != NULL) {
			DisposeBackRel(N);
			N = N->Next;
		}
		Layer = Layer->Next;
	}
}


void DisposeInfoRelAndBackRel(S)
	ISTSharingTree *S;
{
	ISTLayer *Layer;
	ISTNode *N;

	DisposeBackRel(S->Root);
	DisposeRel(S->Root);
	Layer = S->FirstLayer;
	while (Layer != NULL) {
		N = Layer->FirstNode;
		while (N != NULL) {
			DisposeRel(N);
			DisposeBackRel(N);
			N = N->Next;
		}
		Layer = Layer->Next;
	}
}


/*compute for each layer of S the forward simulation */
void ComputeForwardSimulation(S)
	ISTSharingTree *S;
{
	ISTLayer *CurrentLayer, *NextLayer;
	ISTNode *NodeSon;
	ISTSon *FatherSimul, *FatherNode, *NodeSimul, *FirstSimuled;
	long NbNode, Abs, Ord, ZeroForLayer;

	InitMatriceSimulation();
	DisposeInfoRel(S);
	CurrentLayer = S->LastLayer;
	NodeSon = CurrentLayer->FirstNode;
	AddNodeToRel(NodeSon, NodeSon);
	NodeSon->AuxI = 1;
	while (CurrentLayer->Previous != NULL) {
		NextLayer = CurrentLayer;
		CurrentLayer = CurrentLayer->Previous;
		NbNode = InitializeNodeFromLayer(CurrentLayer);
		/*if the matrice is too small*/
		if (NbNode > MatriceLenght) {
			printf("Depasse%12ld\n", NbNode);
			err_quit("ComputeForwardSimulation(S) : Too many nodes on this layer! Be more reasonable!\n");
		} else {
			if (NbNode > Matrice.MaxDimUsed) {
				InitMatrice(Matrice.MaxDimUsed + 1, NbNode);
				Matrice.MaxDimUsed = NbNode;
			}
			ZeroForLayer = Matrice.BiggestValueUsed;
			NodeSon = NextLayer->FirstNode;
			while (NodeSon != NULL) {
				NodeSimul = NodeSon->Rel;
				while (NodeSimul != NULL) {
					FirstSimuled = NodeSon->FirstFather;
					FatherSimul = NodeSimul->Son->FirstFather;
					while (FatherSimul != NULL) {
						if (FatherSimul->Son->Mark != NodeSon->AuxI) {
							/*
							 * If we acess more than once the same FatherSimul  wrt the same NodeSon,
							 * it's not relevant to recompute the simulation relation since we have
							 * already computed it before.
							 */
							FatherSimul->Son->Mark = NodeSon->AuxI;
							FatherNode = FirstSimuled;
							while (FatherNode != NULL) {
								if (FatherNode->Son->Info->Left < FatherSimul->Son->Info->Left)
									/*
									 * If we satisfy this test, we know that we can't have
									 * FatherNode->Son is simulated by FatherSimul->Son. We know also
									 * that by the order on fathers that this FatherNode won't
									 * be simulated by any FatherSimul
									 */
									FirstSimuled = FirstSimuled->Next;
								else {
									if (ist_less_or_equal_value(FatherNode->Son->Info->Right,FatherSimul->Son->Info->Right)){
										/*
										 * We have that FatherNode is simulated by FatherSimul.
										 */
										Abs = FatherSimul->Son->AuxI;
										Ord = FatherNode->Son->AuxI;
										Abs--;
										Ord--;
										if (Matrice.Tab[Abs][Ord] < ZeroForLayer)
											Matrice.Tab[Abs][Ord] = ZeroForLayer + 1;
										else
											Matrice.Tab[Abs][Ord]++;
										Matrice.BiggestValueUsed = max(Matrice.BiggestValueUsed, Matrice.Tab[Abs][Ord]);
										if (Matrice.Tab[Abs][Ord] - ZeroForLayer == FatherNode->Son->NbSons)
											AddNodeToRelNotOrdered(FatherNode->Son, FatherSimul->Son);
									}
								}
								FatherNode = FatherNode->Next;
							}
						}
						FatherSimul = FatherSimul->Next;
					}
					NodeSimul = NodeSimul->Next;
				}
				NodeSon = NodeSon->Next;
			}
		}
	}
	AddNodeToRel(S->Root, S->Root);
}


/*compute for each layer of S the backward simulation */
void ComputeBackwardSimulation(S)
	ISTSharingTree *S;
{
	ISTLayer *CurrentLayer, *PreviousLayer;
	ISTNode *NodeFather;
	ISTSon *SonSimul, *SonNode, *NodeSimul, *FirstSimuled;
	long NbNode, Abs, Ord, ZeroForLayer;
	boolean finished;

	InitMatriceSimulation();
	DisposeInfoBackRel(S);
	CurrentLayer = S->FirstLayer;
	PreviousLayer = S->FirstLayer;
	NodeFather = S->Root;
	AddNodeToBackRel(NodeFather, NodeFather);
	NodeFather->AuxI = 1;
	finished = false;
	while (!finished) {
		NbNode = InitializeNodeFromLayer(CurrentLayer);
		/*if the matrice is too small*/
		if (NbNode > MatriceLenght) {
			printf("Depasse%12ld\n", NbNode);
			err_quit("ComputeForwardSimulation(S) : Too many nodes on this layer! Be more reasonable!\n");
			exit(EXIT_FAILURE);
		} else {
			if (NbNode > Matrice.MaxDimUsed) {
				InitMatrice(Matrice.MaxDimUsed + 1, NbNode);
				Matrice.MaxDimUsed = NbNode;
			}

			ZeroForLayer = Matrice.BiggestValueUsed;
			while (NodeFather != NULL) {
				NodeSimul = NodeFather->BackRel;
				while (NodeSimul != NULL) {
					FirstSimuled = NodeFather->FirstSon;
					SonSimul = NodeSimul->Son->FirstSon;
					while (SonSimul != NULL) {
						if (SonSimul->Son->Mark != NodeFather->AuxI) {
							SonSimul->Son->Mark = NodeFather->AuxI;
							SonNode = FirstSimuled;
							while (SonNode != NULL) {
								if (SonNode->Son->Info->Left < SonSimul->Son->Info->Left)
									/*
									 * If we satisfy this test, we know that we can't have
									 * FatherNode->Son is simulated by FatherSimul->Son. We know also
									 * that by the order on fathers that this FatherNode won't
									 * be simulated by any FatherSimul
									 */
									FirstSimuled = FirstSimuled->Next;
								else {
									if (ist_less_or_equal_value(SonNode->Son->Info->Right,SonSimul->Son->Info->Right)){
										/*
										 * We have that FatherNode is simulated by FatherSimul.
										 */
										Abs = SonSimul->Son->AuxI;
										Ord = SonNode->Son->AuxI;
										Abs--;
										Ord--;
										if (Matrice.Tab[Abs][Ord] < ZeroForLayer)
											Matrice.Tab[Abs][Ord] = ZeroForLayer + 1;
										else
											Matrice.Tab[Abs][Ord]++;
										Matrice.BiggestValueUsed = max(Matrice.BiggestValueUsed,
												Matrice.Tab[Abs][Ord]);
										if (Matrice.Tab[Abs][Ord] - ZeroForLayer == SonNode->Son->NbFathers)
											AddNodeToBackRelNotOrdered(SonNode->Son, SonSimul->Son);
									}
								}
								SonNode = SonNode->Next;
							}
						}
						SonSimul = SonSimul->Next;
					}
					NodeSimul = NodeSimul->Next;
				}
				NodeFather = NodeFather->Next;
			}
		}
		PreviousLayer = CurrentLayer;
		CurrentLayer = CurrentLayer->Next;
		if (CurrentLayer == NULL)
			finished = true;
		else
			NodeFather = PreviousLayer->FirstNode;
	}

}

/*Compute the forward simulation between the nodes of a sharing tree S and a sharing tree T*/
void ComputeFSimul2(T, S)
	ISTSharingTree *T, *S;
{
	ISTNode *NodeSon;
	ISTLayer *CurrentLayerS, *CurrentLayerT;
	long NbNodeS, NbNodeT, NbNodeMax, Abs, Ord, ZeroForLayer;
	ISTSon *FatherSimul, *NodeSimul, *FatherNode, *FirstSimuled;

	InitMatriceSimulation();
	DisposeInfoRel(S);
	CurrentLayerS = S->LastLayer;
	CurrentLayerS->FirstNode->AuxI = 1;
	CurrentLayerT = T->LastLayer;
	NodeSon = CurrentLayerS->FirstNode;
	AddNodeToRel(NodeSon, CurrentLayerT->FirstNode);
	while (CurrentLayerS->Previous != NULL) {
		NbNodeS = InitializeNodeFromLayer(CurrentLayerS->Previous);

		NbNodeT = InitializeNodeFromLayer(CurrentLayerT->Previous);
		NbNodeMax = max(NbNodeS, NbNodeT);
		if (NbNodeMax > MatriceLenght) {
			printf("depasse%12ld\n", NbNodeMax);
		} else {
			if (NbNodeMax > Matrice.MaxDimUsed) {
				InitMatrice(Matrice.MaxDimUsed + 1, NbNodeMax);
				Matrice.MaxDimUsed = NbNodeMax;
			}
			ZeroForLayer = Matrice.BiggestValueUsed;
			NodeSon = CurrentLayerS->FirstNode;
			while (NodeSon != NULL) {
				NodeSimul = NodeSon->Rel;
				while (NodeSimul != NULL) {
					FirstSimuled = NodeSon->FirstFather;
					FatherSimul = NodeSimul->Son->FirstFather;
					while (FatherSimul != NULL) {
						if (FatherSimul->Son->Mark != NodeSon->AuxI) {
							FatherSimul->Son->Mark = NodeSon->AuxI;
							/*the fathers are ordered. If a father of a node don't simule a father of a*/
							/*other node, then the next father don't simulate this father too*/
							FatherNode = FirstSimuled;
							while (FatherNode != NULL) {
								if (ist_include_interval(FatherNode->Son->Info,
											FatherSimul->Son->Info))
									FirstSimuled = FirstSimuled->Next;
								else {
									Abs = FatherSimul->Son->AuxI;
									Ord = FatherNode->Son->AuxI;
									if (Matrice.Tab[Abs - 1][Ord - 1] < ZeroForLayer)
										Matrice.Tab[Abs - 1][Ord - 1] = ZeroForLayer + 1;
									else
										Matrice.Tab[Abs - 1][Ord - 1]++;
									Matrice.BiggestValueUsed = max(Matrice.BiggestValueUsed,
											Matrice.Tab[Abs - 1][Ord - 1]);
									if (Matrice.Tab[Abs - 1][Ord - 1] - ZeroForLayer ==
											FatherNode->Son->NbSons)
										AddNodeToRelNotOrdered(FatherNode->Son, FatherSimul->Son);
								}
								FatherNode = FatherNode->Next;
							}
						}
						FatherSimul = FatherSimul->Next;
					}
					NodeSimul = NodeSimul->Next;
				}
				NodeSon = NodeSon->Next;
			}
		}
		CurrentLayerS = CurrentLayerS->Previous;
		CurrentLayerT = CurrentLayerT->Previous;
	}
}


/*Compute the backward simulation between the nodes of a sharing tree S and a sharing tree T*/
void ComputeBSimul2(T, S)
	ISTSharingTree *T, *S;
{
	ISTNode *NodeFather;
	ISTLayer *CurrentLayerS, *PreviousLayerS, *CurrentLayerT;
	boolean Finished;
	long NbNodeS, NbNodeT, NbNodeMax, ZeroForLayer, Abs, Ord;
	ISTSon *NodeSimul, *FirstSimuled, *SonSimul, *SonNode;

	InitMatriceSimulation();
	DisposeInfoBackRel(S);
	NodeFather = S->Root;
	S->Root->AuxI = 1;
	T->Root->AuxI = 1;
	Finished = false;
	AddNodeToBackRel(NodeFather, T->Root);
	CurrentLayerS = S->FirstLayer;
	CurrentLayerT = T->FirstLayer;
	while (!Finished) {
		NbNodeS = InitializeNodeFromLayer(CurrentLayerS);
		NbNodeT = InitializeNodeFromLayer(CurrentLayerT);
		NbNodeMax = max(NbNodeS, NbNodeT);
		if (NbNodeMax > MatriceLenght) {
			if (CurrentLayerS == S->FirstLayer) {
				printf("Depasse%12ld\n", NbNodeMax);
			}

		} else {
			if (NbNodeMax > Matrice.MaxDimUsed) {
				InitMatrice(Matrice.MaxDimUsed + 1, NbNodeMax);
				Matrice.MaxDimUsed = NbNodeMax;
			}
			ZeroForLayer = Matrice.BiggestValueUsed;
			while (NodeFather != NULL) {
				NodeSimul = NodeFather->BackRel;
				while (NodeSimul != NULL) {
					FirstSimuled = NodeFather->FirstSon;
					SonSimul = NodeSimul->Son->FirstSon;
					while (SonSimul != NULL) {
						if (SonSimul->Son->Mark != NodeFather->AuxI) {
							SonSimul->Son->Mark = NodeFather->AuxI;
							/*the fathers are ordered. If a father of a node don't simule a father of a*/
							/*other node, then the next father don't simulate this father too*/
							SonNode = FirstSimuled;
							while (SonNode != NULL) {
								if (ist_include_interval(SonNode->Son->Info, SonSimul->Son->Info))
									FirstSimuled = FirstSimuled->Next;
								else {
									Abs = SonSimul->Son->AuxI;
									Ord = SonNode->Son->AuxI;
									if (Matrice.Tab[Abs - 1][Ord - 1] < ZeroForLayer)
										Matrice.Tab[Abs - 1][Ord - 1] = ZeroForLayer + 1;
									else
										Matrice.Tab[Abs - 1][Ord - 1]++;
									Matrice.BiggestValueUsed = max(Matrice.BiggestValueUsed,
											Matrice.Tab[Abs - 1][Ord - 1]);
									if (Matrice.Tab[Abs - 1][Ord - 1] - ZeroForLayer ==
											SonNode->Son->NbFathers)
										AddNodeToBackRelNotOrdered(SonNode->Son, SonSimul->Son);
								}
								SonNode = SonNode->Next;
							}
						}
						SonSimul = SonSimul->Next;
					}
					NodeSimul = NodeSimul->Next;
				}
				NodeFather = NodeFather->Next;
			}
		}
		PreviousLayerS = CurrentLayerS;
		CurrentLayerS = CurrentLayerS->Next;
		if (CurrentLayerS == NULL) {
			Finished = true;
			break;
		}
		CurrentLayerT = CurrentLayerT->Next;
		NodeFather = PreviousLayerS->FirstNode;
	}
}


