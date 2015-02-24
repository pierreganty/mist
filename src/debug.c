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

#include <stdio.h>
#include <syslog.h>
#include <stdarg.h>
#include <string.h>

#include "debug.h"


void debug_printf(int line, const char *file, const char *function, char *fmt, ...)
{
	char buffer[DEFAULT_BUFF_SIZE];
	va_list args;

	snprintf(buffer, DEFAULT_BUFF_SIZE, "%s (%s:%d): %s", function, file, line, fmt);

	va_start(args, fmt);
	vfprintf(stderr, buffer, args);
	va_end(args);
}

