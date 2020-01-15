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
 * $Id: fluke_189.c,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

/*
 * Original version by: Dan Smith (dsmith@danplanet.com) 
 *
 * Extensive modifications have occurred to reflect the new API
 * between the query interface processor and different meter types.
 *
 * Based on the Fluke 89/18x Remote Interface Specification
 * http://cfa-www.harvard.edu/~thunter/manuals/RemoteSpec89_18X.htm
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <string.h>
#include "meter.h"
#include "fluke_189.h"

/* serial port baud rate */
#define BAUDRATE		B9600

extern int mdebug;
extern int errno;

static int command;     /* last command that was executed */

/* valid command types */
#define CMD_QM		1
#define CMD_ID		2
#define CMD_DS		3
#define CMD_RI		4
#define CMD_SF		5

/* local prototypes */
static int get_command(char *cmd);

speed_t fluke_189_baudrate() {
   return BAUDRATE;
}

/* set additional attributes */
int fluke_189_init(struct termios *newtio) {
   newtio->c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
   newtio->c_iflag = IGNPAR;
   newtio->c_oflag = 0;
   newtio->c_lflag = 0;
   newtio->c_cflag |= (CLOCAL | CREAD);
   newtio->c_cc[VMIN] = 1;

   /* no software flow control */
   newtio->c_iflag &= ~(IXON | IXOFF | IXANY);

   return 0;
}

int fluke_189_query(int fd) {
   return fluke_189_send(fd, "QM", 2);
}

int fluke_189_send(int fd, char *cmd, int len) {
   char str[16];
   int r;
   unsigned int status;

   /* save the command type for the read */
   command = get_command(cmd);

   /* Construct the FLUKE Command XY<CR> */
   len = strlen(cmd);
   strncpy(str, cmd, len);
   str[len] = '\r';
   str[len + 1] = '\0';

   /* ensure DTR goes low and RTS goes high */ 
   ioctl(fd, TIOCMGET, &status);
   status &= ~TIOCM_DTR ;
   status |= TIOCM_RTS;
   ioctl(fd, TIOCMSET, &status);

   if (mdebug)
      printf("fluke_189_send: command %s\n", str);
   if ((r = write(fd, str, len + 1)) != (len + 1)) { /* send command */
      perror("fluke write");
      exit(1);
   }

   return r;
}

int fluke_189_read(int fd, char *buf, int len, int *status) {
   int r, c = 0;
   int i;
   char buf_tmp[len];
   int maxlines = 1;

   *status = 0;

   /* only QM and ID are multiline */
   if (command == CMD_ID || command == CMD_QM)
      maxlines = 2;

   while (1) { /* read all lines based on command type */
      int lines = 0;
      if ((r = read(fd, buf_tmp + c, len - c)) == -1)
         return -1;
      c += r;
      for (i = 0; i < c; i++)
         if (buf_tmp[i] == '\r')
            lines++;
      if (mdebug)
         printf("fluke_189_read: line %d of %d\n", lines, maxlines);
      if (lines == maxlines || r == 0)
         break;
   }

   for (i = c - 1; i > 0; i--) { /* remove last <CR> */
      if (buf_tmp[i] == '\r') {
         buf_tmp[i] = '\0';
         break;
      }
   }

   *status = atoi(buf_tmp);
   if (mdebug)
      printf("fluke_189_read: status %d\n", *status);
   if (*status == 0) { /* success */
      while (buf_tmp[i] != '\r' && i < c)
         buf_tmp[i++] = '\0';
      if (mdebug) {
         printf("fluke_189_read: data ");
         for (i = 2; i < c; i++) /* skip the leading ACK and \r */
            printf("%c", buf_tmp[i]);
         printf("\n");
      }
      memcpy(buf, buf_tmp + 2, c);
      c -= 2;
   }

   return c;
}

int fluke_189_parse(char *buf, int len, mm_t *mm) {
   int i, j;
   float value;

   if (buf == NULL || mm == NULL) {
      errno = EINVAL;
      return -1;
   }
   for (i = 0; i < len; i++) /* skip past the comma */
      if (buf[i] == ',')
         break;
   i++;
   if (i >= len) { /* search for "QM," */
      errno = EINVAL;
      return -1;
   }
   if (mdebug) {
      printf("fluke_189_parse: ");
      for (j = i; j < len; j++)
         printf("%c", buf[j]);
      printf("\n");
   }
   if (sscanf(buf + i, "+%f", &value) != 1) {
      fprintf(stderr, "fluke_189_parse: unable to parse value\n");
   } else {
      int units = 0;

      while (!isspace(buf[i]) && i < len)
         i++;
      i++;
      if (i >= len) {
         errno = EINVAL;
         return -1;
      }
      if (buf[i] == 'm') {
         units |= MUNITS_MILLI;
         i++;
      }
      if (buf[i] == 'u') {
         units |= MUNITS_MICRO;
         i++;
      }
      if (buf[i] == 'A') {
         units |= MUNITS_AMPS;
      } else if (buf[i] == 'V') {
         units |= MUNITS_VOLTS;
      } else {
         errno = EINVAL;
         return -1;
      }
      mm[0].value = value;
      mm[0].units = units;
   }

   return 0;
}

static int get_command(char *cmd) {
   int c = -1;

   if (strncmp(cmd, "QM", 2) == 0)
      c = CMD_QM;
   else if (strncmp(cmd, "ID", 2) == 0)
      c = CMD_ID;
   else if (strncmp(cmd, "DS", 2) == 0)
      c = CMD_DS;
   else if (strncmp(cmd, "SF", 2) == 0)
      c = CMD_SF;
   else if (strncmp(cmd, "RI", 2) == 0)
      c = CMD_RI;

   return c;
}

