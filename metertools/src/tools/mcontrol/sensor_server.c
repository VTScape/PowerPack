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
 * $Id: mlogger.c,v 1.1.1.1 2007/02/07 15:07:23 fengx Exp $
 */

/*
 * Use select to read from 1 file descriptor (and wait on kbd input for stop)
 * based on Serial Programming Guide for POSIX Operating Systems
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <syslog.h>
#include <errno.h>
#include <libgen.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif
#include "passivesock.h"
#include "meter.h"

#include "watts_up.h"

/* thread stack size for servicing new connections */
#define THREAD_STACK_SIZE		65535L

/* for power measurements, simulate this number for volts */
#ifndef SIM_VOLTS
#   define SIM_VOLTS			110.0
#endif

/* maximum number of connections handled by server */
#ifndef MAXCONNS
#   define MAXCONNS			20
#endif

#define	DEFAULT_METER_PORT	6999


#define ENABLE_RAPL 1
#define ENABLE_MIC_SENSOR 0

#if ENABLE_RAPL

#include <inttypes.h>
#include <math.h>

#define MSR_RAPL_POWER_UNIT             0x606

/*
 *  * Platform specific RAPL Domains.
 *   * Note that PP1 RAPL Domain is supported on 062A only
 *    * And DRAM RAPL Domain is supported on 062D only
 *     */
/* Package RAPL Domain */
#define MSR_PKG_RAPL_POWER_LIMIT        0x610
#define MSR_PKG_ENERGY_STATUS           0x611
#define MSR_PKG_PERF_STATUS             0x613
#define MSR_PKG_POWER_INFO              0x614

/* PP0 RAPL Domain */
#define MSR_PP0_POWER_LIMIT             0x638
#define MSR_PP0_ENERGY_STATUS           0x639
#define MSR_PP0_POLICY                  0x63A
#define MSR_PP0_PERF_STATUS             0x63B

/* PP1 RAPL Domain, may reflect to uncore devices */
#define MSR_PP1_POWER_LIMIT             0x640
#define MSR_PP1_ENERGY_STATUS           0x641
#define MSR_PP1_POLICY                  0x642

/* DRAM RAPL Domain */
#define MSR_DRAM_POWER_LIMIT            0x618
#define MSR_DRAM_ENERGY_STATUS          0x619
#define MSR_DRAM_PERF_STATUS            0x61B
#define MSR_DRAM_POWER_INFO             0x61C

/* RAPL UNIT BITMASK */
#define POWER_UNIT_OFFSET       0
#define POWER_UNIT_MASK         0x0F

#define ENERGY_UNIT_OFFSET      0x08
#define ENERGY_UNIT_MASK        0x1F00

#define TIME_UNIT_OFFSET        0x10
#define TIME_UNIT_MASK          0xF000

#endif

extern int errno;
extern char *optarg;
extern int optind, opterr, optopt;

extern int mdebug;

/* prototypes */
void *request_handler(void *arg);
void integrate(struct timeval begin, struct timeval end, mm_t *mm,
   mm_t *volts, float *interval, float *energy);
void initialize(pthread_t *thread, unsigned short port);
void *client_req(void *arg);
void show_usage(char *argv[]);
int monitor(int type, char *serialport, FILE *output, int count,
   float sim_volts, unsigned int to_sec);

typedef struct {
   unsigned int id;
   pthread_t thread;
   short int sock;
   struct sockaddr_in client;
} connection_t;

static connection_t *connection;
static float power;
static unsigned int cumulative_energy;
static int running = 1;
static int console = 0;
static mm_t last_mm = { 0.0, 0, 0 };

static struct timeval tv_last_reading;

#if 0
extern pthread_mutexattr_t AsyncMutexAttr;
extern pthread_mutex_t AsyncMutex;
extern pthread_mutexattr_t DataMutexAttr;
extern pthread_mutex_t DataMutex;
extern pthread_mutex_t MeterReadReadyMutex;
extern pthread_cond_t  MeterReadReadyCond;
#endif

//End

#if ENABLE_MIC_SENSOR

#define NUM_MIC_PWRS 1
unsigned int mic_milli_powers[NUM_MIC_PWRS];

#endif

#if ENABLE_RAPL

#define NUM_RAPL_PWRS 3

unsigned int rapl_milli_powers[NUM_RAPL_PWRS];

