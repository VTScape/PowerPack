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
 * $Id: rs_22812.c,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

/*
 * Original version by:  Sean "Zekat" Scanlon (zekat@mail.com)
 * http://zmeter.sf.net (GPL)
 *
 * Modified to be incorporated in the general meter reading framework.
 */

#include <unistd.h>
#include <errno.h>
#include "rs_22812.h"

extern int errno;

/* meter debugging */
extern int mdebug;

/* baud rate for the serial port */
#define BAUDRATE		B4800

static void cook_digit(LCD_DIGIT *digit);
static void cook_value(RS22812_PACKET* packet);
static void cook_output(RS22812_PACKET* packet);
static unsigned int compute_checksum( RS22812_PACKET *packet);
static void print_bool(char *label, int bool);

static SEG_TAB segtab[] = {
   { 0x00, ' '}, { 0x20, '-'}, { 0xd7, '0'}, { 0x50, '1'}, { 0xb5, '2'},
   { 0xf1, '3'}, { 0x72, '4'}, { 0xe3, '5'}, { 0xe7, '6'}, { 0x51, '7'},
   { 0xf7, '8'}, { 0xf3, '9'}, { 0x87, 'C'}, { 0xa7, 'E'}, { 0x27, 'F'},
   { 0x66, 'h'}, { 0x64, 'n'}, { 0x37, 'P'}, { 0x24, 'r'}, { 0xa6, 't'}
};
static int seg_tab_len = sizeof(segtab) / sizeof(SEG_TAB);

#if 0
static SEG_TAB lettab[] = {
    { 0xd7, 'O'},
    { 0xe3, 'S'}
};
static int let_tab_len = sizeof(lettab) / sizeof(SEG_TAB);
#endif

/* convert LCD segment-representation of digit to an unsigned char */
static void cook_digit(LCD_DIGIT *digit) {
   int i;
   int segments = 0;

   assert(digit);
   if (digit->seg_d) segments |= 0x80;
   if (digit->seg_c) segments |= 0x40;
   if (digit->seg_g) segments |= 0x20;
   if (digit->seg_b) segments |= 0x10;
   /* skip seg_pt */
   if (digit->seg_e) segments |= 0x04;
   if (digit->seg_f) segments |= 0x02;  
   if (digit->seg_a) segments |= 0x01;

   digit->digit = '?'; /* default character */
   for (i = 0; i < seg_tab_len; i++)
      if (segments == segtab[i].seg)
         break;

   if (i < seg_tab_len)
      digit->digit = segtab[i].chr;
}

/* convert digits into floating point value */
static void cook_value(RS22812_PACKET* packet) {
   int  i = 0;
   char digit_str[10];

   assert(packet);
   if (packet->neg)
      digit_str[i++] = '-';
   digit_str[i++] = packet->digit_1.digit;
   if (packet->digit_2.seg_pt)
      digit_str[i++] = '.';
   digit_str[i++] = packet->digit_2.digit;
   if (packet->digit_3.seg_pt)
      digit_str[i++] = '.';
   digit_str[i++] = packet->digit_3.digit;
   if (packet->digit_4.seg_pt)
      digit_str[i++] = '.';
   digit_str[i++] = packet->digit_4.digit;
   digit_str[i] = '\0';
   packet->value = atof(digit_str);
}

/* convert digits and bit flags into output string */
static void cook_output(RS22812_PACKET *packet) {
  int  i;

  assert( packet );
  bzero( &(packet->output), sizeof(packet->output) );

  i = 0;
  if (packet->neg )                       packet->output[i++] = '-';
  packet->output[i++] = packet->digit_1.digit;
  if (packet->digit_2.seg_pt)             packet->output[i++] = '.';
  packet->output[i++] = packet->digit_2.digit;
  if (packet->digit_3.seg_pt)             packet->output[i++] = '.';
  packet->output[i++] = packet->digit_3.digit;
  if (packet->digit_4.seg_pt)             packet->output[i++] = '.';
  packet->output[i++] = packet->digit_4.digit;
  packet->output[i++] = ' ';

  /* multiplier: max: 1 byte */
  if      (packet->mega)  packet->output[i++] = 'M';
  else if (packet->kilo)  packet->output[i++] = 'K';
  else if (packet->milli) packet->output[i++] = 'm';
  else if (packet->micro) packet->output[i++] = 'u';
  else if (packet->nano)  packet->output[i++] = 'n';

  /* unit: max 4 bytes */
  if      (packet->farads)  packet->output[i++] = 'F';
  else if (packet->amps)    packet->output[i++] = 'A';
  else if (packet->volts)   packet->output[i++] = 'V';
  else if (packet->s)       packet->output[i++] = 'S';
  else if (packet->percent) packet->output[i++] = '%';
  else if (packet->hz)      strcat( packet->output, "Hz" );
  else if (packet->dbm)     strcat( packet->output, "dbm" );
  else if (packet->hfe)     strcat( packet->output, "hFE" );
  else if (packet->ohms)    strcat( packet->output, "Ohms" );

  /* other indicators: max 38 bytes */
  if (packet->ac)   strcat( packet->output, " AC" );
  if (packet->rel)  strcat( packet->output, " Rel" );
  
  /* max and min are mutually exclusive */
  if      (packet->min) strcat( packet->output, " Min" );
  else if (packet->max) strcat( packet->output, " Max" );
  
  if (packet->beep)  strcat( packet->output, " Continuity" );
  if (packet->_auto) strcat( packet->output, " Auto Range" );
  if (packet->hold)  strcat( packet->output, " Hold" );
  if (packet->diode) strcat( packet->output, " Diode" );
  if (packet->rs232) strcat( packet->output, " RS232" );

  strcat( packet->output, "\n" ); /* be sure there is a terminating null */
}

