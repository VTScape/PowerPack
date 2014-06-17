#include <windows.h>
#include <winsock.h>
//#include <winsock2.h>
//#include <ws2tcpip.h>
#include <stdio.h>
#include <signal.h>

#include "nidaqmx_server.h"
#include "netmeter.h"

/* maximum number of connections handled by server */
#ifndef MAXCONNS
#   define MAXCONNS			20
#endif

#define MAXPENDING 5    /* Maximum outstanding connection requests */
#define	DEFAULT_METER_PORT	6913

typedef struct {
   unsigned int id;
   HANDLE thread;
   short int sock;
   struct sockaddr_in client;
} connection_t;

static connection_t *connection;

/* terminatiOn signal received */static unsigned int cumulative_energy;
int running = 1;
static int console = 0;
static mm_t last_mm = { 0.0, 0, 0 };

//unsigned int nidaq_volts[NUM_NIDQ_CHANNELS];
unsigned int nidaq_milli_powers[NUM_NIDQ_CHANNELS];

HANDLE hMutex;

void send_reading(int s) {
}

void send_energy(int s) {
}

void send_power(int s) {
   static unsigned int msgsz = sizeof(netmeter_req_t);
   netmeter_req_t msg;
   int r;
   //mm_t mm;
   float value;

   char  pkg[1024], pwr_arry[128];
   unsigned int pkg_len, pwr_val;
   //unsigned int value;
   int i;
#if 0
   /* construct the return value */
   mm.value = power;
   mm.units = MUNITS_WATTS;
   value = meter_to_milli(&mm);
#endif
   /* setup the return data */
   msg.cmd = NETMETER_CMD_POWER;
   //msg.value = (void *)(unsigned int)value;
   //msg.value = (unsigned int)value;
   msg.num_chs = NUM_ACTIVE_NIDQ_CHANNELS;
   msg.units = MUNITS_MILLI | MUNITS_WATTS;

//   pwr_val = (unsigned int) value;
   memcpy(pkg, &msg, sizeof(netmeter_req_t));
//   memcpy(pkg+sizeof(netmeter_req_t), &pwr_val, sizeof(unsigned int));
//memcpy(&pwr_val, pkg+sizeof(netmeter_req_t), sizeof(unsigned int));
//printf("%d %d\n", pwr_val, *((unsigned int *)(pkg+sizeof(netmeter_req_t))));
//pthread_mutex_lock(&mutex);
   //memcpy(pwr_arry, nidaq_milli_powers, sizeof(unsigned int)*NUM_ACTIVE_NIDQ_CHANNELS);
   //memcpy(pwr_arry+sizeof(unsigned int), &(timely_nidaq.nidaq_powers), sizeof(unsigned int)*16);
//pthread_mutex_unlock(&mutex);
   WaitForSingleObject( hMutex, INFINITE );
   memcpy(pkg+sizeof(netmeter_req_t), nidaq_milli_powers, sizeof(unsigned int)*NUM_ACTIVE_NIDQ_CHANNELS);
   ReleaseMutex( hMutex );
   //if ((r = write(s, &msg, msgsz)) != msgsz)
   //   fprintf(stderr, "send_power: wrote only %i of %i bytes\n", r, msgsz);
   pkg_len = sizeof(netmeter_req_t) + sizeof(unsigned int)*NUM_ACTIVE_NIDQ_CHANNELS;

   if(timeout_read_write (s, 0, 1) < 0)
	   fprintf(stderr, "send_power: timeout waiting client write fd.\n");
   else if ((r = send(s, &pkg, pkg_len, 0)) != pkg_len)
      fprintf(stderr, "send_power: wrote only %i of %i bytes\n", r, pkg_len);

   //printf("%d %d %d %d\n", nidaq_milli_powers[0], nidaq_milli_powers[1], nidaq_milli_powers[2], nidaq_milli_powers[3]);

   return;
}

void signal_exit(int sig) {
   running = 0;
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


DWORD WINAPI client_req(LPVOID lpParam) 
{
   char msgbuf[MSG_BUF_SIZE];
   netmeter_req_t msg;                /* message received from peer */
   control_msg_t *cmsg;                /* message received from peer */
   //int id = (int)arg;                 /* connection id number in array */
   int id = *((int*)lpParam);
   //ssize_t size = 0;                  /* amount of data read from socket */
   int r, size = 0;
   struct sockaddr_in fsin;           /* socket info of connected peer */
   int alen = sizeof(fsin);           /*   useful for debugging */
   int len;
   int rc, connected = 1;                 /* loop while connected */
   struct timeval start, stop;        /* track timings for connections */
   unsigned int hr = 0, min = 0, sec; /* connection service time info */
   unsigned long commands = 0;        /* number of commands serviced */
   fd_set cli_fdset;

   gettimeofday(&start, NULL);
   //connection[id].id = pthread_self();
   //pthread_detach(connection[id].id);
   if (getpeername(connection[id].sock, (struct sockaddr *)&fsin, &alen) == -1)
	   return -1; //(void *)-1; /* client abort */
   printf("connection %d: start client %s\n", id, inet_ntoa(fsin.sin_addr));
   while (running && connected) {

	   FD_ZERO(&cli_fdset);
	   FD_SET(connection[id].sock, &cli_fdset);

	   rc = select(connection[id].sock+1, &cli_fdset, NULL, NULL, NULL);
	   if (rc < 0)
	   {
		   fprintf(stderr, "client_req: select error.\n");
		   connected = 0;
	   }

	  memset(msgbuf, 0, sizeof(msgbuf));

	  if ((size = recv(connection[id].sock, msgbuf, sizeof(msgbuf), 0)) == -1 ){
	     fprintf(stderr, "connection %d: error request size %d errno %d\n", id, size, errno);
         connected = 0;
      } else if (!running) {
		  connected = 0;
      } else {
//printf("size(%d) %d\n", size, sizeof( netmeter_req_t ));
		 if (size!=0 && (size % sizeof(netmeter_req_t))==0) /* receive multiple package at a time, read the latest one. */
			 size = sizeof(netmeter_req_t);

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
			printf("msg from %s (size=%d): {cmd = %d len = %d txt = %s}\n", inet_ntoa(fsin.sin_addr), size, cmsg->cmd, cmsg->len, cmsg->msg);
#if 0
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
#endif
		 }
		 else{
		 	fprintf(stderr, "unexpected message size %d\n", size);
			connected = 0;
		 }
      }
   } /* end while loop */

   closesocket(connection[id].sock);
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

   return;
}

