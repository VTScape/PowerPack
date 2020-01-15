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
 * $Id: util.c,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include "util.h"

extern int errno;

/*
 * Keep a static copy of the hostname, if not already known perform
 * a lookup.  If an error occurs return NULL and errno will indicate
 * the specific error.
 */
char *get_hostname() {
   static char *hn = NULL;

   if (hn == NULL) {
      unsigned int size = 32;

      hn = malloc(sizeof(char) * size);
      if (hn == NULL) {
         errno = ENOMEM;
         return NULL;
      }
      while (gethostname(hn, size) == -1) {
         if (errno == EINVAL || errno == ENAMETOOLONG) {
            size *= 2;
            hn = realloc(hn, size);
         } else {
            free(hn);
            break;
         }
      }
   }

   return hn;
}

/*
 * Remove the provided directory totally from the filesystem.  This
 * is done in a recursive fashion for any files and directories
 * contained within that directory.  Any problem in this operation
 * should cause -1 to be returned and errno to contain the error.
 */
int recursive_rmdir(char *directory) {
   DIR *d = NULL;
   struct dirent *dir = NULL;
   char *name = NULL;
   int i = 0;
   struct stat s;
   unsigned int size = 0;

   if (directory == NULL) {
      errno = EINVAL;
      return -1;
   }
   if ((d = opendir(directory)) == NULL)
      return -1;
   while ((dir = readdir(d)) != NULL) {
      if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
         continue;
      size = strlen(directory) + strlen(dir->d_name) + 2;
      name = (char *)malloc(size * sizeof(char));
      if (name == NULL) {
         closedir(d);
         errno = ENOMEM;
         return -1;
      }
      snprintf(name, size, "%s%c%s", directory, FILE_SEPARATOR, dir->d_name);
      if (lstat(name, &s) == -1) {
         closedir(d);
         free(name);
         return -1;
      }
      if (S_ISDIR(s.st_mode)) {
         i = recursive_rmdir(name);
      } else {
         i = unlink(name);
      }
      free(name);
      if (i == -1) {
         closedir(d);
         return -1;
      }
   }
   closedir(d);
   rmdir(directory);

   return 0;
}

/*
 * Make the current process a daemon by forking and detaching I/O.
 * This method is suitable to detach any process from the current
 * terminal and run it in the background.
 */
void daemonize() {
   int fd;
                                                                                
   switch (fork()) {
      case -1:
         fprintf(stderr, "unable to fork: %s", strerror(errno));
         exit(-1);
      case 0:
         break;
      default:
         exit(0);
   }
   if (setsid() == -1) {
      fprintf(stderr, "could not set process group: %s", strerror(errno));
      exit(-1);
   }
   if ((fd = open(PATH_DEVNULL, O_RDWR, 0)) != -1) {
      dup2(fd, STDIN_FILENO);
      dup2(fd, STDOUT_FILENO);
      dup2(fd, STDERR_FILENO);
      if (fd > 2)
         close(fd);
   } else {
      fprintf(stderr, "could not detach file discriptors: %s", strerror(errno));
      exit(-1);
   }
}

/*
 * Accept a variable list of arguments and using the given format
 * create a new string that is dynamically allocated.  The caller
 * is responsible to free any memory.
 */
char *varg_tostring(const char *fmt, ...) {
   char *p = NULL;
   va_list ap;
   int n, size = VARG_MIN_SIZE;

   if ((p = malloc(size)) == NULL)
      return NULL;
   while (1) {
      va_start(ap, fmt);
      n = vsnprintf(p, size, fmt, ap);
      va_end(ap);
      if (n > -1 && n < size)
         break;
      if (n > -1)
         size = n + 1;
      else
         size *= 2;
      if ((p = realloc(p, size)) == NULL)
         return NULL;
   }

   return p;
}

void trim(char *trim_string) {
   char *p;
   char ch;
   unsigned int len;
                                                                                
   p = trim_string;
   while ((*p == ' ') || (*p == '\t'))
      ch = *p++;
   trim_string = p;
   len = strlen(trim_string);
   p = &trim_string[len - 1];
   while ((*p == ' ') || (*p == '\t'))
      ch = *p--;
   ch = *p++;
   *p = '\0';
}

