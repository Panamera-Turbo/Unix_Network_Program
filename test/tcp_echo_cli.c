#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>

#define MAX_CMD_STR 100

#define bprintf(fp, format, ...) \
	    if(fp == NULL){printf(format, ##__VA_ARGS__);} 	\
	    else{printf(format, ##__VA_ARGS__);	\
	    fprintf(fp, format, ##__VA_ARGS__);fflush(fp);}


// 设置全局变量
int sig_type = 0;
FILE * fp_res = NULL;

void sPipe(int signo) {
}

void sChild(int signo){
}


/*
业务函数，构造PDU，发送到服务器端，并接收回送
*/

int echo_rqt(int sockfd, int pin)
{
	return 0;
}

int main(int argc, char* argv[])
{
	sPipe(0);
	Schild(0);
	echo_rqt(6668, 9);
	return 0;
}