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
 * $Id: meter.h,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

/*
 * This file contains the interfaces and data structures needed to embed
 * the information obtained from multimeters into an application.  These
 * interfaces are also used by the meter logger server (mlogger), the
 * meter command client (msend) and the network client (mclient).  Thus,
 * there are two different ways to use these tools, the first is to run
 * the built-in mlogger and use mclient to query values.  Second, you
 * can create your own custom application (patterned like mlogger) that
 * reads and manipulates (via commands) the meter directly.
 *
 * If you want to utilize the mlogger and the network enabled layer to
 * access its functionality, reference the documentation contained in the
 * netmeter.h header file.  Detailed specifics for network-access are not
 * contained herein although functionality in this file may be needed for
 * some operations (i.e. units conversion, etc.).
 */

#ifndef _METER_H
#define _METER_H

#include <stdio.h>
#include <termios.h>

/* meter measurement or display value */
typedef struct {
   float value;             /* value from the meter */
   unsigned int units;      /* units for the measurement (see above) */
   unsigned int to_sec;     /* timeout in seconds for the measurement */
} mm_t;

#include "netmeter.h"

/* the name, type, read, init, parse, and baudrate variables are required */
typedef struct {
   char *name;                                 /* meter name or short code */
   char *description;                          /* human readable */
   int count;                                  /* # of measures per query */
   int type;                                   /* type code from above */
   int (*query)(int fd);                       /* request measurement (req) */
   int (*send)(int fd, char *cmd, int len);    /* general send routine */
   int (*read)(int fd, char *buf, int len,
      int *status);                            /* general read routine (req) */
   int (*init)(struct termios *newtio);        /* initialization */
   int (*parse)(char *buf, int len, mm_t *mm); /* parse the reading (req) */
   speed_t (*baudrate)();                      /* baud transfer rate (req) */
} meter_t;

/* default serial port if not specified */
#ifndef SERIAL_PORT
#   define SERIAL_PORT			"/dev/ttyS0"
#endif

/* data buffer size for I/O */
#ifndef DBUFSZ
#   define DBUFSZ			256
#endif

/*
 * Internal type code for each supported meter, not all functions
 * are supported for each meter type (i.e. some meters don't support
 * sending arbitrary commands to it).  If you need a different meter
 * then feel free to donate one to the cause.
 */
#define METER_FLUKE_189			1
#define METER_PROTEK_506		2
#define METER_RS_22812			3
#define METER_WATTS_UP			4

/* timeout while reading data from a meter */
#ifndef METER_TIMEOUT
#   define METER_TIMEOUT		5
#endif

/* default meter type */
#ifndef METER_TYPE
#   define METER_TYPE			METER_WATTS_UP
#endif
                                                                                
/*
 * Measurements that are reported back are generally in the display
 * representation.  So when doing arbitrary readings you have to
 * know both the units and the scale but for simplicity we will
 * track both in the units member for mm_t data variables.  Note that
 * in the conversion routines below, many overflow conditions are
 * not inherently handled so make sure you allocate appropriate data
 * types as needed.
 */
#define MUNITS_AC			0x000001
#define MUNITS_DC			0x000002
#define MUNITS_AMPS			0x000004
#define MUNITS_VOLTS			0x000008
#define MUNITS_CELCIUS			0x000010
#define MUNITS_FAHRENHEIT		0x000020
#define MUNITS_DECIBELS			0x000040
#define MUNITS_OHMS			0x000080
#define MUNITS_HZ			0x000100
#define MUNITS_JOULES			0x000200
#define MUNITS_WATTS			0x000400
#define MUNITS_POWERFACTOR		0x000800
#define MUNITS_NANO			0x010000
#define MUNITS_MICRO			0x020000
#define MUNITS_MILLI			0x040000
#define MUNITS_MEGA			0x080000
#define MUNITS_GIGA			0x100000

/* conversion routines and helper functions for printing and unit handling */
static inline char *meter_units_prefix(mm_t *mm) {
   char *prefix = "";

   if (mm == NULL) {
      prefix = NULL;
   } else if (mm->units & MUNITS_NANO) {
      prefix = "nano";
   } else if (mm->units & MUNITS_MICRO) {
      prefix = "micro";
   } else if (mm->units & MUNITS_MILLI) {
      prefix = "milli";
   } else if (mm->units & MUNITS_MEGA) {
      prefix = "mega";
   } else if (mm->units & MUNITS_GIGA) {
      prefix = "giga";
   }

   return prefix;
}

