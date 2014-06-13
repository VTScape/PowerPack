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
 * $Id: netmeter.h,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

/*
 * The following routines allow you to utilize network-enabled network
 * meter reading functionality.  This is useful if you have multiple
 * meters and need to access them all across the network or have a
 * meter that you want to read periodically in real-time according to a
 * schedule on a different node.  All these methods use the mlogger
 * command line tool to startup the logging on the system the meter
 * is connected to.  The following is an example of compiling and
 * using the below functionality (for compiling add appropriate -I and -L
 * options):
 *
 * compiling example.c using: gcc -lmeter -lsupport -o example example.c
 *
 * #include <stdio.h>
 * #include <sys/socket.h>
 * #include "meter.h"
 *
 * #define READINGS			10
 *
 * int main(int argc, char *argv[]) {
 *    int sock;
 *    netmeter_req_t request;
 *    mm_t mm;
 *
 *    if ((sock = netmeter_connect(NULL, 0) == -1)) {
 *       fprintf(stderr, "unable to connect\n");
 *       return -1;
 *    }
 *    for (i = 0; i < READINGS; i++) {
 *       request.cmd = NETMETER_CMD_READ;
 *       netmeter_request(&request);
 *       netmeter_convert(&request, &mm);
 *       printf("reading: %f %s%s\n", mm.value,
 *          meter_units_prefix(&mm), meter_units_measure(&mm));
 *    }
 *    netmeter_close();
 *
 *    return 0;
 * }
 *
 * Note that you should not just add "netmeter.h" with an include
 * statement as types and definitions are also in meter.h (which
 * includes this file).
 */

#ifndef _NETMETER_H
#define _NETMETER_H

/* meter requests are handled on a tcp port by the server */
#ifndef METER_TCP_PORT
#   define METER_TCP_PORT		44000
#endif

/* default host name for meter requests */
#ifndef METER_HOST
#   define METER_HOST			"172.16.0.240"
#endif

/* return codes from the request */
#define NETMETER_RC_SUCCESS		0
#define NETMETER_RC_FAIL		1

typedef struct {
   unsigned int cmd;      /* command to execute (types defined below) */
   void *value;           /* the value returned from the cmd */
   unsigned int units;    /* the units of the returned data */
   int rc;                /* return code from the call */
} netmeter_req_t;

/* initialize a network connection to a meter server */
/* if port is 0 or host is NULL use the defaults above */
int netmeter_connect(char *hostname, unsigned short port);

/* perform a remote request to the meter server (must be connected) */
/* fill in at least the cmd variable, the output from the request is also */
/* stored in msg and should have a cmd which matches the original */
int netmeter_request(netmeter_req_t *msg);

/* close the network connection to the meter server */
int netmeter_close();

/* given the return data from a request, return the meter measure value */
/* this is a convience method so you need not worry about the network */
/* data types that were used and perform the appropriate casts */
int netmeter_convert(netmeter_req_t *msg, mm_t *mm);

/* remote procedure command types for the meter request */
#define NETMETER_CMD_ENERGY		1   /* joules */
#define NETMETER_CMD_POWER		2   /* milliwatts */
#define NETMETER_CMD_READ		3   /* general read */
#define NETMETER_CMD_RESET		4   /* reset statistics (i.e. joules) */
#define NETMETER_CMD_QUIT		99  /* close connection gracefully */

#endif

