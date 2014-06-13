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
 * $Id: passivesock.h,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

/*
 * The following are generally passive socket routines (used for servers
 * that accept inbound connections) for TCP, UDP, and named pipes.  Most
 * of the routines either return -1 if there is an error or the socket
 * number otherwise.
 */

#ifndef _PASSIVESOCK_H
#define _PASSIVESOCK_H
#include <sys/socket.h>

/* setup and listen to a udp socket */
int udp_passive_serv(char *service);

/* setup and listen to a udp socket */
int udp_passive_port(ushort port);

/* setup and listen to a tcp socket with the given service name */
int tcp_passive_serv(char *service, int backlog);

/* setup and listen to a tcp socket with the provided port number */
int tcp_passive_port(ushort port, int backlog);

/* setup and listen to a named pipe */
int pipe_passive_port(char *name, int backlog);

#endif

