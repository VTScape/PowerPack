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
 * $Id: watts_up.c,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

/*
 * This is the communication interface to the Watts Up Pro watt meter
 * available from Double Educational Products (www.doubleed.com).
 * For details about what is possible, reference the comprotocol.pdf
 * file.  This conforms to the standard meter communication protocol.
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "meter.h"
#include "watts_up.h"

/* serial port baud rate */
//#define BAUDRATE		B38400

/* usb port baud rate */
#define BAUDRATE                B115200

//#define BAUDRATE		B9600

static int preamble = 0;

extern int errno;

#define SEND_CLEAR		"#R,W,0;"

/* query format: E=external, timestamp, sampling interval */
//#define SEND_QUERY		"#L,W,3,E,-,1;"

/* for usb version  */
#define SEND_QUERY              "#L,W,3,E,1,1;"

speed_t watts_up_baudrate() {
   return BAUDRATE;
}

#define Sleep(ms) usleep(1000*(ms))

static int				bReadRC;
static int				OpenRC;
static unsigned long    iBytesRead;
static char				sBuffer[128] = "                                            ";
static char				asyncBuffer[128] = "                                            ";
static int				threadState = 0; 

int SampleMS = 1000;

int m_fd;

// variables to store data passed from async to sync threads
double Vasync, Wasync, Aasync, PFasync;
int VasyncValid, WasyncValid, AasyncValid, PFasyncValid;
int asyncValid = 0;

const int kBufferSize = 1024;

//extern int firstRead;

//Elson
pthread_t athread;

//pthread_t mthread;
pthread_mutexattr_t AsyncMutexAttr;
pthread_mutex_t AsyncMutex;
/*
pthread_mutexattr_t DataMutexAttr;
pthread_mutex_t DataMutex;
pthread_mutex_t MeterReadReadyMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  MeterReadReadyCond  = PTHREAD_COND_INITIALIZER;
*/

int ReadRS232(char buffer[128], int BytesToRead, unsigned long *BytesRead) {

	ssize_t i;

	i = read(m_fd, buffer, BytesToRead);
	// fprintf(stdout,"Bytes read: %d\n", i); fflush(stdout);

	if (i < 0) {
		perror("Read on meter failed with error");
		return 0;
	} else {
		*BytesRead = i;	
		buffer[*BytesRead]=0;
		// fprintf(stdout,"port returns %d bytes: %s\n",*BytesRead,buffer); fflush(stdout);
		return 1;
	}

}

int WriteRS232(char* cmd, int CmdBytes, unsigned long *BytesWritten) {

	ssize_t i;

	i = write(m_fd, cmd, CmdBytes);

	if (i < 0) {
		perror("Write on meter failed with error");
printf("WriteRS232: i=%d\n", i);
		return 0;
	} else {
		// fprintf(stdout,"Bytes written: %d\n", i); fflush(stdout);
		*BytesWritten = i;	
		return 1;
	}
}

int ReadSerial(char buffer[128], int BytesToRead, unsigned long *bytesRead)
{
	unsigned long tempBytes;
	int rc;

	tempBytes = 0;

	rc = ReadRS232( buffer,  BytesToRead,  &tempBytes);
	*bytesRead = tempBytes;

	return rc;
}

int WriteSerial(char* cmd, int CmdBytes, unsigned long *bytesWritten)
{
	unsigned long tempBytes;
	int rc;
	
	rc = WriteRS232( cmd,  CmdBytes,  &tempBytes);
	*bytesWritten = tempBytes;

	return rc;
}

int WriteSetupString(char* cmd)
{
	unsigned long tempBytes = 0;
	int rc, CmdBytes;

	
	fprintf(stderr,"Writing setup string %s...",cmd);
	CmdBytes = strlen(cmd);
	
	rc = WriteSerial( cmd,  CmdBytes,  &tempBytes);
	//*bytesWritten = tempBytes;

	if (!rc) {
		fprintf(stderr,"* Error %d writing setup string, %lu bytes written.\n",rc,tempBytes);
	}
	else {
		fprintf(stderr,"  Success!\n");
	}
	return rc;
}

