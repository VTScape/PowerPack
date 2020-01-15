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
 * $Id: passivesock.c,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include "passivesock.h"

extern int errno;

/* local prototypes */
static int listen_passive(ushort port, char *protocol, int backlog);

int pipe_passive(char *name, int backlog) {
   int sock;
   struct sockaddr_un server;

   if (name == NULL) {
      errno = EINVAL;
      return -1;
   }
   if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
      fprintf(stderr, "can't create socket: %s\n", strerror(errno));
      return -1;
   }
   server.sun_family = AF_UNIX;
   strncpy(server.sun_path, name, sizeof(server.sun_path) - 1);
   if (bind(sock, (struct sockaddr *)&server,
         sizeof(struct sockaddr_un)) == -1) {
      fprintf(stderr, "can't bind to pipe %s: %s\n", name, strerror(errno));
      return -1;
   }
   if (listen(sock, backlog) == -1) {
      fprintf(stderr, "can't listen to %s: %s\n", name, strerror(errno));
      return -1;
   }

   return sock;
}

int udp_passive_port(ushort port) {
   return listen_passive(htons(port), "udp", 0);
}

int udp_passive_serv(char *service) {
   struct servent *pse;
   u_short port;

   if ((pse = getservbyname(service, "udp")) != NULL)
      port = pse->s_port;
   else if ((port = htons((u_short)atoi(service))) == 0) {
      fprintf(stderr, "can't get \"%s\" service entry\n", service);
      return -1;
   }

   return listen_passive(port, "udp", 0);
}

int tcp_passive_port(ushort port, int backlog) {
   return listen_passive(htons(port), "tcp", backlog);
}

int tcp_passive_serv(char *service, int backlog) {
   struct servent *pse;
   u_short port;

   if ((pse = getservbyname(service, "tcp")) != NULL)
      port = pse->s_port;
   else if ((port = htons((u_short)atoi(service))) == 0) {
      fprintf(stderr, "can't get \"%s\" service entry\n", service);
      return -1;
   }

   return listen_passive(port, "tcp", backlog);
}

/* internal routine to do the real work for tcp and udp sockets */
static int listen_passive(ushort port, char *protocol, int backlog) {
   struct protoent *ppe;
   struct sockaddr_in sin;
   int sock, type;

   bzero((char *)&sin, sizeof(sin));
   sin.sin_family = AF_INET;
   sin.sin_addr.s_addr = INADDR_ANY;
   sin.sin_port = port;
   if ((ppe = getprotobyname(protocol)) == 0) {
      fprintf(stderr, "can't get \"%s\" protocol entry\n",protocol);
      return -1;
   }
   if (strcmp(protocol, "udp") == 0)
      type = SOCK_DGRAM;
   else
      type = SOCK_STREAM;
   if ((sock = socket(PF_INET, type, ppe->p_proto)) == -1) {
      fprintf(stderr, "can't create socket: %s\n", strerror(errno));
      return -1;
   }
   if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
      fprintf(stderr, "can't bind to port %d: %s\n", ntohs(port),
         strerror(errno));
      return -1;
   }
   if ((type == SOCK_STREAM) && (listen(sock, backlog) == -1)) {
      fprintf(stderr, "can't listen port %d: %s\n", ntohs(port),
         strerror(errno));
      return -1;
   }

   return sock;
}