//float rapl_package_pwr, rapl_pp0_pwr, rapl_dram_pwr;

int open_msr(int core) {

  char msr_filename[BUFSIZ];
  int fd;

  sprintf(msr_filename, "/dev/cpu/%d/msr", core);
  fd = open(msr_filename, O_RDONLY);
  if ( fd < 0 ) {
    if ( errno == ENXIO ) {
      fprintf(stderr, "rdmsr: No CPU %d\n", core);
      exit(2);
    } else if ( errno == EIO ) {
      fprintf(stderr, "rdmsr: CPU %d doesn't support MSRs\n", core);
      exit(3);
    } else {
      perror("rdmsr:open");
      fprintf(stderr,"Trying to open %s\n",msr_filename);
      exit(127);
    }
  }

  return fd;
}

long long read_msr(int fd, int which) {

  uint64_t data;

  if ( pread(fd, &data, sizeof data, which) != sizeof data ) {
    perror("rdmsr:pread");
    exit(127);
  }

  return (long long)data;
}
#endif

void send_reading(int s) {
   static unsigned int msgsz = sizeof(netmeter_req_t);
   netmeter_req_t msg;
   int r;

   msg.cmd = NETMETER_CMD_READ;
   msg.units = last_mm.units;
   //msg.value = (void *)(unsigned int)(last_mm.value * 100000);
   //msg.value = (unsigned int)(last_mm.value * 100000);
   if ((r = write(s, &msg, msgsz)) != msgsz)
      fprintf(stderr, "send_reading: wrote only %i of %i bytes\n", r, msgsz);
}

void send_energy(int s) {
   static unsigned int msgsz = sizeof(netmeter_req_t);
   netmeter_req_t msg;
   int r;

   msg.cmd = NETMETER_CMD_ENERGY;
   //msg.value = (void *)cumulative_energy;
   //msg.value = cumulative_energy;
   msg.units = MUNITS_JOULES;
   if ((r = write(s, &msg, msgsz)) != msgsz)
      fprintf(stderr, "send_energy: wrote only %i of %i bytes\n", r, msgsz);
}

/*
 * Try to avoid any issues with network packets and data types and send
 * the return data last measured for power as the truncated version of
 * the real float value (milliwatts).
 */
void send_power(int s) {
   static unsigned int msgsz = sizeof(netmeter_req_t);
   netmeter_req_t msg;
   int r;
   mm_t mm;
   float value;
   struct timeval tv;

   char  pkg[1024];
   unsigned int pkg_len, pwr_val;
   //unsigned int value;

   /* construct the return value */
#if ENABLE_RAPL
   char  pwr_arry[128];

   /* setup the return data */
   msg.cmd = NETMETER_CMD_POWER;
#if ENABLE_MIC_SENSOR
   msg.num_chs = (NUM_RAPL_PWRS + NUM_MIC_PWRS);
#else
   msg.num_chs = NUM_RAPL_PWRS;
#endif
   msg.units = MUNITS_MILLI | MUNITS_WATTS;

   memcpy(pkg, &msg, sizeof(netmeter_req_t));
#if ENABLE_MIC_SENSOR
   memcpy(pkg+sizeof(netmeter_req_t), rapl_milli_powers, sizeof(unsigned int)*NUM_RAPL_PWRS);
   memcpy(pkg+sizeof(netmeter_req_t)+sizeof(unsigned int)*NUM_RAPL_PWRS, mic_milli_powers, sizeof(unsigned int)*NUM_MIC_PWRS);
   pkg_len = sizeof(netmeter_req_t) + (sizeof(unsigned int)*NUM_RAPL_PWRS) + (sizeof(unsigned int)*NUM_MIC_PWRS);
#else
   memcpy(pkg+sizeof(netmeter_req_t), rapl_milli_powers, sizeof(unsigned int)*NUM_RAPL_PWRS);
   pkg_len = sizeof(netmeter_req_t) + sizeof(unsigned int)*NUM_RAPL_PWRS;
#endif
   if ((r = write(s, &pkg, pkg_len)) != pkg_len)
      fprintf(stderr, "send_power: wrote only %i of %i bytes\n", r, pkg_len);

#else
   mm.value = power;
   mm.units = MUNITS_WATTS;
   value = meter_to_milli(&mm);
   /* setup the return data */
   msg.cmd = NETMETER_CMD_POWER;
   //msg.value = (void *)(unsigned int)value;
   //msg.value = (unsigned int)value;
   //msg.last_read = ;
   msg.num_chs = 1;
   msg.units = MUNITS_MILLI | MUNITS_WATTS;

   pwr_val = (unsigned int) value;

   memcpy(&tv, &tv_last_reading, sizeof(struct timeval));
   
   msg.last_read = tv.tv_sec + tv.tv_usec * 1.0e-6;

   memcpy(pkg, &msg, sizeof(netmeter_req_t));
   memcpy(pkg+sizeof(netmeter_req_t), &pwr_val, sizeof(unsigned int));
//memcpy(&pwr_val, pkg+sizeof(netmeter_req_t), sizeof(unsigned int));
//printf("%d %d\n", pwr_val, *((unsigned int *)(pkg+sizeof(netmeter_req_t))));
   pkg_len = sizeof(netmeter_req_t)+sizeof(unsigned int);
   //if ((r = write(s, &msg, msgsz)) != msgsz)
   //   fprintf(stderr, "send_power: wrote only %i of %i bytes\n", r, msgsz);

   if ((r = write(s, &pkg, pkg_len)) != pkg_len)
      fprintf(stderr, "send_power: wrote only %i of %i bytes\n", r, pkg_len);
#endif
}

