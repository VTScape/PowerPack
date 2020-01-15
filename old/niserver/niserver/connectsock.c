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
 * $Id: connectsock.c,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

#include <stdio.h>
#if 0
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include <stdlib.h>
//#include <errno.h>
//#include <strings.h>
#include <winsock.h>
#include "connectsock.h"

//extern int errno;

/* local prototypes */
static int tcp_connect(char *host, unsigned short port, char *protocol);

int tcp_connect_serv(char *host, char *service) {
#if 0
   struct servent *pse;
   unsigned short port = 0;

   if (service == NULL || host == NULL) {
      //errno = EINVAL;
      return -1;
   }
   if ((pse = getservbyname(service, "tcp")) != NULL)
      port = pse->s_port;
   else if ((port = htons((u_short)atoi(service))) == 0) {
      fprintf(stderr, "can't get \"%s\" service entry\n", service);
      return -1;
   }

   return tcp_connect(host, port, "tcp");
#endif
}

/* connect to the indicated host and port */
int tcp_connect_port(char *host, unsigned short port) {
   return tcp_connect(host, htons(port), "tcp");
}

/* connect to the indicated named pipe and return the socket number */
int pipe_connect(char *name) {
#if 0
   struct sockaddr_un server;
   int sock;

   if (name == NULL) {
      //errno = EINVAL;
      return -1;
   }
   if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
      //fprintf(stderr, "can't create socket: %s\n",strerror(errno));
      return -1;
   }
   server.sun_family = AF_UNIX;
   strncpy(server.sun_path, name, sizeof(server.sun_path) - 1);
   if (connect(sock, (struct sockaddr *)&server,
         sizeof(struct sockaddr_un)) == -1) {
      //fprintf(stderr, "can't connect to pipe %s: %s\n", name, strerror(errno));
      //errno = EPIPE;
      return -1;
   }

   return sock;
#endif
}

/* connect to the indicated host on the provided port using protocol */
static int tcp_connect(char *host, unsigned short port, char *protocol) {
   struct hostent *phe;
   struct protoent *ppe;
   struct sockaddr_in sin;
   int sock, type;

   sin.sin_family = AF_INET;
   sin.sin_port = port;
   if ((phe = gethostbyname(host)) != NULL)
      memcpy((char *)&sin.sin_addr, phe->h_addr, phe->h_length);
   else if ((sin.sin_addr.s_addr = inet_addr(host)) == -1) {
      fprintf(stderr, "can't get \"%s\" host entry\n", host);
      return -1;
   }
   if ((ppe = getprotobyname(protocol)) == 0) {
      fprintf(stderr, "can't get \"%s\" protocol entry\n",protocol);
      return -1;
   }
   if (strcmp(protocol, "udp") == 0)
      type = SOCK_DGRAM;
   else
      type = SOCK_STREAM;
   if ((sock = socket(PF_INET, type, ppe->p_proto)) < 0) {
      //fprintf(stderr, "can't create socket: %s\n",strerror(errno));
      return -1;
   } 
   if (connect(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
      //fprintf(stderr, "can't connect to %s,%d: %s\n", host, ntohs(port), strerror(errno));
      return -1;
   }

   return sock;
}