int WriteAndReadSetup(char* cmd, int delayMS, char buffer[128])
{
	unsigned long tempBytes = 0;
	int rc, CmdBytes;
//	char respBuf[128];
	
	fprintf(stderr,"Writing query string %s...\n",cmd);
	CmdBytes = strlen(cmd);
	
	rc = WriteSerial( cmd,  CmdBytes,  &tempBytes);

	if (!rc) {
		fprintf(stderr,"* Error %d writing setup string, %lu bytes written.\n",rc,tempBytes);
		return rc;
	}
	Sleep(delayMS);

	rc = ReadSerial(buffer, 120, &tempBytes);

	if (!rc) {
	    fprintf(stderr,"read failed during meter init with error %d.\n",rc);
		return 0;
	}

	if (tempBytes < 1) {
		fprintf(stderr, "Zero byte count from meter\n");
		return 0;
	}

	buffer[tempBytes]= 0;   // terminate string

	fprintf(stderr, "response of %lu bytes: %s\n", tempBytes, buffer);


	return rc;
}

int ReadRS232Term(char buffer[128], int BytesToRead, unsigned long *BytesRead, char terminator) {

  ssize_t i;

  struct timeval t1,t2;

  int delta;

  static int generation=0;

  char *p = buffer;

  int spaceleft = BytesToRead;



  int j;



  memset(buffer,0,sizeof(buffer));



  j = 0;

  // we could probably be more sophisticated in how we read from the

  // device, but for initial bring-up, one byte at a time will be

  // known to work

  do {

    gettimeofday(&t1,NULL);

    i = read(m_fd,p,spaceleft);

    gettimeofday(&t2,NULL);

    if (i > 0) {

      j += i;

      p += i;

      spaceleft -= i;

}

    if (i == 0) {

      // I don't think this should ever happen, and if it does with a

      // timeout of one second, I don't think there is much chance of

      // things being happy again

      if (t2.tv_usec < t1.tv_usec) {

	t2.tv_sec -= 1;

	t2.tv_usec += 1000000;

      }

      delta = t2.tv_usec - t1.tv_usec;

      delta += (t2.tv_sec - t1.tv_sec)*1000000;

      // printf("Timeout after %d usec starting %d.%06d gen %d\n",delta,t1.tv_sec,t1.tv_usec,generation);

      continue;

      // return false;

    }

    if (i < 0) {

      perror("read");

      continue;

      //return false;

    }

  } while ((buffer[j-1] != terminator) && (spaceleft > 0)) ;

  buffer[j] = 0;

  *BytesRead = j;

  generation++;

  return 1;



}

int ReadSerialTerm(char buffer[128], int BytesToRead, unsigned long *bytesRead, char terminator)
{
	unsigned long tempBytes;
	int rc;

	tempBytes = 0;

	rc = ReadRS232Term( buffer,  BytesToRead,  &tempBytes, terminator);
	*bytesRead = tempBytes;

	return rc;
}

int LockBuffers() {

	int rc;

	 if ((rc = pthread_mutex_lock (&AsyncMutex)) != 0) {
		fprintf(stderr,"failed to get mutex lock!\n" );
        return 0;
   }
		
	return 1;
}



int UnlockBuffers() {

	int rc;
	if ((rc = pthread_mutex_unlock (&AsyncMutex)) != 0) {
		fprintf(stderr,"Failed to unlock mutex!\n" );
		return 0;
    }
	
	return 1;
}

void* AsyncReadThread( void *lpParam) {

    int  i, offset, semicolon;
  
	while (threadState == 1) {
		semicolon = 0;
		offset = 0;
		for (i=0; i < 127; i++) { sBuffer[i] = 0;}
 
		while (!semicolon){

			bReadRC = ReadSerialTerm(sBuffer + offset, 200, &iBytesRead, '\n');
			if ((!bReadRC)) {
				fprintf(stderr,"read of WattsUp failed\n");
					// fix this
					//LogWatts(-1.0, 0); LogVolts(-1.0, 0); LogAmps(-1.0,0); LogPF(-1.0,0);
					// return false;
			}
		//	 fprintf(stderr,"%d bytes read\n",iBytesRead);
			if (iBytesRead > 0) {
				offset += iBytesRead;
				sBuffer[offset] = '\0';
			}
			if (strchr(sBuffer,';')) {
		//		fprintf(stderr,"found semicolon\n");
				semicolon = 1;
			}
			Sleep(50);
		}

		// fprintf(stderr, "meter: %s\n", sBuffer);
		// move buffer to shared area under lock, mark it valid
		if(LockBuffers()){
			strcpy(asyncBuffer, sBuffer);
			asyncValid ++;
			// avoid overflow when ptd is idle
			if (asyncValid > 100000) asyncValid = 100;
			UnlockBuffers();
		}

		Sleep(50);
    }  // end of while threadState == 1

	return (void *)1;
}