void *request_handler(void *arg) {
   int s;
   struct sockaddr_in raddr;
   int rlen;
   int i;
   pthread_attr_t thr_attr;
   unsigned short port;
   struct pollfd pfd;

   port = (unsigned short)(int)arg;
   printf("listening on port %i for clients\n", port);
   pfd.fd = s = tcp_passive_port(port, 5);
   if (s == -1)
      return (void *)-1;
   pfd.events = POLLIN;
   if (pthread_attr_init(&thr_attr)) {
      perror("pthread_attr_init");
      exit(1);
   }
   if (pthread_attr_setstacksize(&thr_attr, THREAD_STACK_SIZE)) {
      perror("pthread_attr_setstacksize");
      exit(1);
   }
   while (running) {
      memset(&raddr, 0, sizeof(raddr));
      rlen = sizeof(raddr);
      for (i = 0; ; i++) {
         if (i > MAXCONNS) {
            struct timespec req, rem;
            req.tv_sec = 1;
            req.tv_nsec = 0;
            nanosleep(&req, &rem);
            i = 0;
            continue;
         }
         if (connection[i].sock == 0)
            break;
      }
      poll(&pfd, 1, 1000);
      if (pfd.revents == POLLIN) {
         connection[i].sock = accept(s, (struct sockaddr *)&raddr, &rlen);
         if (connection[i].sock < 0) {
            connection[i].sock = 0;
            fprintf(stderr, "connection %d: error accept: %s\n", i,
               strerror(errno));
            continue;
         } else {
            if (pthread_create(&connection[i].thread, &thr_attr,
                  client_req, (void *)i) != 0) {
               fprintf(stderr, "connection %d: error pthread_create: %s\n",
                  i, strerror(errno));
               close(connection[i].sock);
               connection[i].sock = 0;
            }
         }
      }
   }
   close(s);

   return 0;
}

#define	MSG_BUF_SIZE	64
#define MAX_MSG_SIZE    62  
#define NEW_LOG_FILE    20  
#define NEW_SESSION     21  
#define END_LOG_FILE    22  
#define END_SESSION     23  
#define CHG_WORKING_DIR 24  

typedef struct control_msg
{
    char    cmd;
	char    len; 
	char    msg[MAX_MSG_SIZE]; 
}control_msg_t;

static int logging = 0;
static FILE *fplog = 0;

