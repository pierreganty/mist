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

   Copyright 2002, Anthony Piron
 */

#include "tbsymbol.h"
#include "error.h"
#include "xmalloc.h"
#include <stdlib.h>
#include <string.h>

void
tbsymbol_init(T_PTR_tbsymbol* tbsymbol, size_t tb_size) {
  (*tbsymbol) = (T_PTR_tbsymbol)xmalloc(sizeof(T_tbsymbol));
  (*tbsymbol)->table = (T_PTR_tbsymbol_entry)
    xmalloc(tb_size * sizeof(T_tbsymbol_entry));
  (*tbsymbol)->size = tb_size;
  (*tbsymbol)->nbr_entries = 0;
}


void
tbsymbol_destroy(T_PTR_tbsymbol* tbsymbol) {
/*   size_t pos; */

/*   for (pos=0; pos < (*tbsymbol)->nbr_entries; pos++) */
/*     if ((*tbsymbol)->table[pos].info != NULL)  */
/*       free((*tbsymbol)->table[pos].info); */

  xfree((*tbsymbol)->table);
  xfree((*tbsymbol));
  (*tbsymbol) = NULL;
}


T_PTR_tbsymbol_entry
tbsymbol_insert(T_PTR_tbsymbol tbsymbol, char* name) {
  if (tbsymbol->nbr_entries < tbsymbol->size) {
    strncpy(tbsymbol->table[tbsymbol->nbr_entries].name, name, MAX_STR-1);
    tbsymbol->table[tbsymbol->nbr_entries].name[MAX_STR-1] = 0;
    tbsymbol->table[tbsymbol->nbr_entries].info = NULL;
    return (tbsymbol->table)+(tbsymbol->nbr_entries++);
  }

  return NULL;
}


T_PTR_tbsymbol_entry
tbsymbol_select(T_PTR_tbsymbol tbsymbol, char* name) {
  size_t pos;

  for (pos=0; pos < tbsymbol->nbr_entries; pos++)
    if (strcmp(tbsymbol->table[pos].name, name) == 0)
      break;

  return (pos >= tbsymbol->nbr_entries)?NULL:tbsymbol->table+pos;
}


void
tbsymbol_setinfo(T_PTR_tbsymbol_entry entry, void* info, size_t size) {
  entry->info = info;
}


void*
tbsymbol_getinfo(T_PTR_tbsymbol_entry entry) {
  return entry->info;
}


void tbsymbol_dump(T_PTR_tbsymbol tbsymbol,
		   void (*callback)(T_PTR_tbsymbol_entry entry) ) {
  size_t pos;

/*   printf("Max entries: %d   Nbr entries: %d\n", */
/* 	 tbsymbol->size, */
/* 	 tbsymbol->nbr_entries); */
  for (pos=0; pos < tbsymbol->nbr_entries; pos++) {
    if (callback)
      (*callback)((tbsymbol->table)+pos);
  }
}