/* compute checksum for a packet */
static unsigned int compute_checksum(RS22812_PACKET *packet) {
   int i;
   unsigned int checksum = 0;

   assert(packet);
   for (i = 0; i < RAW_PACKET_LENGTH - 1; i++)
      checksum += packet->raw[i];
   checksum += 0x57;
   checksum &= 0xff;

   return checksum;
}

/* used in dump to print a boolean value (e.g. a bit of a bitfield) */
static void print_bool(char* label, int bool) {
   printf("%s:%c  ", label, bool ? '1' : '0');
}

/* "constructor" */
RS22812_PACKET* rs_22812_packet_new() {
   RS22812_PACKET* packet = (RS22812_PACKET *)malloc(sizeof(RS22812_PACKET));
   bzero(packet, sizeof(RS22812_PACKET));

   return packet;
}

/* "destructor" */
void rs_22812_packet_delete(RS22812_PACKET *packet) {
   assert(packet);
   free(packet);
}

/* print a packet dump for debugging */
void rs_22812_packet_dump(RS22812_PACKET *packet) {
  int i;
  assert(packet);

  printf("raw packet:");
  for (i=0; i<RAW_PACKET_LENGTH; i++) {
    printf(" %2x", packet->raw[i]);
  }
  printf("\n");

  /* byte 1 */
  printf("mode:%2d\n", packet->mode);

  /* byte 2 */
  print_bool("hz",      packet->hz);
  print_bool("   ohms", packet->ohms);
  print_bool(" kilo",   packet->kilo);
  print_bool("mega",    packet->mega);
  print_bool("farads",  packet->farads);
  print_bool(" amps",   packet->amps);
  print_bool("volts",   packet->volts);
  print_bool("milli",   packet->milli);
  printf("\n");

  /* byte 3 */
  print_bool("micro",   packet->micro);
  print_bool("nano",    packet->nano);
  print_bool(" dbm",    packet->dbm);
  print_bool(" s",      packet->s);
  print_bool("   percent", packet->percent);
  print_bool("hfe",     packet->hfe);
  print_bool(" rel",    packet->rel);
  print_bool("  min",   packet->min);
  printf("\n");
  
  /* byte 4 */
  print_bool("4d",     packet->digit_4.seg_d);
  print_bool("   4c",  packet->digit_4.seg_c);
  print_bool("   4g",  packet->digit_4.seg_g);
  print_bool("  4b",   packet->digit_4.seg_b);
  print_bool("  pt3",  packet->digit_4.seg_pt);
  print_bool("    4e", packet->digit_4.seg_e);
  print_bool("  4f",   packet->digit_4.seg_f);
  print_bool("   4a",  packet->digit_4.seg_a);
  printf("\n");
  
  /* byte 5 */
  print_bool("3d",     packet->digit_3.seg_d);
  print_bool("   3c",  packet->digit_3.seg_c);
  print_bool("   3g",  packet->digit_3.seg_g);
  print_bool("  3b",   packet->digit_3.seg_b);
  print_bool("  pt2",  packet->digit_3.seg_pt);
  print_bool("    3e", packet->digit_3.seg_e);
  print_bool("  3f",   packet->digit_3.seg_f);
  print_bool("   3a",  packet->digit_3.seg_a);
  printf("\n");
  
  /* byte 6 */
  print_bool("2d",     packet->digit_2.seg_d);
  print_bool("   2c",  packet->digit_2.seg_c);
  print_bool("   2g",  packet->digit_2.seg_g);
  print_bool("  2b",   packet->digit_2.seg_b);
  print_bool("  pt1",  packet->digit_2.seg_pt);
  print_bool("    2e", packet->digit_2.seg_e);
  print_bool("  2f",   packet->digit_2.seg_f);
  print_bool("   2a",  packet->digit_2.seg_a);
  printf("\n");
  
  /* byte 7 */
  print_bool("1d",      packet->digit_1.seg_d);
  print_bool("   1c",   packet->digit_1.seg_c);
  print_bool("   1g",   packet->digit_1.seg_g);
  print_bool("  1b",    packet->digit_1.seg_b);
  print_bool("  [max]", packet->digit_1.seg_pt);
  print_bool("  1e",  packet->digit_1.seg_e);
  print_bool("  1f",    packet->digit_1.seg_f);
  print_bool("   1a",   packet->digit_1.seg_a);
  printf("\n");
  
  /* byte 8 */
  print_bool("beep",    packet->beep);
  print_bool(" diode",  packet->diode);
  print_bool("bat",     packet->bat);
  print_bool(" hold",   packet->hold);
  print_bool("neg",     packet->neg);
  print_bool("    ac",  packet->ac);
  print_bool("  rs232", packet->rs232);
  print_bool("auto",    packet->_auto);
  printf("\n");
  
  /* byte 9 */
  printf("checksum: %2x  ", packet->checksum);
  printf("(computed checksum: %2x)  ", compute_checksum(packet));
  print_bool("max",         packet->max);
  printf("\n");

  /* numeric digits and floating point value */
  printf("d1:%c  d2:%c  d3:%c  d4:%c  ",
    packet->digit_1.digit,
    packet->digit_2.digit,
    packet->digit_3.digit,
    packet->digit_4.digit );
  printf("value:%+3.3f\n",  packet->value);
}

