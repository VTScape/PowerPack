#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif
#include "meter.h"

#define DEFAULT_CMD             NETMETER_CMD_ENERGY
#define DEFAULT_COUNT           1
#define DEFAULT_DELAY           1000
#define DEFAULT_ACCURACY        0
#define DEFAULT_METER_PORT      6713

//extern char *optarg;
extern int optind, opterr, optopt;
extern char *logfile;
//extern char *session_name;

#define MAX_MSG_SIZE    62
#define NEW_LOG_FILE    20
#define NEW_SESSION     21
#define END_LOG_FILE    22
#define END_SESSION     23
#define CHG_WORKING_DIR 24

#define METER_SERVER	"172.16.0.240"

typedef struct control_msg
{
        char    cmd;
        char    len;
        char    msg[MAX_MSG_SIZE];
}control_msg_t;

int  send_control_msg(char*, unsigned short, control_msg_t*);
//void show_usage(char *argv[]);

// API for C.
void powerpack_fp_init(char *);
void powerpack_fp_end();
void powerpack_fp_session_start(char *);
void powerpack_fp_session_end(char *); 
void c_test(char *);

// API for Fortran.
void powerpack_fp_init_(char*, int);
void powerpack_fp_end_();
void powerpack_fp_session_start_(char *, int);
void powerpack_fp_session_end_(char *, int); 
void fortran_test_(char*, int);