DWORD WINAPI request_handler(LPVOID lpParam) {
   int s;
   struct sockaddr_in saddr, raddr;
   int rlen;
   int i, id, rc;
   //pthread_attr_t thr_attr;
   unsigned short port;
   //struct pollfd pfd;
   //WSAPOLLFD pfd;
   struct timeval selTimeout;
   WSADATA wsaData;
   fd_set readfd_set;

   if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) /* Load Winsock 2.0 DLL */
   {
      fprintf(stderr, "WSAStartup() failed");
      exit(1);
   }

   port = *((unsigned short*)lpParam);
   printf("listening on port %i for clients\n", port);

  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s<0) {
	   fprintf(stderr, "error while creating server socket.\n");
	   exit(1);
  }

  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(port);
  saddr.sin_addr.s_addr = INADDR_ANY;

  rc = bind(s, (struct sockaddr *)&saddr, sizeof(saddr));
  if (rc == SOCKET_ERROR) {
	   fprintf(stderr, "unable to bind address.\n");
	   closesocket(s);
	   exit(1);
   }

   /* Mark the socket so it will listen for incoming connections */
   if (listen(s, MAXCONNS) < 0) {
	   fprintf(stderr, "error while listening on port %i.\n", port);
	   closesocket(s);
	   exit(1);
   }

   while(running) /* Run forever */
   {
	   FD_ZERO(&readfd_set);
	   FD_SET(s, &readfd_set);

       memset(&raddr, 0, sizeof(raddr));
       rlen = sizeof(raddr);
       for (i = 0; ; i++) {
           if (i > MAXCONNS) {
/*
			   struct timespec req, rem;
                 req.tv_sec = 1;
                 req.tv_nsec = 0;
                 nanosleep(&req, &rem);
*/
				 Sleep(100);
                 i = 0;
                 continue;
            }
		    /* cleanup client_req thread objects here? */
			/* ... */
		    if (connection[i].sock == 0)
              break;
       }

		selTimeout.tv_sec = 1;       /* timeout (secs.) */
        selTimeout.tv_usec = 0;            /* 0 microseconds */

		rc = select(s+1, &readfd_set, NULL, NULL, &selTimeout);
		if (rc < 0)
		{
			fprintf(stderr, "server select error.\n");
			running = 0;
		}
		else if (rc == 0) /* timeout */
            continue;
        else 
        {
			if (FD_ISSET(s, &readfd_set))
            {
				connection[i].sock = accept(s, (struct sockaddr *)&raddr, &rlen);
				if (connection[i].sock < 0) {
					connection[i].sock = 0;
					fprintf(stderr, "connection %d: error accept.\n", i);
					continue;
				} else {
					id = i;
//printf("creating client_req thread. i[%d], sock[%d]\n", id, connection[id].sock);
					if ((connection[i].thread = CreateThread(NULL, 0, client_req, &id, 0, NULL)) == NULL) { //CreateThread( NULL, 0, request_handler, &port, 0, NULL);
//printf("error creating client_req thread.\n");
						closesocket(connection[i].sock);
						connection[i].sock = 0;
					}
				}
			}
		}
   }

   closesocket(s);
   WSACleanup();  /* Cleanup Winsock */

   return 0;
}

/* initialize the signal handlers and additional threads */
void initialize(HANDLE *thread, unsigned short *port) {

   signal( SIGINT, signal_exit);

   if (*port == 0)
      return;
   connection = (connection_t *)calloc(MAXCONNS, sizeof(connection_t));
   if (connection == NULL) {
      fprintf(stderr, "out of memory creating %d connections\n", MAXCONNS);
      exit(1);
   }

   *thread = CreateThread(NULL, 0, request_handler, port, 0, NULL);  
   if (*thread == NULL) {
      fprintf(stderr, "unable to start network listener: returns error from request_handler\n");
      exit(1);
   }
}

int main()
{
   int count = 0;
   //FILE *output = NULL;
   int c;
   //float volts = SIM_VOLTS;
   unsigned short port = DEFAULT_METER_PORT;
   //pthread_t thread;
   int rc=0;
   HANDLE	thread;

   hMutex = CreateMutex( NULL, FALSE, NULL );

   initialize(&thread, &port);
   //rc = monitor(type, serialport, output, count, volts, to_sec);
   start_nidaq_meter();
   //getchar();

   if (port) {
      printf("disconnecting any clients...\n");
      WaitForSingleObject(thread, 0);
	  CloseHandle(thread);
   }
   printf("finished.\n");

   return rc;
}