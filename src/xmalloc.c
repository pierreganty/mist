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

   Copyright 2003, 2004, Anthony Piron, Ganty Pierre
 */

#include "xmalloc.h"
#include "stdlib.h"
#include "error.h"

void*
xmalloc(size_t size) {
  void* ptr;

  if (size == 0)
    err_quit("xmalloc: zero size");
  ptr = malloc(size);
  if (ptr == NULL) {
    err_sys("xmalloc: not enough memory for %lu bytes", (u_long) size);
  }
  return ptr;
}

void*
xrealloc(void* ptr, size_t size) {
  void* new_ptr;

  if (size == 0)
    err_quit("xrealloc: zero size");
  new_ptr = realloc(ptr, size);
  if (new_ptr == NULL)
    err_sys("\nxrealloc: not enough memory for %lu bytes", (u_long) size);
  return new_ptr;
}

void
xfree(void* ptr) {
  if (ptr == NULL)
    err_quit("xfree: NULL pointer passed as argument");
  free(ptr);
}
