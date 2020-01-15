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
 * $Id: connectsock.h,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

/*
 * Header include file for network socket connections.  Both tcp and
 * named pipes are handled.  Note that all routines either return -1
 * if there is an error or the socket number if successful.
 */

#ifndef _CONNECTSOCK_H
#define _CONNECTSOCK_H

/* lookup the named service and connect to it */
int tcp_connect_serv(char *host, char *service);

/* connect to the indicated host and port */
int tcp_connect_port(char *host, unsigned short port);

/* connect to the indicated named pipe */
int pipe_connect(char *name);

#endif

