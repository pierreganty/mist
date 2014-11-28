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

#ifndef __COMPUTESIM_H
#define __COMPUTESIM_H


#include "proc.h"
#include "def.h"


/*
 * This modules is in charge to build the simulation relation between nodes of two sharing trees
 * (it can be twice the same). Algorithms implememted in this modules are directly taken from :
 * Monika R. Henzinger, Thomas A. Henzinger, and Peter Kopke.
 * Computing Simulations in Finite and Infinite Graphs.
 * Proceedings of the 36th Annual IEEE Symposium on Foundations of Computer Science (FOCS 1995), pp. 453-462.
 */
void STWriteRel(ISTSharingTree *S) ;
void STWriteBackRel(ISTSharingTree *S) ;
void DisposeRel(ISTNode *N) ;
void DisposeBackRel(ISTNode *N) ;
void DisposeInfoRel(ISTSharingTree *S) ;
void DisposeInfoBackRel(ISTSharingTree *S) ;
void DisposeInfoRelAndBackRel(ISTSharingTree *S) ;

/*compute for each layer of S the forward simulation */
void ComputeForwardSimulation( ISTSharingTree *S);
/*compute for each layer of S the backward simulation */
void ComputeBackwardSimulation( ISTSharingTree *S);
/* Compute the forward simulation between the nodes of a sharing tree S and a sharing tree T */
void ComputeFSimul2( ISTSharingTree *S, ISTSharingTree *T);
/*Compute the backward simulation between the nodes of a sharing tree S and a sharing tree T*/
void ComputeBSimul2( ISTSharingTree *S, ISTSharingTree *T);
#endif
