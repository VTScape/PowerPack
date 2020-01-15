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
 * $Id: rs_22812.h,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

#ifndef _RS_22812_H
#define _RS_22812_H

/* #define NDEBUG */  /* uncomment NDEBUG to disable asserts */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "meter.h"

/* length of raw packet byte array */
#define RAW_PACKET_LENGTH 9

/* length of output char array */
#define OUTPUT_LENGTH 256

/* the 26 display modes */
#define MOD_DC_V 0
#define MOD_AC_V 1
#define MOD_DC_UA 2
#define MOD_DC_MA 3
#define MOD_DC_A 4
#define MOD_AC_UA 5
#define MOD_AC_MA 6
#define MOD_AC_A 7
#define MOD_OHM 8
#define MOD_CAP 9
#define MOD_HZ 10
#define MOD_NET_HZ 11
#define MOD_AMP_HZ 12
#define MOD_DUTY 13
#define MOD_NET_DUTY 14
#define MOD_AMP_DUTY 15
#define MOD_WIDTH 16
#define MOD_NET_WIDTH 17
#define MOD_AMP_WIDTH 18
#define MOD_DIODE 19
#define MOD_CONT 20
#define MOD_HFE 21
#define MOD_LOGIC 22
#define MOD_DBM 23
#define MOD_EF 24
#define MOD_TEMP 25

/* macros to return one bit of a byte */
#define BIT0(x) ((x) & 0x1)
#define BIT1(x) (((x) >> 1) & 0x1)
#define BIT2(x) (((x) >> 2) & 0x1)
#define BIT3(x) (((x) >> 3) & 0x1)
#define BIT4(x) (((x) >> 4) & 0x1)
#define BIT5(x) (((x) >> 5) & 0x1)
#define BIT6(x) (((x) >> 6) & 0x1)
#define BIT7(x) (((x) >> 7) & 0x1)

typedef struct {
   unsigned int seg;
   char chr;
} SEG_TAB;


/* LCD 7-segment mapping ASCII art
 *         A
 *        ---
 *     F |   | B
 *       | G |
 *        ---
 *     E |   | C
 *       |---|    PT
 *         D
 */
typedef struct {
   int seg_d:1;
   int seg_c:1;
   int seg_g:1;
   int seg_b:1;
   int seg_pt:1;
   int seg_e:1;
   int seg_f:1;
   int seg_a:1;
   unsigned char digit;
} LCD_DIGIT;


typedef struct {
   /* raw packet */
   unsigned char raw[RAW_PACKET_LENGTH];
  
   /* decoded packet */
   /* byte 1 */
   unsigned char mode;
  
   /* byte 2 */
   int hz:1;
   int ohms:1;
   int kilo:1;
   int mega:1;
   int farads:1;
   int amps:1;
   int volts:1;
   int milli:1;
  
   /* byte 3 */
   int micro:1;
   int nano:1;
   int dbm:1;
   int s:1;
   int percent:1;
   int hfe:1;
   int rel:1;
   int min:1;

   /* byte 4 */
   LCD_DIGIT     digit_4;

   /* byte 5 */
   LCD_DIGIT     digit_3;

   /* byte 6 */
   LCD_DIGIT     digit_2;

   /* byte 7 */
   LCD_DIGIT     digit_1; /* seg_pt in digit_1 is really max (below) */
   int           max:1;

    /* byte 8 */
   int beep:1;
   int diode:1;
   int bat:1;
   int hold:1;
   int neg:1;
   int ac:1;
   int rs232:1;
   int _auto:1;

   /* byte 9 */
   int           checksum;
  
   /* the numeric display value */
   float         value;
  
   /* the text output string */
   char          output[OUTPUT_LENGTH];
} RS22812_PACKET;

/* interface methods */
RS22812_PACKET* rs_22812_packet_new(void);
void rs_22812_packet_delete(RS22812_PACKET* packet);
void rs_22812_packet_dump(RS22812_PACKET* packet);  /* debugging */
void rs_22812_packet_read(RS22812_PACKET* packet, char* buf);
void rs_22812_packet_cook(RS22812_PACKET* packet);
void rs_22812_packet_sprint(RS22812_PACKET* packet, char* buf,
   int bufsize );
int rs_22812_init(struct termios *newtio);
int rs_22812_read(int fd, char *buf, int len, int *status);
int rs_22812_parse(char *buf, int len, mm_t *mm);
speed_t rs_22812_baudrate();

#endif

