// vim:sw=4:ts=4:cindent
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

   Copyright 2014, Pedro Valero
 */

#ifndef _DEBUG_H
#define _DEBUG_H


#define DEFAULT_BUFF_SIZE 255
extern int verbose;

/*
    These functions provide us with details where the error / event occurred: line, file,
    function, plus whichever message we wrote explaining the error.
 */
void debug_printf(int line, const char *file, const char *function, char *fmt, ...);

/*
   These macros provide a simpler interface to invoke functions above declared
   with the correct arguments, and should be preferred.
 */

/*
   These macros always expand to actual code.
 */
#define PRINTF(...) do {if (verbose >= 1) debug_printf(__LINE__, __FILE__,__PRETTY_FUNCTION__, __VA_ARGS__); } while(0)


#define PRINT_IST(X) do {if (verbose >= 2 ) ist_write(X); if (verbose >= 1) ist_stat(X); } while(0)


#endif /* _DEBUG_H */

