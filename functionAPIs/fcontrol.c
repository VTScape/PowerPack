/**
 * PowerPack Function Level API
 * Author: SCAPE Lab (Bo Li)
 * Date: 05/01/2013
 * PowerPack Version 4.0
 */

#include "fcontrol.h"
#include <unistd.h>

char *hostname = METER_HOST;       /* hostname to connect to */
unsigned short port = DEFAULT_METER_PORT;  /* tcp port number */
char *lfile;

void powerpack_fp_init(char *logfile){
  char *optarg;
  control_msg_t g_msg;
  int len;
  lfile=(char *)malloc(sizeof(char)*100);
  char *file_name;
  file_name=(char *)malloc(sizeof(char)*100);
  strcpy(file_name, logfile);
  strcat(file_name, ".pwr");
  lfile=file_name;
  optarg=file_name;
  
  // Change output directory.
  optarg="/var/log/powerpack";
  g_msg.cmd = CHG_WORKING_DIR;
  len = strlen( optarg );
  if ( len >= MAX_MSG_SIZE ){
          len = MAX_MSG_SIZE-1;
          optarg[len] = '\0';
  }
  sprintf(g_msg.msg, optarg);
  g_msg.len = strlen( g_msg.msg );
  send_control_msg( hostname, port, &g_msg);

  // Start a new log file.
  g_msg.cmd = NEW_LOG_FILE;
  
  len = strlen( file_name );
  //len = strlen( optarg );
  if ( len >= MAX_MSG_SIZE ){
    len = MAX_MSG_SIZE-1;
    //optarg[len] = '\0';
  }
  //sprintf(g_msg.msg, optarg);
  sprintf(g_msg.msg, file_name);
  g_msg.len = strlen( g_msg.msg );
  send_control_msg( hostname, port, &g_msg);
}

void powerpack_fp_end(){
  // Close current log.
  char *optarg;
  control_msg_t g_msg;
  optarg="log";
  g_msg.cmd = END_LOG_FILE;
  sprintf(g_msg.msg, optarg);
  g_msg.len = strlen( g_msg.msg );
  send_control_msg( hostname, port, &g_msg);

  // SCP back to local dir.
//  char a[100]="scp 172.16.0.240:/var/log/powerpack/";
  char a[100]="scp ";
  strcat(a, METER_SERVER);
  strcat(a, ":/var/log/powerpack/");
  strcat(a, lfile); 
  strcat(a, " ./");
  strcat(a, lfile);

  char *b=NULL;
  b=(char*)calloc(sizeof(char),(39+3+2*strlen(lfile)));
//  strcat(b,"scp 172.16.0.240:/var/log/powerpack/");
  strcat(b, "scp ");
  strcat(b, METER_SERVER);
  strcat(b, ":/var/log/powerpack/");
  strcat(b,lfile);
  strcat(b, " ./");
  strcat(b, lfile);

  system(b);
}

void powerpack_fp_session_start(char *session_name){
  char *hostname = METER_HOST;       /* hostname to connect to */
  unsigned short port = DEFAULT_METER_PORT;  /* tcp port number */
  int len;
  control_msg_t g_msg;

  g_msg.cmd = NEW_SESSION;
  len = strlen( session_name );
  if ( len >= MAX_MSG_SIZE ){
    len = MAX_MSG_SIZE-1;
  }
  sprintf(g_msg.msg, session_name);
  g_msg.len = strlen( g_msg.msg );
  send_control_msg( hostname, port, &g_msg);
}

void powerpack_fp_session_end(char *session_name){
  char *optarg;
  control_msg_t g_msg;
  char s_end_name[100]="session_";
  strcat(s_end_name,session_name);
  optarg=s_end_name;
  g_msg.cmd = END_SESSION;
  sprintf(g_msg.msg, optarg);
  g_msg.len = strlen( g_msg.msg );
  send_control_msg( hostname, port, &g_msg);
}

void c_test(char *testStr){
  printf("%s\n", testStr);
}


/**
 *  interfaces for fortran programs [BEGIN]
 */
