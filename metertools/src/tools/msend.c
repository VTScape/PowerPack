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
 * $Id: msend.c,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

/*
 * For those meters that accept commands, send the formatted
 * command based on that provided on the command line.
 */

#include <stdio.h>
#include <libgen.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "stringbuffer.h"
#include "meter.h"

/* meter debugging */
extern int mdebug;

extern int errno;

/* local prototypes */
void show_usage(char *name);

extern int optind, opterr, optopt;

int main(int argc, char *argv[]) {
   struct termios oldtio;
   char buf[256];
   int rc;
   int fd;
   int type = METER_TYPE;
   int slen, rlen;
   int c;
   sb_t *sb;
   char *serial_port = SERIAL_PORT;

   opterr = 0;
   while ((c = getopt(argc, argv, "s:m:dh")) != -1) {
      switch (c) {
         case 'm':
            if ((type = meter_type(optarg)) == 0) {
               fprintf(stderr, "unknown meter %s for -m\n", optarg);
               exit(1);
            }
            break;
         case 's':
            serial_port = strdup(optarg);
            break;
         case 'd':
            mdebug = 1;
            break;
         case 'h':
            show_usage(basename(argv[0]));
            exit(0);
         default:
            fprintf(stderr, "unknown option -%c, use -h for help\n", optopt);
            exit(1);
      }
   }

   if (optind < argc - 1) {
      fprintf(stderr, "command not given, use -h for help\n");
      exit(1);
   }
   sb = sb_create(NULL, 10);
   while (optind < argc)
      sb_cat_str(sb, argv[optind++]);
   if ((fd = meter_init(serial_port, &oldtio, type)) == -1) {
      fprintf(stderr, "meter_init: %s\n", strerror(errno));
      exit(1);
   }
   slen = sb->size;
   rlen = sizeof(buf);
   if ((rc = meter_command(fd, type, sb->data, slen, buf,
         rlen)) != 0)
      fprintf(stderr, "meter_command returned %d\n", rc);
   else if (strlen(buf) > 0)
      printf("%s\n", buf);
   else
      rc = (unsigned int)buf;
   meter_close(fd, &oldtio);
   sb_free(sb);

   return rc;
}

void show_usage(char *name) {
   printf("Usage: %s [-d] [-m meter] [-s serialport] command [arg1 arg2 ...]\n",
      name);
   printf("       %s [-h]\n", name);
   printf("\t-m meter       Meter type [%s]\n", meter_name(METER_TYPE));
   printf("\t-s serialport  Serial port [%s]\n", SERIAL_PORT);
   printf("\t-d             Turn on debugging\n");
   printf("\t-h             This help screen\n");
}