void *client_req(void *arg) {
   char msgbuf[MSG_BUF_SIZE];
   netmeter_req_t msg;                /* message received from peer */
   control_msg_t *cmsg;                /* message received from peer */
   int id = (int)arg;                 /* connection id number in array */
   ssize_t size = 0;                  /* amount of data read from socket */
   struct sockaddr_in fsin;           /* socket info of connected peer */
   int alen = sizeof(fsin);           /*   useful for debugging */
   int len;
   int connected = 1;                 /* loop while connected */
   struct timeval start, stop;        /* track timings for connections */
   unsigned int hr = 0, min = 0, sec; /* connection service time info */
   unsigned long commands = 0;        /* number of commands serviced */

   gettimeofday(&start, NULL);
   connection[id].id = pthread_self();
   //pthread_detach(connection[id].id);
   if (getpeername(connection[id].sock, (struct sockaddr *)&fsin,
         &alen) == -1)
      return (void *)-1; /* client abort */
   printf("connection %d: start client %s\n", id, inet_ntoa(fsin.sin_addr));
   while (running && connected) {
      memset(msgbuf, 0, sizeof(msgbuf));
      memset(&msg, 0, sizeof(msg));
      //if ((size = read(connection[id].sock, &msg, sizeof(msg))) == -1 ||
      //     size != sizeof(netmeter_req_t)) {
      if ((size = read(connection[id].sock, msgbuf, sizeof(msgbuf))) == -1 ){
         fprintf(stderr, "connection %d: error request size %d errno %d\n",
            id, size, errno);
         connected = 0;
      } else if (!running) {
         connected = 0;
      } else {
//printf("size(%d) %d\n", size, sizeof( netmeter_req_t ));
	  	 if ( size == sizeof( netmeter_req_t )){
		 strncpy((char *)&msg, (char *)msgbuf, sizeof(msg));
         switch(msg.cmd) {
            case NETMETER_CMD_ENERGY:
               send_energy(connection[id].sock); commands++;
               break;
            case NETMETER_CMD_POWER:
               send_power(connection[id].sock); commands++;
               break;
            case NETMETER_CMD_READ:
               send_reading(connection[id].sock); commands++;
               break;
            case NETMETER_CMD_RESET:
               cumulative_energy = 0;
               commands++;
               break;
            case NETMETER_CMD_QUIT: /* gracefully exit */
               connected = 0;
               break;
            default:
               fprintf(stderr, "connection %d: error unknown command %i\n",
                  id, msg.cmd);
               connected = 0;
               break;
		 }
		 }else if ( size > 2 ){
		 	cmsg = (control_msg_t *)msgbuf;
			len = cmsg->len > MAX_MSG_SIZE-1 ? cmsg->len :  MAX_MSG_SIZE-1;
			cmsg->msg[len] = '\0';
			printf("msg from %s (size=%d): {cmd = %d len = %d txt = %s}\n", 
				inet_ntoa(fsin.sin_addr), size, cmsg->cmd, cmsg->len, cmsg->msg);

			switch( cmsg->cmd ){
				case NEW_LOG_FILE:
					if ( fplog != 0 ){
						logging = 0;
						fclose( fplog );
						fplog = 0;
					}
				    if( (fplog = fopen(cmsg->msg, "w+")) == NULL) {
							fprintf(stderr, "Can not create file :%s (%s)\n", cmsg->msg, strerror(errno)); 
							logging = 0;
							fplog = 0;
					}
					else{
						fprintf( fplog, "[BEGIN_LOG: %.3f %s %s ]\n", start.tv_sec + start.tv_usec * 1.0e-6, cmsg->msg, inet_ntoa(fsin.sin_addr));
						fflush( fplog );
						logging = 1;
					}
					break;
				case END_LOG_FILE:
					logging = 0;
					if ( fplog!=0 ){
						fclose(fplog);
					}
					fplog = 0;
					break;
				case NEW_SESSION:
					if ( fplog!=0){
						fprintf(fplog, "[BEGIN_SESSION: %s %.3f]\n", cmsg->msg, start.tv_sec + start.tv_usec * 1.0e-6);
						fflush(fplog);
					}
					else {
						fprintf(stderr, "no valid log file");
						logging = 0;
						fplog = 0;
					}
					break;
				case END_SESSION:
					if ( fplog != 0 ){
						fprintf(fplog, "[END_SESSION: %s %.3f]\n", cmsg->msg, start.tv_sec + start.tv_usec * 1.0e-6);
						fflush(fplog);
					}
					else {
						fprintf(stderr, "no valid log file");
						logging = 0;
						fplog = 0;
					}
					break;
				case CHG_WORKING_DIR:
					if ( chdir( cmsg->msg ) == 0 ){
						fprintf(stdout, "change working directory to %s\n", cmsg->msg);
					}
					else {
						fprintf(stderr, "can not access directory: %s\n", cmsg->msg);
					}
					break;
				default:
					fprintf(stderr, "unknow control command %d\n", cmsg->cmd);
					break;
			}
			connected = 0;
		 }else{
		 	fprintf(stderr, "unexpected message size %d\n", size);
			connected = 0;
		 }

      }
   }
   close(connection[id].sock);
   connection[id].sock = 0;
   gettimeofday(&stop, NULL);
   sec = stop.tv_sec - start.tv_sec;
   if (sec >= 3600) { /* seconds per hr */
      hr = sec / 3600;
      sec -= hr * 3600;
   }
   if (sec >= 60) { /* seconds per min */
      min = sec / 60;
      sec -= min * 60;
   }
   printf("connection %d: stop service time %uh:%um:%.3fs for %lu command%s\n",
      id, hr, min, sec + (stop.tv_usec / 1000000.0), commands,
      commands == 1 ? "": "s");
   pthread_exit(0);
}

