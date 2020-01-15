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
 * $Id: stringbuffer.c,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "stringbuffer.h"
#include "util.h"

extern int errno;

void sb_clear(sb_t *sb) {
   if (sb == NULL)
      return;
   if (sb->data != NULL)
      free(sb->data);
   sb->data = NULL;
   sb->size = 0;
}
                                                                                
void sb_free(sb_t *sb) {
   if (sb == NULL)
      return;
   if (sb->data != NULL)
      free(sb->data);
   free(sb);
   sb = NULL;
}
                                                                                
sb_t *sb_create(sb_t *sb, unsigned int size) {
   sb_t *ob = NULL;

   if ((ob = (sb_t *)malloc(sizeof(sb_t))) == NULL)
      return NULL;
   if (sb == NULL || sb->used == 0) {
      ob->size = size;
      ob->used = 0;
      if ((ob->data = (char *)malloc(sizeof(char) * size)) == NULL) {
         free(ob);
         return NULL;
      }
   } else {
      ob->used = sb->used;
      ob->size = ob->used + 1;
      if ((ob->data = (char *)malloc(MAX(ob->used, size))) == NULL) {
         free(ob);
         ob = NULL;
      } else {
         memcpy(ob->data, sb->data, sb->used);
         ob->data[ob->size] = '\0';
      }
   }
   return ob;
}

int sb_reset(sb_t *sb) {
   if (sb == NULL) {
      errno = EINVAL;
      return -1;
   }
   sb_free(sb);
   sb->size = 0;
   sb->used = 0;
   sb->data = NULL;

   return 0;
}


int sb_copy(sb_t *sb_dst, sb_t *sb_src) {
   char *data = NULL;

   if (sb_src == NULL || sb_dst == NULL) {
      errno = EINVAL;
      return -1;
   }
   if ((data = (char *)malloc(sb_src->size + 1)) == NULL) {
      errno = ENOMEM;
      return -1;
   }
   if (sb_dst->data != NULL)
      free(sb_dst->data);
   sb_dst->data = data;
   memcpy(sb_dst->data, sb_src->data, sb_src->used);
   sb_dst->size = sb_src->size;
   sb_dst->used = sb_src->used;

   return 0;
}

int sb_cat_str(sb_t *sb, const char *s) {
   char *data;
   long size;
   long l, used;

   if (sb == NULL || s == NULL) {
      errno = EINVAL;
      return -1;
   }
   size = sb->size;
   l = strlen(s);
   used = sb->used + l;
   if (used >= size) {
      size *= 2;
      size += l;
      if ((data = realloc(sb->data, size)) == NULL) {
         errno = ENOMEM;
         return -1;
      }
      sb->data = data;
      sb->size = size;
   }
   memcpy (sb->data + sb->used, s, l);
   sb->used = used;
   sb->data[sb->used] = 0;

   return 0;
}

/* the following temp variables should be close enough */
int sb_cat_int(sb_t *sb, int i) {
   char temp[32];
   snprintf(temp, sizeof(temp), "%d", i);
   return sb_cat_str(sb, temp);
}

int sb_cat_uint(sb_t *sb, unsigned int ui) {
   char temp[32];
   snprintf(temp, sizeof(temp), "%u", ui);
   return sb_cat_str(sb, temp);
}

int sb_cat_long(sb_t *sb, long l) {
   char temp[64];
   snprintf(temp, sizeof(temp), "%ld", l);
   return sb_cat_str(sb, temp);
}

int sb_cat_ulong(sb_t *sb, unsigned long ul) {
   char temp[64];
   snprintf(temp, sizeof(temp), "%lu", ul);
   return sb_cat_str(sb, temp);
}

