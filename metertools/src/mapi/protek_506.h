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
 * $Id: protek_506.h,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

#ifndef __PROTEK_506_H
#define __PROTEK_506_H

int protek_506_query(int fd);
int protek_506_send(int fd, char *cmd, int len);
int protek_506_read(int fd, char *buf, int len, int *status);
int protek_506_init(struct termios *newtio);
int protek_506_parse(char *buf, int len, mm_t *mm);
speed_t protek_506_baudrate();

#endif
