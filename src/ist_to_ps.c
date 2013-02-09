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

#include "ist_to_ps.h"
#include <stdio.h>
#include "basis.h"

static void rec_parc(FILE * f, ISTNode *n)
{
    ISTSon *p=n->FirstSon ;
    if (n->AuxI != -1)
    {	
	fprintf(f, "node%p [shape=box, label=\"%ld,%ld\"];\n",n,n->Info->Left,n->Info->Right) ;
	n->AuxI = -1 ; 
	/* We mark nodes because we could encounter them several times but we want to print them only once */
	while(p)
	{
	    if (p->Son)
	    {
		fprintf(f, "node%p -> node%p ;\n", n, p->Son) ;
		rec_parc(f, p->Son) ;
	    }

	    p=p->Next ;
	}
    }
    fprintf(f, "\n\n") ; 
}

/*
	From the IST T,
	void ist_dump_graph(char * filename, ISTSharingTree * T)
	produces a file at the dot format. From the dot file
	you can obtain a graphical representation of T.
	For instance, to get a postscript file, under
	the shell write the command :
   	dot <dot file> -o<postscript file> -Tps

	the dot tools is part of the Graphiz tool suite that
	you can find freely at the URL :
	http://www.research.att.com/sw/tools/graphviz/
 */
void ist_dump_graph(char * filename, ISTSharingTree * T)
{
    ISTSharingTree *S;
    FILE * f ;

    f =fopen(filename,  "w") ;

    fprintf(f,"digraph IST {\n") ;


    S = ist_copy(T);
    rec_parc(f, S->Root) ;
    ist_dispose(S);

    fprintf(f,"}\n") ;
    fclose (f) ;	

}
