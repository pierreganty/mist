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

   Copyright 2002, 2003, 2004, Pierre Ganty, Anthony Piron
 */

#ifndef __LAPARSER_H
#define __LAPARSER_H

#include <string.h>
#include <stdlib.h>
#include "error.h"
#include "xmalloc.h"
#include "tbsymbol.h"
#include "tbsymbolinfo.h"
#include "tree.h"
#include "transsystem.h"

#ifndef __LOCAL
#define EXTERN extern
#endif

int linenumber;
int nbr_var;
T_PTR_tbsymbol tbsymbol;

int my_yyparse(T_PTR_tree* tree, char* filename);

void reset(T_PTR_tbsymbol_entry entry);

#endif
