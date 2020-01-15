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
 * $Id: netmeter.c,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

/*
 * Read the readings from the meter server for client print out.
 * These routines perform the appropriate actions to the mlogger meter
 * server and return the data.  Most of these routines are stubs to
 * other lower-level implementation routines.  For specific instructions
 * on semantic usage, reference the netmeter.h header file.
 */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "connectsock.h"
#include "meter.h"

extern int errno;

//static int sock;

static int timeout_read_write(int fd, int readfd, int writefd) {
   fd_set set;
   struct timeval timeout;
     
   /* Initialize the file descriptor set. */
   FD_ZERO (&set);
   FD_SET (fd, &set);
     
   /* Initialize the timeout data structure. */
   timeout.tv_sec = 0;
   timeout.tv_usec = 10000;
     
   if (readfd)
       return select(fd+1/*FD_SETSIZE*/, &set, NULL, NULL, &timeout);

   return select(fd+1/*FD_SETSIZE*/, NULL, &set, NULL, &timeout);

}

int netmeter_connect(char *hostname, unsigned short port) {
   int sock;

   sock = tcp_connect_port(hostname, port);

   return sock;
}

int netmeter_close(int sock) {
   int r;

   netmeter_req_t msg = { NETMETER_CMD_QUIT, 0.0, 0, 0, 0 };
   r = write(sock, &msg, sizeof(netmeter_req_t));

   return close(sock);
}

int netmeter_request(int sock, netmeter_req_t *msg, unsigned int *pwr_readings) {
   int r, num_chs, i;
   unsigned int value, buf_len=1024;
   unsigned char buf[1024];
   //struct timeval tv;
   double ts;
//printf("n1, %d\n", timeout_read_write(sock, 0, 1));

//   if (r = timeout_read_write(sock, 0, 1) != 1)
//      return -1;
   r = write(sock, msg, sizeof(netmeter_req_t));
   if (r != sizeof(netmeter_req_t))
      return -2;
   if (msg->cmd == NETMETER_CMD_RESET) /* no reply */
      return r;
/*
   r = read(sock, msg, sizeof(netmeter_req_t));
   if (r != sizeof(netmeter_req_t))
      return -1;
*/
//printf("n2, %d\n", timeout_read_write(sock, 1, 0));
//  if (r = timeout_read_write(sock, 1, 0) != 1)
//      return -3;
  r = read(sock, buf, buf_len);
  if (r < sizeof(netmeter_req_t))
      return -4;
//printf("n3\n");
  memcpy(msg, buf, sizeof(netmeter_req_t));
  //memcpy(&ts, &(msg->last_read), sizeof(struct timeval));
  ts = msg->last_read;
  num_chs = msg->num_chs;
//ts = tv.tv_sec + tv.tv_usec * 1.0e-6;
//printf("%f size(%d), num_chs(%d) %d\n", ts, r, num_chs, sizeof(double));
  if (r != (sizeof(netmeter_req_t) + sizeof(unsigned int)*num_chs))
      return -5;
//printf("n4\n");
  memcpy (pwr_readings, buf + sizeof(netmeter_req_t), num_chs*sizeof(unsigned int));

  //ts = tv.tv_sec + tv.tv_usec * 1.0e-6;
/*
  printf("receiving ..\n %f ", ts);
  for(i=0; i< num_chs; i++)
    printf("%d ", pwr_readings[i]);
  printf("\n");
*/
  return r;
}

int netmeter_convert(netmeter_req_t *msg, mm_t *mm, unsigned int *pwr_readings) {
   int i, num_chs;

   if (msg == NULL || mm == NULL) {
      errno = EINVAL;
      return -1;
   }
   //mm->units = msg->units;
   num_chs = msg->num_chs;
   
   for (i=0; i<num_chs; i++)
   {
      mm[i].units = msg->units;

      if (msg->cmd == NETMETER_CMD_READ)
        mm[i].value = (float)(((unsigned int)pwr_readings[i]) / 100000.0);
      else
        mm[i].value = (float)(unsigned int)pwr_readings[i];
   }

   return 0;
}

