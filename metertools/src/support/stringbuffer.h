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
 * $Id: stringbuffer.h,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

#ifndef _STRINGBUFFER_H
#define _STRINGBUFFER_H
#include <stdio.h>

/*
 * This is an implementation for dynamic string buffers.  These buffers can
 * grow as needed depending on what is added and they should be type safe
 * with regards to how they are created.  Using data types other than
 * arrays of characters causes a conversion in the type to be done.  Note
 * that no formatting extensions are provided for primitive types.  In all
 * instances when not otherwise indicated below, a -1 is returned on error
 * or 0 otherwise.  Note that mallocs are done efficiently because it is
 * done in "chunks".
 */

/* disassociate length from an application */
typedef unsigned int sb_len_t;

typedef struct {
   sb_len_t size;            /* current maximum size of the buffer */
   unsigned int used;        /* number of characters in the string */
   char *data;               /* pointer to the data array buffer */
} sb_t;


/* create a string buffer and return it (use sb as a starting string) */
/* the size is used for optimization if you know it, otherwise give 0) */
sb_t *sb_create(sb_t *sb, sb_len_t size);

/* copies from source to destination */
int sb_copy(sb_t *sb_dst, sb_t *sb_src);

/* concatenate another string must be a \0 terminate string */
int sb_cat_str(sb_t *sb, const char *s);

/* concatenate an integer on the end */
int sb_cat_int(sb_t *sb, int i);

/* concatenate an unsigned integer to the end */
int sb_cat_uint(sb_t *sb, unsigned int ui);

/* concatenate the long value to the end */
int sb_cat_long(sb_t *sb, long l);

/* concatenate an unsigned long value to the end */
int sb_cat_ulong(sb_t *sb, unsigned long ul);

/* reset the buffer for reuse (freeing the data) */
int sb_reset(sb_t *sb);

/* clear the buffer for reuse (keep the same sized array */
void sb_clear(sb_t *sb);

/* free and return all memory */
void sb_free(sb_t *sb);

/* get the char array string buffer */
static inline char *sb_tostring(sb_t *sb) {
   if (sb == NULL)
      return NULL;
   return sb->data;
}

/* return the length of the string */
static inline sb_len_t sb_length(sb_t *sb) {
   if (sb == NULL)
      return 0;
   return sb->size;
}

#endif