/* terminatiOn signal received */
void signal_exit(int sig) {
   running = 0;
}

/* initialize the signal handlers and additional threads */
void initialize(pthread_t *thread, unsigned short port) {
   signal(SIGINT, signal_exit);
   if (port == 0)
      return;
   connection = (connection_t *)calloc(MAXCONNS, sizeof(connection_t));
   if (connection == NULL) {
      fprintf(stderr, "out of memory creating %d connections\n", MAXCONNS);
      exit(1);
   }
   if (pthread_create(thread, NULL, request_handler, (void *)(int)port) != 0) {
      fprintf(stderr, "unable to start network listener: %s\n",
         strerror(errno));
      exit(1);
   }
}

/* perform simple rectangular integration */
void integrate(struct timeval begin, struct timeval end, mm_t *mm,
      mm_t *volts, float *interval, float *energy) {
   if (mm == NULL || volts == NULL)
      return;
   if (!(mm->units & MUNITS_WATTS))
      power = mm->value * volts->value;
   else
      power = mm->value;
   *interval = (end.tv_sec - begin.tv_sec) +
      ((end.tv_usec - begin.tv_usec) / 1000000.0);
   *energy = power * (*interval);
   cumulative_energy += *energy;
}

int main(int argc, char *argv[]) {
   int count = 0;
   FILE *output = NULL;
   int c;
   float volts = SIM_VOLTS;
   char *serialport = SERIAL_PORT;
   unsigned short port = DEFAULT_METER_PORT;
   pthread_t thread;
   unsigned int to_sec = METER_TIMEOUT;
   int type = METER_TYPE;
   int rc;

   opterr = 0;
   while ((c = getopt(argc, argv, "m:t:s:p:v:l:c:doh")) != -1) {
      switch (c) {
         case 'm':
            if ((type = meter_type(optarg)) == 0) {
               fprintf(stderr, "unknown meter %s for -m\n", optarg);
               exit(1);
            }
            break;
         case 'd':
            mdebug = 1;
            break;
         case 't':
            to_sec = atoi(optarg);
            break;
         case 'p':
            port = atoi(optarg);
            break;
         case 's':
            serialport = strdup(optarg);
            break;
         case 'v':
            volts = atof(optarg);
            break;
         case 'l':
            if (optarg[0] != '-') {
               output = fopen(optarg, "a");
               if (output == NULL) {
                  perror(argv[1]);
                  exit(1);
               }
            }
            break;
         case 'o':
            console = 1;
            break;
         case 'c':
            count = atoi(optarg);
            break;
         case 'h':
            show_usage(argv);
            exit(0);
         default:
            fprintf(stderr, "unknown option -%c, use -h for help\n", optopt);
            exit(1);
      }
   }

   initialize(&thread, port);
   rc = monitor(type, serialport, output, count, volts, to_sec);
   if (port) {
      printf("disconnecting any clients...\n");
      pthread_join(thread, NULL);
   }
   printf("finished.\n");
   if (output != NULL)
      fclose(output);

   return rc;
}

#if ENABLE_MIC_SENSOR
int mic_sensor_read() {
  FILE *fp;
  int status;
  float val;
  char path[1024];

  fp = popen("/opt/intel/mic/bin/micsmc -f | grep \"Total\" | awk '{print $4}'", "r");
//  while (fgets(path, 1024, fp) != NULL)
//        printf("%s", path);
  fgets(path, 1024, fp);
  //printf("%f\n", atof(path));

  status = pclose(fp);

  mic_milli_powers[0] = (unsigned int)((atof(path))*1000);

  return 0;
}
#endif

