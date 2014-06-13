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
 * $Id: watts_up.c,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

/*
 * This is the communication interface to the Watts Up Pro watt meter
 * available from Double Educational Products (www.doubleed.com).
 * For details about what is possible, reference the comprotocol.pdf
 * file.  This conforms to the standard meter communication protocol.
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "meter.h"
#include "watts_up.h"

/* serial port baud rate */
#define BAUDRATE		B38400
//#define BAUDRATE		B9600

static int preamble = 0;

extern int errno;

#define SEND_CLEAR		"#R,W,0;"

/* query format: E=external, timestamp, sampling interval */
#define SEND_QUERY		"#L,W,3,E,-,1;"

speed_t watts_up_baudrate() {
   return BAUDRATE;
}

/* set new attributes */
int watts_up_init(struct termios *newtio) {
   newtio->c_cflag = (BAUDRATE | CS8 | CLOCAL | CREAD | CSTOPB);
   newtio->c_iflag = (IGNPAR);
   newtio->c_oflag = (IGNPAR);
   newtio->c_lflag = 0;

   /* no software flow control */
   newtio->c_iflag &= ~(IXON | IXOFF | IXANY);
   newtio->c_oflag &= ~(IXON | IXOFF | IXANY);

   return 0;
}

int watts_up_query(int fd) {
   int rc = 0;
   char *c;

   if (!preamble) {
      struct timespec tspec;

      c = SEND_CLEAR;
      tspec.tv_sec = 1;
      tspec.tv_nsec = 0;
      if (watts_up_send(fd, SEND_CLEAR, strlen(SEND_CLEAR)) < 0) {
         fprintf(stderr, "watts_up_query: sending preamble failed\n");
         rc = -1;
      } else {
         preamble = 1;
      }
      nanosleep(&tspec, NULL);
   }
   c = SEND_QUERY;

   return watts_up_send(fd, SEND_QUERY, strlen(SEND_QUERY));
}

int watts_up_send(int fd, char *cmd, int len) {
   int r;

   if ((r = write(fd, cmd, len)) == -1) {
      perror("watts_up write");
      exit(1);
   }

   return r;
}

int watts_up_read(int fd, char *buf, int len, int *status) {
   char buf_tmp[256];
   int c, r;
   char *p;
   int i;

   *status = 0;

   /* read until EOL */
   c = 0;
   while (1) {
      if ((r = read(fd, buf_tmp + c, sizeof(buf_tmp) - c)) == -1) {
         errno = ENOBUFS;
         return -1;
      }
      c += r;
      if (strchr(buf_tmp, ';'))
         break; /* EOL */
   }
   i = 0;
   *strchr(buf_tmp, ';') = '\0';
   p = buf_tmp;
   while (*p != '#' && *p != '\0' && ++i < c)
      p++;
   if (i >= c || i >= len) {
      errno = ENOBUFS;
      return -1;
   }
   c = strlen(p);
   memcpy(buf, p, c);
   buf[c] = '\0';

   return c;
}

/*
 * Format is as follows:
 *
 *    #d,-,16,Watts,Volts,Amps,Watt Hours,Cost,Mo. kWh,Mo. Cost,
 *       Max watts,Max volts,Max amps,Min watts,Min volts,Min Amps,
 *       Power factor,Duty cycle,Power Cycle
 */
int watts_up_parse(char *buf, int len, mm_t *mm) {
   double value;
   char *token;
   int i = 0;
   char *p;
   int pos = 0;

   if (buf == NULL || mm == NULL) {
      errno = EINVAL;
      return -1;
   }
   p = strstr(buf, "#");
   if (p == NULL || strncmp(p, "#d,", 3) != 0)
      return -1;
//printf("have buffer %s\n", p);
   token = strtok(p, ",");
   while (token != NULL) {
      value = -2.0; /* set to a flag */
      switch (i++) { /* look at offset and set applicable field */
         case 3:
            value = atoi(token);
            value /= 10;
            mm[pos].units = MUNITS_WATTS;
            break;
         case 4:
            value = atoi(token);
            value /= 10;
            mm[pos].units = MUNITS_VOLTS;
            break;
         case 5:
            value = atoi(token);
            value /= 1000;
            mm[pos].units = MUNITS_AMPS;
            break;
         case 16:
            value = atoi(token);
            value /= 100;
            mm[pos].units = MUNITS_POWERFACTOR;
            break;
      }
      if (value > -1.0)
         mm[pos++].value = value;
      token = strtok(NULL, ",");
   }

   return 0;
}

