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
 * $Id: protek_506.c,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "meter.h"
#include "protek_506.h"

/* serial port baud rate */
#define BAUDRATE		B1200

extern int errno;

speed_t protek_506_baudrate() {
   return BAUDRATE;
}

/* set new attributes */
int protek_506_init(struct termios *newtio) {
   newtio->c_cflag = (BAUDRATE | CS7 | CLOCAL | CREAD | CSTOPB);
   newtio->c_iflag = (IGNPAR);
   newtio->c_oflag = (IGNPAR);
   newtio->c_lflag = 0;

   /* no software flow control */
   newtio->c_iflag &= ~(IXON | IXOFF | IXANY);
   newtio->c_oflag &= ~(IXON | IXOFF | IXANY);

   return 0;
}

int protek_506_query(int fd) {
   return protek_506_send(fd, "\r", 1);
}

/* Note: Protek doesn't accept commands! */
/* The only command to the Protek is a '\r' */
int protek_506_send(int fd, char *cmd, int len) {
   int r;

   if ((r = write(fd, cmd, len)) != 1) {
      perror("protek write");
      exit(1);
   }

   return r;
}

int protek_506_read(int fd, char *buf, int len, int *status) {
   char buf_tmp[256];
   int c, r;

   *status = 0;

   /* read until EOL <CR> */
   c = 0;
   while (1) {
      if ((r = read(fd, buf_tmp + c, 1)) == -1)
         return -1;
      c += r;
      if (strchr(buf_tmp, '\r'))
         break; /* EOL */
   }
   *strchr(buf_tmp, '\r') = '\0';
   memcpy(buf, buf_tmp, strlen(buf_tmp));

   return r;
}

int protek_506_parse(char *buf, int len, mm_t *mm) {
   float value;
   char a,b,u;

   if (buf == NULL || mm == NULL) {
      errno = EINVAL;
      return -1;
   }
   if (!sscanf(buf, "%c%c %f %c", &a, &b, &value, &u)) {
      printf("ERROR: Unable to parse PROTEK reading: %s\n", buf);
   }
   mm[0].value = value;

   return 0;
}

