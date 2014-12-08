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


/*
 * In that file we define some useful values (e.g. TRUE FALSE etc) and some
 * type (e.g. boolean, integer16, etc)
 */
#ifndef __DEF_H
#define __DEF_H

/*
 * Attention the uppercase version is used by the parser and have a different
 * value
 */
#ifndef true
# define true    1
# define false   0
#endif

/*bounds for integer types*/
#define MaxInt8         127
#define MaxInt16        32767
#define MaxInt32        2147483647L


/*We define our infinite value as a negative one*/
#undef INFINITY
#define INFINITY        (-32768L)

/* Classical useful types ... */
typedef unsigned char boolean;
typedef short integer8;
typedef int integer16;
typedef long integer32;
#endif
