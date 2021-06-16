#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>


#define BACKLOG 1024

#define bprintf(fp, format, ...) \
	    if(fp == NULL){printf(format, ##__VA_ARGS__);} 	\
	    else{printf(format, ##__VA_ARGS__);	\
	    fprintf(fp, format, ##__VA_ARGS__);fflush(fp);}


int sig_type = 0, sig_to_exit = 0;
FILE * fp_res = NULL;	

void sig_int(int signo) {	

}

void sig_pipe(int signo) {	
	
}

void sig_chld(int signo){
    
}

int install_sig_handlers(){
	return 0;
}


int echo_rep(int sockfd)
{

}

int main(int argc, char* argv[])
{
	
}