#if ENABLE_RAPL

double rapl_power_units,rapl_energy_units,rapl_time_units;

int rapl_init(){
  int fd;
  int core=0;
  long long result;
  //double power_units,energy_units,time_units;

  fd=open_msr(core);

  /* Calculate the units used */
  result=read_msr(fd,MSR_RAPL_POWER_UNIT);

  rapl_power_units=pow(0.5,(double)(result&0xf));
  rapl_energy_units=pow(0.5,(double)((result>>8)&0x1f));
  rapl_time_units=pow(0.5,(double)((result>>16)&0xf));


  return fd;
}

int rapl_read(int fd, int type, mm_t *mm){
  long long result;
  static double last_package=0, last_pp0=0, last_pp1, last_dram=0;
  double package=0, pp0=0, pp1, dram=0;
  struct timeval tv, last_tv;
  double ts;
  static double last_ts;
  //static timeval last_tv;

  gettimeofday(&tv, NULL);
  ts = tv.tv_sec + tv.tv_usec * 1.0e-6;

  result=read_msr(fd,MSR_PKG_ENERGY_STATUS);
  package = (double)result*rapl_energy_units;
  mm[0].value = package - last_package;
  last_package = package;
  //package_before=(double)result*energy_units;
  //printf("Package energy before: %.6fJ\n",package_before);

  result=read_msr(fd,MSR_PP0_ENERGY_STATUS);
  pp0 = (double)result*rapl_energy_units;
  mm[1].value = pp0 - last_pp0;
  last_pp0 = pp0;
  //printf("PowerPlane0 (core) for core %d energy before: %.6fJ\n",core,pp0_before);

  /* not available on SandyBridge-EP */
#if 0
  result=read_msr(fd,MSR_PP1_ENERGY_STATUS);
  pp1 = (double)result*rapl_energy_units;
  mm[3].value = pp1 - last_pp1;
  last_pp1 = pp1;
/*
  pp1_before=(double)result*energy_units;
  printf("PP1 (on-core GPU if avail) before: %.6fJ\n",pp1_before);
*/
#endif

  result=read_msr(fd,MSR_DRAM_ENERGY_STATUS);
  dram = (double)result*rapl_energy_units;
  mm[2].value = dram - last_dram;
  last_dram = dram;
  //printf("DRAM energy before: %.6fJ\n",dram_before);

  //if ()
  if ((ts-last_ts) > 0) {
  //printf("%.6fW %.6fW %.6fW %.6fW\n", mm[0].value/(ts-last_ts), mm[1].value/(ts-last_ts), mm[2].value/(ts-last_ts));
  //rapl_package_pwr = mm[0].value/(ts-last_ts);
  //rapl_pp0_pwr = mm[1].value/(ts-last_ts);
  //rapl_dram_pwr = mm[2].value/(ts-last_ts);

  rapl_milli_powers[0] = (unsigned int)( (mm[0].value/(ts-last_ts))*1000);
  rapl_milli_powers[1] = (unsigned int)( (mm[1].value/(ts-last_ts))*1000);
  rapl_milli_powers[2] = (unsigned int)( (mm[2].value/(ts-last_ts))*1000);

  //printf("%u %u %u\n", rapl_milli_powers[0], rapl_milli_powers[1], rapl_milli_powers[2]);
  }
  last_ts = ts;
  //memcpy(&last_tv, &tv, sizeof(struct timeval));

#if ENABLE_MIC_SENSOR
  mic_sensor_read();
#endif

  usleep(500000);
  //usleep(200000);

  return 0;
}

int rapl_close(int fd){

  close(fd);

  return 0;
}

#endif