void powerpack_fp_init_(char *logfile, int strLen){
  logfile[strLen--]='\0';
  char *optarg;
  control_msg_t g_msg;
  int len;
  int trimLen=0, i=0;
  for(i=0;i<strLen;i++){
    if(logfile[i]!=' '){
      trimLen++;
    }
  }
  lfile=(char *)calloc(sizeof(char),(strLen+1+5));
  char *file_name;
  file_name=(char *)calloc(sizeof(char), (strLen+1+5));
  strcpy(file_name, logfile);
  strcat(file_name, ".pwr");
  lfile=file_name;
  optarg=file_name;
  
  // Change output directory.
  optarg="/var/log/powerpack";
  g_msg.cmd = CHG_WORKING_DIR;
  len = strlen( optarg );
  if ( len >= MAX_MSG_SIZE ){
          len = MAX_MSG_SIZE-1;
          optarg[len] = '\0';
  }
  sprintf(g_msg.msg, optarg);
  g_msg.len = strlen( g_msg.msg );
  send_control_msg( hostname, port, &g_msg);

  // Start a new log file.
  g_msg.cmd = NEW_LOG_FILE;
  
  len = strlen( file_name );
  if ( len >= MAX_MSG_SIZE ){
    len = MAX_MSG_SIZE-1;
  }
  sprintf(g_msg.msg, file_name);
  g_msg.len = strlen( g_msg.msg );
  send_control_msg( hostname, port, &g_msg);
}

void powerpack_fp_end_(){
  char *optarg;
  control_msg_t g_msg;
  // Close current log.
  optarg="log";
  g_msg.cmd = END_LOG_FILE;
  sprintf(g_msg.msg, optarg);
  g_msg.len = strlen( g_msg.msg );
  send_control_msg( hostname, port, &g_msg);

  // SCP back to local dir.
//  char a[100]="scp 172.16.0.240:/var/log/powerpack/";
  char a[100]="scp ";
  strcat(a, METER_SERVER);
  strcat(a, ":/var/log/powerpack/");
  strcat(a, lfile); 
  strcat(a, " ./");
  strcat(a, lfile);

  char *b=NULL;
  b=(char*)calloc(sizeof(char),(39+3+2*strlen(lfile)));
//  strcat(b,"scp 172.16.0.240:/var/log/powerpack/");
  strcat(b, "scp ");
  strcat(b, METER_SERVER);
  strcat(b, ":/var/log/powerpack/");
  strcat(b,lfile);
  strcat(b, " ./");
  strcat(b, lfile);

  int i=0;
  for(i=0;i<strlen(b);i++){
    printf("%c,",b[i]);
  }printf("\n");
  printf("return value : %d\n", system(b));
  printf("Command to retrieve data file: %s \n", b);
}

void powerpack_fp_session_start_(char *session_name, int strLen){
  session_name[strLen--]='\0';
  char *hostname = METER_HOST;       /* hostname to connect to */
  unsigned short port = DEFAULT_METER_PORT;  /* tcp port number */
  int len;
  control_msg_t g_msg;

  g_msg.cmd = NEW_SESSION;
  len = strlen( session_name );
  if ( len >= MAX_MSG_SIZE ){
    len = MAX_MSG_SIZE-1;
  }
  sprintf(g_msg.msg, session_name);
  g_msg.len = strlen( g_msg.msg );
  send_control_msg( hostname, port, &g_msg);
}

void powerpack_fp_session_end_(char *session_name, int strLen){
  session_name[strLen--]='\0';
  char *optarg;
  control_msg_t g_msg;
  char s_end_name[100]="session_";
  strcat(s_end_name,session_name);
  optarg=s_end_name;
  g_msg.cmd = END_SESSION;
  sprintf(g_msg.msg, optarg);
  g_msg.len = strlen( g_msg.msg );
  send_control_msg( hostname, port, &g_msg);
}

void fortran_test_(char *testStr, int strLen){
  printf("%s\n", testStr);
}
/**
 *  interfaces for fortran programs [END]
 */

int send_control_msg(char *hostname, unsigned short port, control_msg_t *msg)
{
  int sock, nb;
  if ( (sock=netmeter_connect(hostname, port)) == -1)
          return -1;
  nb = write(sock, (void *)msg, sizeof(control_msg_t));
  fprintf(stderr, "request to send %lu bytes; send out %d bytes\n", sizeof(control_msg_t), nb);
  usleep(1);
  netmeter_close(port);
  return 0;
}

/*int main(int argc, char *argv[]){
  powerpack_fp_init("testFile");
  powerpack_fp_session_start("testSession");
  powerpack_fp_session_end();
  powerpack_fp_end();
}*/
