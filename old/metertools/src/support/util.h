/*
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 of the
 *   License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Library General Public License for more details.
 *
 *   For a copy of the GNU Library General Public License
 *   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *   Boston, MA 02111-1307, USA.  or go to http://www.gnu.org
 *
 * $Id: util.h,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

#ifndef _UTIL_H
#define _UTIL_H

#ifndef PATH_SEPARATOR
#   define PATH_SEPARATOR               ":"
#endif
#ifndef FILE_SEPARATOR
#   define FILE_SEPARATOR               '/'
#endif

/* path to the null device */
#define PATH_DEVNULL			"/dev/null"

/* IPv4 */
#define MAX_RELAY_SIZE			16

/* minimum start size for VARG buffer */
#define VARG_MIN_SIZE			64

#ifndef MIN
#   define MIN(x, y)	((x < y) ? x : y)
#endif
#ifndef MAX
#   define MAX(x, y)	((x > y) ? x : y)
#endif

#define MINUTES_TO_SECONDS(x)		(x * 60)
#define SECONDS_TO_MINUTES(x)		(x / 60)

/* lookup the hostname, return NULL if an error */
char *get_hostname();

/* recursively remove a directory and all contents */
int recursive_rmdir(char *directory);

/* create a new string based on format and arguments */
char *varg_tostring(const char *fmt, ...);

/* make the current process a daemon */
void daemonize();

/* trim the string of any whitespace (leading and trailing) */
void trim(char *s);

#endif