int monitor(int type, char *serialport, FILE *output, int count,
      float sim_volts, unsigned int to_sec) {
   struct termios oldtio;
   int fd;
   int rc = 0;
   mm_t *mm, volts;
   struct timeval begin_tv, tv, last_tv;
   double ts;
   int loops = 0;
   int num;
   int i;

   if (count)
      loops = count - 1;
   if ((num = meter_alloc_mms(type, &mm)) == -1) {
      fprintf(stderr, "meter_alloc_mms: %s\n", strerror(errno));
      return -1;
   }

//   if ((fd = meter_init(serialport, &oldtio, type)) == -1) {
#if ENABLE_RAPL
   if ((fd = rapl_init()) == -1) {
#else
   if ((fd = wattsup_usb_meter_init(serialport, &oldtio, type)) == -1) {
#endif
//End
      fprintf(stderr, "meter_init: %s\n", strerror(errno));
      running = 0;
      return -1;
   }
   volts.value = sim_volts;
   for (i = 0; i < num; i++)
      mm[i].to_sec = to_sec;
   volts.to_sec = to_sec;
   cumulative_energy = 0;
   gettimeofday(&begin_tv, NULL);
   memcpy(&last_tv, &begin_tv, sizeof(struct timeval));

   while (running && loops >= 0) {
			//Elson
#if ENABLE_RAPL
			rc = rapl_read(fd, type, mm);
#else
			rc = watts_up_usb_meter_read(fd, type, mm);
#endif
      //rc = meter_read(fd, type, mm);
      if (rc == -1) {
         fprintf(stderr, "meter_read: %s\n", strerror(errno));
         loops = -1;
         break;
      }
      last_mm.units = mm[0].units;
      last_mm.value = mm[0].value;
      if (rc != 1) { /* no timeout and need to display */
	  	 gettimeofday(&tv, NULL);
		 ts = tv.tv_sec + tv.tv_usec * 1.0e-6;
		 if (console ){
		 	printf("%.2f ", ts);
         	for (i = 0; i < num; i++)
            	printf("%f %s%s ", mm[i].value, meter_units_prefix(&mm[0]),
               		meter_units_measure(&mm[i]));
         	printf("\n");
		 }
		 if (logging && fplog!=0){
		 	fprintf(fplog, "%.2f ", ts);
         	for (i = 0; i < num; i++)
            	fprintf(fplog, "%f %s%s ", mm[i].value, meter_units_prefix(&mm[0]),
               		meter_units_measure(&mm[i]));
         	fprintf(fplog, "\n");
			fflush(fplog);
		 }
      }
      if (mm[0].value > 0.0) {
         float interval, energy;

         if (gettimeofday(&tv, NULL) == -1) {
            fprintf(stderr, "gettimeofday error: %s\n", strerror(errno));
            loops = -1;
         } else {
            integrate(last_tv, tv, &mm[0], &volts, &interval, &energy);
            if (output != NULL)
               fprintf(output, "%u %f %f %f %f %f %f\n",
                  (unsigned int)tv.tv_sec,
                  (tv.tv_sec - begin_tv.tv_sec) + (tv.tv_usec / 1000000.0),
                  mm[0].value, volts.value,
                  power, interval, energy);
            memcpy(&last_tv, &tv, sizeof(struct timeval));
            memcpy(&tv_last_reading, &tv, sizeof(struct timeval));
         }
         if (count)
            loops--;
         if (output != NULL && (loops % 10 == 0))
            fflush(output);
      }
   }
   running = 0;
#if ENABLE_RAPL
	rapl_close(fd);
#else
   //Elson
   //meter_close(fd, &oldtio);
	 watts_up_usb_meter_read_close();
#endif

   return rc;
}

void show_usage(char *argv[]) {
   char *name = basename(argv[0]);

   printf("Usage: %s [-l log] [-c count] [-v volts] [-s serialport] [-p port] [-t timeout] [-m meter] [-o] [-d]\n", name);
   printf("       %s -h\n", name);
   printf("\t-o             Output the value read to standard out\n");
   printf("\t-d             Turn on debugging\n");
   printf("\t-t timeout     Timeout waiting for data from meter (sec) [%d]\n",
      METER_TIMEOUT);
   printf("\t-l log         Write data to the indicated log file\n");
   printf("\t-c count       Read count values and then stop, 0 for no limit [0]\n");
   printf("\t-v volts       Simulate the indicated volts for power [%.1f]\n",
      SIM_VOLTS);
   printf("\t-m meter       Name of the meter (see README) [%s]\n",
      meter_name(METER_TYPE));
   printf("\t-s serialport  Serial port for meter [%s]\n", SERIAL_PORT);
   printf("\t-p port        Network port for clients, 0 to disable [%d]\n",
      METER_TCP_PORT);
   printf("\t-h             This help screen\n");
}