int CloseRS232() {

	close(m_fd);

	return 1;
}

int CloseSerial()
{
	return CloseRS232( );
}

int watts_up_usb_meter_read_close () {

	threadState = 0;
	Sleep(2000);

	// stop meter output
    if (!WriteSetupString("#L,R,0;")) {
        fprintf(stderr,"closing write to WattsUp failed\n");
    }
	Sleep(2000);
	fprintf(stderr,"Returning WattsUp to internal logging.\n");
  	if (!WriteSetupString("#L,W,3,I,0,1;")) fprintf(stderr,"internal logging command to WattsUp failed\n");

	Sleep(2000);
	fprintf(stderr,"Sending alternate command format.\n");
  	if (!WriteSetupString("#L,W,3,I,-,1;")) fprintf(stderr,"internal logging command to WattsUp failed\n");

	Sleep (1000); 

	CloseSerial();

	return 1;
}

int wattsup_usb_meter_init(char *devicename, struct termios *options, int type) {

	int rc;

//Elson
	pthread_mutexattr_init( &AsyncMutexAttr );
	if ( rc = pthread_mutex_init( &AsyncMutex, NULL)) {
	   printf( "Mutex not created.\n" );
	   return -1;
	}
/*
	pthread_mutexattr_init( &DataMutexAttr );
	if ( rc = pthread_mutex_init( &DataMutex, NULL)) {
	   printf( "Mutex not created.\n" );
	   return -1;
	}

	if (pthread_mutex_init( &MeterReadReadyMutex, NULL)) {
	   perror("pthread_mutex_init MeterReadReadyMutex Error");
	   return -1;
	}
	if (pthread_cond_init( &MeterReadReadyCond, NULL)) {
	   perror("pthread_cond_init MeterReadReadyCond Error");
	   return -1;
	}
*/
#if 0
	MeterName = "WattsUpProUSB";
	// MeterCapabilities string contains the following (with 0/1 for ? values)
        // <averaging interval (ms)>,<watts?>,<volts?>,
        //   <amps?>,<pf?>,<energy?>,<frequency?>,<valid for submissions?>

    MeterCapabilities = "1000,1,1,1,1,0,0,0";

	// MeterCapabilitiesExtension contains
	//   <accuracy estimation enabled?>

	MeterCapabilitiesExtension = "1,0";
#endif

#if 0
	AccuracyEnabled = 1;

	AveragingInterval = 1000;
#endif

	//meterAsync = 1;    // tell ptd that this meter runs asynchronously

/*
	OpenRC = OpenSerial(meterArg, 115200, 8, 0, 1);

	if ((!OpenRC)) {
	    fprintf(stderr,"serial port open failed with error %d.\n",OpenRC);
		return false;
	}
*/

// open and setup RS232 fd
	 m_fd = open(devicename, O_RDWR | O_NOCTTY | O_NDELAY);

	 if (m_fd == -1) {
	 	 perror ("wattsup_usb_meter_init: Unable to open port");
		 return -1;
	 }

	 fcntl(m_fd, F_SETFL, 0);
	 bzero(options, sizeof(struct termios));
//	tcgetattr(m_fd, &options);

	 /* set raw input, 1 second timeout */
   options->c_cflag     = (CLOCAL | CREAD | BAUDRATE | CS8 );
   options->c_iflag	= IGNPAR;
   options->c_oflag     = IGNPAR;
   options->c_iflag     &= ~( IXON | IXOFF | IXANY);
   options->c_oflag     &= ~( IXON | IXOFF | IXANY);

  //  options.c_lflag     = ICANON;

	// different meters seem to need different settings here
	// CAUTION: setting VMIN to anything other than 0 can lead to a hang

	options->c_cc[VMIN] = 0;
	options->c_cc[VTIME] = 1;

  /* set the options */
  tcsetattr(m_fd, TCSANOW, options);
  tcflush(m_fd, TCIOFLUSH);

	// get version info
	if (!WriteAndReadSetup("#V,R,0;\n", 200, sBuffer)) return -1;
/*
	// soft reset
	fprintf(stderr, "Resetting device, please wait 15 seconds...\n");
	if (!WriteSetupString("#V,W,0;")) return false;
	Sleep(15000);

	// request that all fields be logged
	if (!WriteAndReadSetup("#C,W,18,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1;", 200, sBuffer)) return false;
	Sleep(500);
*/

	// clear logging memory just in case
	if (!WriteSetupString("#R,W,0;")) return -1;
	Sleep(500);
	// get log format string for fun
	if (!WriteSetupString("#H,R,0;")) return -1;
	Sleep(500);
	bReadRC = ReadSerial(sBuffer, 200, &iBytesRead);
	if (!bReadRC) {
	    fprintf(stderr,"read of WattsUpPro failed during meter init with error %d.\n",bReadRC);
		return -1;
	}

	sBuffer[iBytesRead]= 0;   // terminate string
	fprintf(stderr, "Meter responds: %s\n", sBuffer);
	if (iBytesRead < 40) {
		fprintf(stderr, "Incorrect byte count %lu from meter\n",iBytesRead);
		return -1;
	}

	// try a "chosen fields request"  for W,V,A,WH,PF,Hz
 	if (!WriteAndReadSetup("#C,W,18,1,1,1,1,0,0,0,0,0,0,0,0,0,1,0,0,1,0;", 2000, sBuffer)) return -1;

	Sleep(1000);
 // if (!WriteAndReadSetup("#C,R,0;")) return false;

	if (!WriteSetupString("#L,W,3,E,0,1;")) return -1;


	// create async thread for meter reading

	threadState = 1;


	if (pthread_create( &athread, NULL, AsyncReadThread, &SampleMS)) {
		fprintf(stderr,"error creating async read thread!");
		watts_up_usb_meter_read_close();
		return -1;
	}

	// now need to test if data is being read
	Sleep(2000);
	if (!asyncValid) {
		fprintf(stderr,"Error: no valid data received from WattsUpPro!\n");
		fprintf(stderr,"Trying alternate command format...\n");
		if (!WriteSetupString("#L,W,3,E,-,1;")) return -1;
		Sleep(2000);
		if (!asyncValid) {
			fprintf(stderr,"Error: still no valid data received from WattsUpPro!\n");
			return -1;   // never got a full response into buffer
		}
	}

  return m_fd;

}