/* read a raw packet from the serial port */
void rs_22812_packet_read(RS22812_PACKET* packet, char* buf) {
  assert(packet);

  bcopy(buf, packet->raw, RAW_PACKET_LENGTH);
}


/* break up raw packet into elements */
void rs_22812_packet_cook(RS22812_PACKET* packet) {
  assert(packet);

  /* byte 1 */
  packet->mode = packet->raw[0];
  
  /* byte 2 */
  packet->hz     = BIT7(packet->raw[1]);
  packet->ohms   = BIT6(packet->raw[1]);
  packet->kilo   = BIT5(packet->raw[1]);
  packet->mega   = BIT4(packet->raw[1]);
  packet->farads = BIT3(packet->raw[1]);
  packet->amps   = BIT2(packet->raw[1]);
  packet->volts  = BIT1(packet->raw[1]);
  packet->milli  = BIT0(packet->raw[1]);
 
  /* byte 3 */
  packet->micro   = BIT7(packet->raw[2]);
  packet->nano    = BIT6(packet->raw[2]);
  packet->dbm     = BIT5(packet->raw[2]);
  packet->s       = BIT4(packet->raw[2]);
  packet->percent = BIT3(packet->raw[2]);
  packet->hfe     = BIT2(packet->raw[2]);
  packet->rel     = BIT1(packet->raw[2]);
  packet->min     = BIT0(packet->raw[2]);
 
  /* byte 4 */
  packet->digit_4.seg_d  = BIT7(packet->raw[3]);
  packet->digit_4.seg_c  = BIT6(packet->raw[3]);
  packet->digit_4.seg_g  = BIT5(packet->raw[3]);
  packet->digit_4.seg_b  = BIT4(packet->raw[3]);
  packet->digit_4.seg_pt = BIT3(packet->raw[3]);
  packet->digit_4.seg_e  = BIT2(packet->raw[3]);
  packet->digit_4.seg_f  = BIT1(packet->raw[3]);
  packet->digit_4.seg_a  = BIT0(packet->raw[3]);
 
  /* byte 5 */
  packet->digit_3.seg_d  = BIT7(packet->raw[4]);
  packet->digit_3.seg_c  = BIT6(packet->raw[4]);
  packet->digit_3.seg_g  = BIT5(packet->raw[4]);
  packet->digit_3.seg_b  = BIT4(packet->raw[4]);
  packet->digit_3.seg_pt = BIT3(packet->raw[4]);
  packet->digit_3.seg_e  = BIT2(packet->raw[4]);
  packet->digit_3.seg_f  = BIT1(packet->raw[4]);
  packet->digit_3.seg_a  = BIT0(packet->raw[4]);
 
  /* byte 6 */
  packet->digit_2.seg_d  = BIT7(packet->raw[5]);
  packet->digit_2.seg_c  = BIT6(packet->raw[5]);
  packet->digit_2.seg_g  = BIT5(packet->raw[5]);
  packet->digit_2.seg_b  = BIT4(packet->raw[5]);
  packet->digit_2.seg_pt = BIT3(packet->raw[5]);
  packet->digit_2.seg_e  = BIT2(packet->raw[5]);
  packet->digit_2.seg_f  = BIT1(packet->raw[5]);
  packet->digit_2.seg_a  = BIT0(packet->raw[5]);
 
  /* byte 7 */
  packet->digit_1.seg_d  = BIT7(packet->raw[6]);
  packet->digit_1.seg_c  = BIT6(packet->raw[6]);
  packet->digit_1.seg_g  = BIT5(packet->raw[6]);
  packet->digit_1.seg_b  = BIT4(packet->raw[6]);
  packet->digit_1.seg_pt = BIT3(packet->raw[6]);
  packet->digit_1.seg_e  = BIT2(packet->raw[6]);
  packet->digit_1.seg_f  = BIT1(packet->raw[6]);
  packet->digit_1.seg_a  = BIT0(packet->raw[6]);
  packet->max            = BIT3(packet->raw[6]);
 
  /* byte 8 */
  packet->beep  = BIT7(packet->raw[7]);
  packet->diode = BIT6(packet->raw[7]);
  packet->bat   = BIT5(packet->raw[7]);
  packet->hold  = BIT4(packet->raw[7]);
  packet->neg   = BIT3(packet->raw[7]);
  packet->ac    = BIT2(packet->raw[7]);
  packet->rs232 = BIT1(packet->raw[7]);
  packet->_auto = BIT0(packet->raw[7]);
 
  /* byte 9 */
  packet->checksum = packet->raw[8];
  
  /* convert LCD segments to digits */
  cook_digit( &(packet->digit_1) );
  cook_digit( &(packet->digit_2) );
  cook_digit( &(packet->digit_3) );
  cook_digit( &(packet->digit_4) );

  /* numeric display value */
  cook_value( packet );
  
  /* output string */
  cook_output( packet );
}


