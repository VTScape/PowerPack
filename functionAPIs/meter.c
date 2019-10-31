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
 * $Id: meter.c,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

/*
 * Meter control general routines that can be embedded in an application
 * and are used by the msend, mlogger, and mclient command line utilities.
 * For general interface specifications, reference the meter.h file,
 * only implementation specific details are here but not necessarily
 * how to use them in a cohesive manner.
 */

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>
#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif
#include "util.h"
#include "meter.h"

/* implementation interfaces for all meters */
#include "rs_22812.h"
#include "fluke_189.h"
#include "protek_506.h"
#include "watts_up.h"

extern int errno;

/* maximum time to wait for a meter reading */
#ifndef MAX_TIMEOUT
#   define MAX_TIMEOUT		30
#endif

/* for those that need debugging, define to 1 externally */
int mdebug = 0;

/* if changes are made to this list, ensure the terminator is last */
static meter_t meters[] = {
   { "wattsup", "Watts Up Pro", 4, METER_WATTS_UP,
     watts_up_query, watts_up_send,
     watts_up_read, watts_up_init,
     watts_up_parse, watts_up_baudrate },
   { "fluke189", "Fluke 189", 1, METER_FLUKE_189,
     fluke_189_query, fluke_189_send,
     fluke_189_read, fluke_189_init,
     fluke_189_parse, fluke_189_baudrate },
   { "protek506", "Protek 506", 1, METER_PROTEK_506,
     protek_506_query, protek_506_send,
     protek_506_read, protek_506_init,
     protek_506_parse, protek_506_baudrate },
   { "rs22812", "Radio Shack 22-812", 1, METER_RS_22812,
     NULL, NULL, /* not supported */
     rs_22812_read, rs_22812_init,
     rs_22812_parse, rs_22812_baudrate },
   { NULL, NULL, 0, 0, NULL, NULL, NULL, NULL, NULL } /* terminator */
}; /* end if meter list */

int meter_close(int fd, struct termios* oldtio) {
   tcsetattr(fd, TCSANOW, oldtio);
   close(fd);

   return 0;
}

/* send a command to the meter and expect a response */
int meter_command(int fd, int type, char *sbuf, int lsbuf, char *rbuf,
      int lrbuf) {
   int rc = 0;             /* return code */
   int sb, rb;             /* sent and received bytes */
   meter_t *meter = NULL;  /* pointer to meter info */

   if ((meter = meter_lookup(type)) == NULL || meter->read == NULL) {
      errno = EINVAL;
      rc = -1;
   } else {
      if (meter->send != NULL) {
         sb = meter->send(fd, sbuf, lsbuf);
         if (mdebug)
            printf("send %d bytes\n", sb);
      }
      rb = meter->read(fd, rbuf, lrbuf, &rc);
      if (mdebug)
         printf("read %d bytes\n", rb);
   }

   return rc;
}

/* search for the meter information and return a pointer if found */
meter_t *meter_lookup(int type) {
   meter_t *meter = NULL;
   int i = 0;

   while (meters[i].name != NULL && meter == NULL) {
      if (meters[i].type == type)
         meter = &meters[i];
      i++;
   }

   return meter;
}

int meter_read(int fd, int type, mm_t *mm) {
   int n;
   int rc = 0;
   fd_set input;
   struct timeval timeout, *tout = NULL;
   char buf[DBUFSZ];
   meter_t *meter = NULL;

   if (mm == NULL) {
      errno = EINVAL;
      return -1;
   }

   /* find the meter */
   if ((meter = meter_lookup(type)) == NULL) {
      errno = EINVAL;
      return -1;
   }

   /* initialize the input set */
   FD_ZERO(&input);
   FD_SET(fd, &input);
   mm->value = 0.0;

   /* initialize the timeout structure (default is no timeout) */
   if (mm->to_sec > 0) {
      timeout.tv_sec = MIN(mm[0].to_sec, MAX_TIMEOUT);
      timeout.tv_usec = 0;
      tout = &timeout;
   }

   /* perform the initial query */
   if (meter->query != NULL)
      meter->query(fd);

   /* read the data or get keyboard input */
   if ((n = select(fd + 1, &input, NULL, NULL, tout)) == -1) {
      if (errno != EINTR)
         perror("select failed");
      return -1;
   } else if (n == 0) {
      fprintf(stderr, "timeout reading meter\n"); /* retry */
      rc = 1;
   } else { /* have data, parse as needed */
      if (FD_ISSET(fd, &input)) {
         int status;
         int rb;

         rb = meter->read(fd, buf, sizeof(buf), &status);
         if (rb == 0) {
            errno = ENODATA;
            return -1;
         }
         rc = meter->parse(buf, rb, mm);
      }
   }

   return rc;
}

/* allocate an array of mm measures (return # in array or -1 if error) */
int meter_alloc_mms(int type, mm_t **mm) {
   meter_t *meter = NULL;

   if ((meter = meter_lookup(type)) == NULL) {
      errno = EINVAL;
      return -1;
   }

   if ((*mm = malloc(sizeof(mm_t) * meter->count)) == NULL) {
      errno = ENOMEM;
      return -1;
   }

   return meter->count;
}
                                                                                
int meter_init(char* devicename, struct termios *oldtio, int type) {
   int fd = -1, r = -1;
   struct termios newtio;
   speed_t baudrate;
   meter_t *meter = NULL;

   if ((meter = meter_lookup(type)) == NULL || meter->baudrate == NULL) {
      errno = EINVAL;
      return -1;
   }
   if ((fd = open(devicename, O_RDWR | O_NOCTTY)) < 0)
      return -1;
   tcgetattr(fd, oldtio); /* save current port settings */
   memcpy(&newtio, oldtio, sizeof(struct termios));
   if (meter->init != NULL)
      r = meter->init(&newtio);
   baudrate = meter->baudrate();

   if (r == 0) { /* apply new attributes if init successful */
      cfsetispeed(&newtio, baudrate);
      cfsetospeed(&newtio, baudrate);
      tcflush(fd, TCIFLUSH);
      tcsetattr(fd, TCSANOW, &newtio);
   }
                                                                                
   return fd;
}

int meter_debug(int fd, int count) {
   int i;
   char* buf;
   int rrv = 0;

   buf = (char*)malloc(count + 1);
   if (buf != NULL) {
      bzero(buf, count + 1);
      rrv = read(fd, buf, count);
      for (i = 0; i < count; i++)
         printf("%2x ", buf[i] & 0xff);
      free(buf);
      printf("\n\r");
   }

   return rrv;
}

int meter_nbytes(int fd) {
   int bytes;
   ioctl(fd, FIONREAD, &bytes);
   return bytes;
}

char *meter_name(int type) {
   char *name = NULL;    /* name to return based on meter list */
   int i = 0;            /* index into the meter list */

   while (name == NULL && meters[i].name != NULL) {
      if (meters[i].type == type)
         name = meters[i].name;
      i++;
   }

   return name;
}

int meter_type(char *name) {
   int type = 0;
   int i = 0;

   while (type == 0 && meters[i].name != NULL) {
      if (strcmp(meters[i].name, name) == 0)
         type = meters[i].type;
      i++;
   }

   return type;
}

