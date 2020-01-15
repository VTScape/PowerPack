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
 * $Id: watts_up.h,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

#ifndef __WATTS_UP_H
#define __WATTS_UP_H

int watts_up_query(int fd);
int watts_up_send(int fd, char *cmd, int len);
int watts_up_read(int fd, char *buf, int len, int *status);
int watts_up_init(struct termios *newtio);
int watts_up_parse(char *buf, int len, mm_t *mm);
speed_t watts_up_baudrate();

//Elson
int wattsup_usb_meter_init(char *devicename, struct termios *oldtio, int type);
int watts_up_usb_meter_read(int fd, int type, mm_t *mm);
int watts_up_usb_meter_read_close();

#endif