/* print ASCII contents of packet to buf */
void rs_22812_packet_sprint(RS22812_PACKET* packet, char* buf, int bufsize) {
   assert(packet);
   assert(buf);

   /* print value and units to output buf */
   buf = strncpy(buf, packet->output, bufsize);
   buf[bufsize - 1] = '\0';  /* force a null-terminator */
}

/* null terminate string buf is parsed for data */
int rs_22812_parse(char *buf, int len, mm_t *mm) {
   unsigned int units = 0;
   float value;
   char tmp[DBUFSZ];

   if (buf == NULL || mm == NULL) {
      errno = EINVAL;
      return -1;
   }

   if (sscanf(buf, "%f %s", &value, tmp) != 2) {
      fprintf(stderr, "rs_22812_parse: unable to obtain reading\n");
      return -1;
   }
   if (strncmp(tmp, "A", 2) == 0)
      units |= MUNITS_AMPS;
   mm[0].value = value;
   mm[0].units = units;

   return 0;
}

int rs_22812_read(int fd, char *buf, int len, int *status) {
   char* rbuf;         /* read buffer */
   int   n;
   int   rrv;
   RS22812_PACKET* pak;

   *status = 0;
   if ((n = meter_nbytes(fd)) < 0)
      fprintf(stderr, "rs_22812_read: negative byte count\n");
   else {
      if (mdebug)
         printf("rs_22812_read: reading %d bytes\n", n);
      if ((rbuf = (char *)malloc(n + 1)) != NULL) {
         bzero(rbuf, n + 1);
         rrv = read(fd, rbuf, n);
      }
      if (rrv > 0) {
         pak = rs_22812_packet_new();
         rs_22812_packet_read(pak, rbuf);
         rs_22812_packet_cook(pak);
         //rs_22812_packet_dump(pak);
         rs_22812_packet_sprint(pak, buf, DBUFSZ);
         rs_22812_packet_delete(pak);
      }
      if (rbuf != NULL)
         free(rbuf);
   }

   return rrv;
}

/* add the appropriate additional attributes */
int rs_22812_init(struct termios* newtio) {
   newtio->c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
   newtio->c_iflag = IGNPAR;
   newtio->c_oflag = 0;

   /* set input mode (non-canonical, no echo,...) */
   newtio->c_lflag = 0;

   newtio->c_cc[VTIME] = 0; /* inter-character timer unused */
   newtio->c_cc[VMIN] = 9 /*5*/; /* blocking read until 5 chars received */

   if (mdebug)
      printf("rs_22812_init: finished\n");
   return 0;
}

speed_t rs_22812_baudrate() {
   return BAUDRATE;
}