static inline char *meter_units_measure(mm_t *mm) {
   char *name;

   if (mm == NULL) {
      name = NULL;
   } else if (mm->units & MUNITS_AMPS) {
      name = "amps";
   } else if (mm->units & MUNITS_VOLTS) {
      name = "volts";
   } else if (mm->units & MUNITS_WATTS) {
      name = "watts";
   } else if (mm->units & MUNITS_JOULES) {
      name = "joules";
   } else if (mm->units & MUNITS_CELCIUS) {
      name = "celcius";
   } else if (mm->units & MUNITS_FAHRENHEIT) {
      name = "fahrenheit";
   } else if (mm->units & MUNITS_DECIBELS) {
      name = "decibels";
   } else if (mm->units & MUNITS_OHMS) {
      name = "ohms";
   } else if (mm->units & MUNITS_HZ) {
      name = "hertz";
   } else if (mm->units & MUNITS_POWERFACTOR) {
      name = "pf";
   }

   return name;
}

/* based on the current value, convert to a new base and store in mmnew */
static inline float meter_to_base(mm_t *mmold, mm_t *mmnew) {
   float value;
   int units = 0;

   if (mmold == NULL) {
      value = 0.0;
   } else if (mmold->units & MUNITS_MILLI) {
      value = mmold->value / 1000.0;
      units = mmold->units ^ MUNITS_MILLI;
   } else if (mmold->units & MUNITS_MICRO) {
      value = mmold->value / 1000000.0;
      units = mmold->units ^ MUNITS_MICRO;
   } else if (mmold->units & MUNITS_NANO) {
      value = mmold->value / 1000000000.0;
      units = mmold->units ^ MUNITS_NANO;
   } else if (mmold->units & MUNITS_MEGA) {
      value = mmold->value * 1000000.0;
      units = mmold->units ^ MUNITS_MEGA;
   } else if (mmold->units & MUNITS_GIGA) {
      value = mmold->value * 1000000000.0;
      units = mmold->units ^ MUNITS_GIGA;
   } else { /* unknown */
      value = mmold->value;
      units = mmold->units;
   }
   if (mmnew != NULL) { /* save to new measurement */
      mmnew->value = value;
      mmnew->units = units;
   }

   return value;
}

static inline float meter_to_milli(mm_t *mm) {
   float value;

   if (mm == NULL)
      value = 0.0;
   else if (mm->units & MUNITS_MILLI)
      value = mm->value;
   else {
      value = meter_to_base(mm, NULL);
      value *= 1000.0;
   }

   return value;
}

static inline float meter_to_mega(mm_t *mm) {
   float value;

   if (mm == NULL)
      value = 0.0;
   else if (mm->units & MUNITS_MEGA)
      value = mm->value;
   else {
      value = meter_to_base(mm, NULL);
      value /= 1000000.0;
   }

   return value;
}

static inline float meter_to_micro(mm_t *mm) {
   float value;

   if (mm == NULL)
      value = 0.0;
   else if (mm->units & MUNITS_MICRO)
      value = mm->value;
   else {
      value = meter_to_base(mm, NULL);
      value *= 1000000.0;
   }

   return value;
}

static inline float meter_to_nano(mm_t *mm) {
   float value;

   if (mm == NULL)
      value = 0.0;
   else if (mm->units & MUNITS_NANO)
      value = mm->value;
   else {
      value = meter_to_base(mm, NULL);
      value *= 1000000000.0;
   }

   return value;
}

static inline float meter_to_giga(mm_t *mm) {
   float value;

   if (mm == NULL)
      value = 0.0;
   else if (mm->units & MUNITS_GIGA)
      value = mm->value;
   else {
      value = meter_to_base(mm, NULL);
      value /= 1000000000.0;
   }

   return value;
}

/* allocate an array of mm measures (return # in array or -1 if error) */
int meter_alloc_mms(int type, mm_t **mm);

/* general setup routine for serial ports */
int meter_init(char *devicename, struct termios *oldtio, int type);

/* shutdown routine to restore port settings */
int meter_close(int fd, struct termios *oldtio);

/* debug routine to read count bytes from file descriptor fd */
int meter_debug(int fd, int count);

/* return the meter name based on the type */
char *meter_name(int type);

/* return the meter type based on the name */
int meter_type(char *name);

/* read the given measurement from the meter */
int meter_read(int fd, int type, mm_t *mm);

/* send the indicated command to the meter */
int meter_command(int fd, int type, char *sbuf, int lsbuf, char *rbuf,
   int lrbuf);

/* lookup the meter information from those that are configured */
meter_t *meter_lookup(int type);

/* return the potential number of bytes readable */
int meter_nbytes(int fd);

#endif