/* set new attributes */
int watts_up_init(struct termios *newtio) {
   newtio->c_cflag = (BAUDRATE | CS8 | CLOCAL | CREAD | CSTOPB);
   newtio->c_iflag = (IGNPAR);
   newtio->c_oflag = (IGNPAR);
   newtio->c_lflag = 0;

   /* no software flow control */
   newtio->c_iflag &= ~(IXON | IXOFF | IXANY);
   newtio->c_oflag &= ~(IXON | IXOFF | IXANY);

   return 0;
}

int watts_up_query(int fd) {
   int rc = 0;
   char *c;

   if (!preamble) {
      struct timespec tspec;

      c = SEND_CLEAR;
      tspec.tv_sec = 1;
      tspec.tv_nsec = 0;
      if (watts_up_send(fd, SEND_CLEAR, strlen(SEND_CLEAR)) < 0) {
         fprintf(stderr, "watts_up_query: sending preamble failed\n");
         rc = -1;
      } else {
         preamble = 1;
      }
      nanosleep(&tspec, NULL);
   }
   c = SEND_QUERY;

   return watts_up_send(fd, SEND_QUERY, strlen(SEND_QUERY));
}

int watts_up_send(int fd, char *cmd, int len) {
   int r;

   if ((r = write(fd, cmd, len)) == -1) {
      perror("watts_up write");
      exit(1);
   }

   return r;
}

