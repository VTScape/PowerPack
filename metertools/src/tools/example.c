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
 * $Id: example.c,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

/*
 * The following example represents a typical usage scenario to connect to a
 * meter and read measurement values.  Compile using: (use -I and -L
 * as applicable depending on where you installed)
 *
 * example.c compiled using:  gcc -lmeter -lsupport example.c -o example
 *
 * Important: METER_TYPE should be replaced as applicable per the types
 * defined in meter.h.
 */

#include <stdio.h>
#include <termios.h>
#include <errno.h>
#include "meter.h"

#define TIMEOUT                      5
#define READINGS                     10

int main(int argc, char *argv[]) {
   int fd;
   char *serialport;
   mm_t *mm;
   int i;
   int num;
   int type = METER_TYPE;

   if (argc != 2) {
      fprintf(stderr, "specify serial port (i.e. /dev/ttyS0)\n");
      return -1;
   }
   serialport = argv[1];
   if ((fd = meter_init(serialport, &oldtio, type)) == -1) {
      fprintf(stderr, "unable to initialize meter\n");
      return -1;
   }
   if ((num = meter_alloc_mms(type, &mm)) == -1) {
      fprintf(stderr, "meter_alloc_mms: %s\n", strerror(errno));
      return -1;
   }
   mm->to_sec = TIMEOUT;
   for (i = 0; i < READINGS; i++)
      meter_read(fd, METER_TYPE, mm);
   meter_close(fd, &oldtio);

   return 0;
}

