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
 * $Id: mclient.c,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

/*
 * This is the network meter client.  Using this client and an appropriate
 * server running mlogger, the client will communicate to the server and
 * return the appropriate values.  Numerous options exist on the formatting
 * of the output and other connection related options.
 */

#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif
#include "meter.h"

#define DEFAULT_CMD		NETMETER_CMD_ENERGY
#define DEFAULT_COUNT		1
#define DEFAULT_DELAY		1000
#define DEFAULT_ACCURACY	0

extern char *optarg;
extern int optind, opterr, optopt;

static int finished = 0;

static int baseconv = 0;
static int units = 1;
static int accuracy = DEFAULT_ACCURACY;

/* prototypes */
unsigned int get_data(int count, int delay, char *hostname, int cmd,
   unsigned short port, int timestamp);
void show_usage(char *argv[]);
void signal_done(int sig);

int main(int argc, char *argv[]) {
   char *hostname = METER_HOST;       /* hostname to connect to */
   char c;                            /* character option */
   int count = DEFAULT_COUNT;         /* number of times to get data */
   int delay = DEFAULT_DELAY;         /* delay in milliseconds */
   int cmd = DEFAULT_CMD;             /* command to send */
   unsigned short port = METER_TCP_PORT;  /* tcp port number */
   int timestamp = 0;                 /* timestamp the results */
   int verbose = 0;                   /* set to 1 for verbosity */
   unsigned int readings = 0;         /* number of readings taken */

   opterr = 0;
   while ((c = getopt(argc, argv, "gA:BUvTd:c:wjH:p:h")) != -1) {
      switch (c) {
         case 'A':
            accuracy = atoi(optarg);
            break;
         case 'B':
            baseconv = 1;
            break;
         case 'U':
            units = 0;
            break;
         case 'T':
            timestamp = 1;
            break;
         case 'p':
            port = atoi(optarg);
            break;
         case 'd':
            delay = atoi(optarg);
            break;
         case 'c':
            count = atoi(optarg);
            break;
         case 'H':
            if ((hostname = strdup(optarg)) == NULL) {
               fprintf(stderr, "out of memory");
               exit(1);
            }
            break;
         case 'j':
            cmd = NETMETER_CMD_ENERGY;
            break;
         case 'w':
            cmd = NETMETER_CMD_POWER;
            break;
         case 'g':
            cmd = NETMETER_CMD_READ;
            break;
         case 'v':
            verbose = 1;
            break;
         case 'h':
            show_usage(argv);
            exit(0);
         default:
            fprintf(stderr, "unknown option: '%c'\n", optopt);
            exit(1);
      }
   }
   signal(SIGINT, signal_done);
   readings = get_data(count, delay, hostname, cmd, port, timestamp);
   if (verbose)
      printf("received %u readings\n", readings);

   return 0;
}

void show_usage(char *argv[]) {
   char *name = basename(argv[0]);
   printf("Usage: %s [-H host] [-p port] [-c count] [-d delay] [-T] [-v] [-B] [-U] -w|-j|-g\n", name);
   printf("       %s -h\n", name);
   printf("\t-H host    Server hostname to connect to [%s]\n",
      METER_HOST);
   printf("\t-p port    Server port to connect to [%d]\n", METER_TCP_PORT);
   printf("\t-A digits  Print value to the given number of digits [%d]\n",
      DEFAULT_ACCURACY);
   printf("\t-T         Timestamp the results, default is no\n");
   printf("\t-g         Get the current display value\n");
   printf("\t-w         Poll server for power (watts)\n");
   printf("\t-j         Poll server for energy (joules) [default]\n");
   printf("\t-c count   Number of times to poll, 0 to disable [%d]\n",
      DEFAULT_COUNT);
   printf("\t-d delay   Amount of delay time (msec) between polls [%d]\n",
      DEFAULT_DELAY);
   printf("\t-B         Convert to the base units (no milli, mega, etc.)\n");
   printf("\t-U         Don't print units with the value, default is yes\n");
   printf("\t-v         Be verbose, default is to only print readings\n");
   printf("\t-h         This help screen.\n");
}

void signal_done(int sig) {
   finished = 1;
}

unsigned int get_data(int count, int delay, char *hostname, int cmd,
      unsigned short port, int timestamp) {
   netmeter_req_t msg;        /* for request to meter server */
   struct timespec ts;        /* for delays */
   struct timeval start, now; /* used for timestamps */
   unsigned int i = 0;        /* current number of requests received */
   //mm_t mm;                   /* meter measurement */
   mm_t mm[32];
   unsigned char *prefix;     /* prefix (if any) for the measurement */
   char format[5];            /* format specification for output */

   //char pkg[1024];
   unsigned int ch_pwr_readings[32];
   int r, j;
   int sock;

   if ((sock = netmeter_connect(hostname, port)) == -1)
      return -1;
   msg.cmd = cmd;
   //msg.value = 0;
   ts.tv_sec = delay / 1000;
   ts.tv_nsec = (delay % 1000) * 1000000;
   gettimeofday(&start, NULL);
   snprintf(format, sizeof(format), "%%.%df", accuracy);
   while ((count == 0 || (i < count)) && !finished) {
      i++;
      //netmeter_request(&msg);
      r = netmeter_request(sock, &msg, ch_pwr_readings);
      if (r<0) printf("error while requesting netmeter. erron[%d]\n", r);
      netmeter_convert(&msg, mm, ch_pwr_readings);

      if (timestamp) {
         gettimeofday(&now, NULL);
         printf("%u %.3f ", (unsigned int)now.tv_sec,
            (now.tv_sec - start.tv_sec) + (now.tv_usec / 1000000.0f));
      }
/*
      if (baseconv)
         meter_to_base(&mm, &mm);
      printf(format, mm.value);
      if (units) {
         printf(" ");
         if ((prefix = meter_units_prefix(&mm)) != NULL)
            printf("%s", prefix);
         printf("%s\n", meter_units_measure(&mm));
      } else {
         printf("\n");
      }
*/
      for (j=0; j<msg.num_chs; j++)
         printf("%d ", ch_pwr_readings[j]);
      printf("\n");

      if (count == 0 || (i != count))
         nanosleep(&ts, NULL);
   }
   netmeter_close(sock);

   return i;
}