int ReadWattsUpUSB(mm_t *mm) {

	static char	localBuffer[128] = "                                            ";
	int localAsyncValid = 0;
    double watts, volts, amps, pf, acc;
	int rc;
	int iW, iV, iA, iWH, iPF, iHz;
	// int iC, iMKWH, iMC, iMaxW, iMaxV, iMaxA, iMinW;
    // int iMinV, iMinA, iPF, iDC, iPC, iVA;

	// grab and clear data under lock; keep lock as briefly as possible
	if(LockBuffers()) {
		localAsyncValid = asyncValid;
		if(asyncValid){
			strcpy(localBuffer, asyncBuffer);
			asyncValid = 0;
		}
		UnlockBuffers();
	}

	if (!localAsyncValid){  // no data in buffers, or lock failed, try once more
		Sleep(100);
		if(LockBuffers()) {
			localAsyncValid = asyncValid;
			if (asyncValid){
				strcpy(localBuffer, asyncBuffer);
				asyncValid = 0;
			}
			UnlockBuffers();
		}
	}
	
	if (!localAsyncValid){  // no data in buffers
		fprintf(stderr, "Warning: Missed 1 sample!!!\n");
		//LogWatts(-2.0,0); LogVolts(-2.0,0); LogAmps(-2.0,0); LogPF(-2.0,0); 
	}
	else {  // data is there, get it and clear valid flag
		// fprintf(stderr, "Reading async values\n");
/*
		if (!firstRead) {
			if (asyncValid > 1)
				fprintf(stderr,"Warning: %d meter readings were skipped!\n",localAsyncValid - 1);
		}
*/
		
 // Watts Up USB returns:
 //    #d,-,18,Watts,Volts,Amps,Watt Hours,Cost,Mo. kWh,Mo. Cost,
 //       Max watts,Max volts,Max amps,Min watts,Min volts,Min Amps,
 //      Power factor,Duty cycle,Power Cycle,Hz,VA;

/*        rc = sscanf(localBuffer, "#d,-,18,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                &iW, &iV, &iA, &iWH, &iC, &iMKWH, &iMC, &iMaxW, &iMaxV, &iMaxA, &iMinW, &iMinV, &iMinA,
                &iPF, &iDC, &iPC, &iHz, &iVA);
        if (rc != 18) {
			fprintf(stderr,"Can't parse (rc=%d) meter output %s\n",rc,localBuffer);
 			LogWatts(-1.0, 0); LogVolts(-1.0, 0); LogAmps(-1.0,0); LogPF(-1.0,0);
        } */

		rc = watts_up_parse (localBuffer, 128, mm);
		Sleep(1000);

#if 0
        rc = sscanf(localBuffer, "#d,-,18,%d,%d,%d,%d,_,_,_,_,_,_,_,_,_,%d,_,_,%d,_",
                &iW, &iV, &iA, &iWH, &iPF,  &iHz);
        if (rc != 6) {
			fprintf(stderr,"Can't parse (rc=%d) meter output %s\n",rc,localBuffer);
 			//LogWatts(-1.0, 0); LogVolts(-1.0, 0); LogAmps(-1.0,0); LogPF(-1.0,0);
        }
		else {
		    watts = iW / 10.0;
			volts = iV / 10.0;
			amps = iA / 1000.0;
			pf = iPF/ 100.0;

			//LogWatts(watts, 1); LogVolts(volts, 1); LogAmps(amps,1); LogPF(pf,1);
			//LogFrequency(iHz / 10.0, 1); LogWH(iWH, 1);

			if (calculateAccuracy) {
				// WattUp Pro USB all have specified accuracy of 1.5% + 3 counts of displayed value
				//  displayed values have 3 full digits plus an upper 1, and don't display <0.1W
				//  when W > 199.9, the tenths digit is lost
				if (watts > 199.95) {
					// displayed value is integer watts, at least up to 1999W
					acc = 0.015 + (3.0 / watts);
				}
				else {
					// displayed value is xxx.x watts
					acc = 0.015 + (0.3 / watts);
				}
				LogAccuracy(acc, 1);
				if (verboseMode) {
					fprintf(stderr,"Calculated error estimate for %4f watts is %4f%%\n",
						watts,100.0*acc);
				}
			}
		}
#endif
	}
	return rc;
}

#define MIN(x, y)	((x < y) ? x : y)
#define MAX_TIMEOUT		30

int watts_up_usb_meter_read(int fd, int type, mm_t *mm) {
   int n;
   int rc = 0;
   //fd_set input;
   struct timeval timeout, *tout = NULL;
   //char buf[DBUFSZ];
   //meter_t *meter = NULL;

   if (mm == NULL) {
      errno = EINVAL;
      return -1;
   }

   /* find the meter */
/*
   if ((meter = meter_lookup(type)) == NULL) {
      errno = EINVAL;
      return -1;
   }
*/
   /* initialize the input set */
/*
   FD_ZERO(&input);
   FD_SET(fd, &input);
*/
   mm->value = 0.0;

   /* initialize the timeout structure (default is no timeout) */
/*
   if (mm->to_sec > 0) {
      timeout.tv_sec = MIN(mm[0].to_sec, MAX_TIMEOUT);
      timeout.tv_usec = 0;
      tout = &timeout;
   }
*/
	
	 rc = ReadWattsUpUSB(mm);

#if 0
   /* perform the initial query */
   if (meter->query != NULL)
      meter->query(fd);

   /* read the data or get keyboard input */
   if ((n = select(fd + 1, &input, NULL, NULL, tout)) == -1) {
      if (errno != EINTR)
         perror("select failed");
      return -1;
   } else if (n == 0) {
      fprintf(stderr, "timeout reading meter\n"); /* retry */
      rc = 1;
   } else { /* have data, parse as needed */
      if (FD_ISSET(fd, &input)) {
         int status;
         int rb;

         rb = meter->read(fd, buf, sizeof(buf), &status);
         if (rb == 0) {
            errno = ENODATA;
            return -1;
         }
         rc = meter->parse(buf, rb, mm);
      }
   }
#endif

   return rc;
}

int watts_up_read(int fd, char *buf, int len, int *status) {
   char buf_tmp[256];
   int c, r;
   char *p;
   int i;

   *status = 0;

   /* read until EOL */
   c = 0;
   while (1) {
      if ((r = read(fd, buf_tmp + c, sizeof(buf_tmp) - c)) == -1) {
         errno = ENOBUFS;
         return -1;
      }
      c += r;
      if (strchr(buf_tmp, ';'))
         break; /* EOL */
   }
   i = 0;
   *strchr(buf_tmp, ';') = '\0';
   p = buf_tmp;
   while (*p != '#' && *p != '\0' && ++i < c)
      p++;
   if (i >= c || i >= len) {
      errno = ENOBUFS;
      return -1;
   }
   c = strlen(p);
   memcpy(buf, p, c);
   buf[c] = '\0';

   return c;
}

/*
 * Format is as follows:
 *
 *    #d,-,16,Watts,Volts,Amps,Watt Hours,Cost,Mo. kWh,Mo. Cost,
 *       Max watts,Max volts,Max amps,Min watts,Min volts,Min Amps,
 *       Power factor,Duty cycle,Power Cycle
 */
int watts_up_parse(char *buf, int len, mm_t *mm) {
   double value;
   char *token;
   int i = 0;
   char *p;
   int pos = 0;

   if (buf == NULL || mm == NULL) {
      errno = EINVAL;
      return -1;
   }
   p = strstr(buf, "#");
   if (p == NULL || strncmp(p, "#d,", 3) != 0)
      return -1;
//printf("have buffer %s\n", p);
   token = strtok(p, ",");
   while (token != NULL) {
      value = -2.0; /* set to a flag */
      switch (i++) { /* look at offset and set applicable field */
         case 3:
            value = atoi(token);
            value /= 10;
            mm[pos].units = MUNITS_WATTS;
            break;
         case 4:
            value = atoi(token);
            value /= 10;
            mm[pos].units = MUNITS_VOLTS;
            break;
         case 5:
            value = atoi(token);
            value /= 1000;
            mm[pos].units = MUNITS_AMPS;
            break;
         case 16:
            value = atoi(token);
            value /= 100;
            mm[pos].units = MUNITS_POWERFACTOR;
            break;
      }
      if (value > -1.0)
         mm[pos++].value = value;
      token = strtok(NULL, ",");
   }

   return 0;
}

