#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define COMAND_LENTH 100

#define bprintf(fp, format, ...) \
	    if(fp == NULL){printf(format, ##__VA_ARGS__);} 	\
	    else{printf(format, ##__VA_ARGS__);	\
	    fprintf(fp, format, ##__VA_ARGS__);fflush(fp);}

int SigToUser;
int SigType = 0;
int SigExit = 0;
FILE* resFP = NULL;	
FILE* newFP;
pid_t recentPid;

void sig_pipe(int SigNumber);
void chld(int SigNumber);
int echo_rqt(int sockfd, int pin);
void initInt(int *a, int b);

int main(int argc, char* argv[]){
	if(argc != 4){
		printf("Usage:Please input like: %s <IP> <PORT> <CONCURRENT AMOUNT>\n", argv[0]);
		return 0;
	}

	struct sigaction actPipe, oldActPipe;
	struct sigaction actChld, oldActChld;
	struct sockaddr_in serverAddress;
    int connfd;
    int connectionAmount;

	actPipe.sa_handler = sig_pipe;
	sigemptyset(&actPipe.sa_mask);
	actPipe.sa_flags = 0;
	actPipe.sa_flags |= SA_RESTART;
	sigaction(SIGPIPE, &actPipe, &oldActPipe);
    actChld.sa_handler = chld;
    actChld.sa_flags = 0;
    actChld.sa_flags |= SA_RESTART;
    sigemptyset(&actChld.sa_mask);
    sigaction(SIGCHLD, &actChld, &oldActChld);
    initInt(&connectionAmount, atoi(argv[3]));
	pid_t pid = getpid();
    memset(&serverAddress, 0, sizeof(serverAddress));
    inet_pton(AF_INET, argv[1], &serverAddress.sin_addr);
    serverAddress.sin_port = htons(atoi(argv[2]));
    serverAddress.sin_family = AF_INET;

	for (int i = 0; i < connectionAmount - 1; i++) {
        if (fork() == 0) {
			int pin = i+1;
			char resFN[20];
            pid = getpid();
			sprintf(resFN, "stu_cli_res_%d.txt", pin);
        	resFP = fopen(resFN, "ab"); 
			if(!resFP){
				printf("[cli](%d) child exits, failed to open file \"stu_cli_res_%d.txt\"!\n", pid, pin);
				exit(-1);
			}
            //bprintf(resFP, "[cli](%d) child process %d is created!\n",pid, pin);
            while((connfd = socket(PF_INET, SOCK_STREAM, 0)) == -1);
			while (1 == 1){
				int res;
				res = connect(connfd, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
				if(!res){
					char ipString[20]={0};	//用于IP地址转换
                    //bprintf(resFP, "[cli](%d) server[%s:%d] is connected!\n", pid, \
						inet_ntop(AF_INET, &serverAddress.sin_addr, ipString, sizeof(ipString)), \
						ntohs(serverAddress.sin_port));
					if(echo_rqt(connfd, pin) == 0)break;
				}
				else
					break;	
			}

			close(connfd);
			//bprintf(resFP, "[cli](%d) connfd is closed!\n", pid);
			//bprintf(resFP, "[cli](%d) child process is going to exit!\n", pid);
            if(resFP){
                if(!fclose(resFP))
                    printf("[cli](%d) stu_cli_res_%d.txt is closed!\n", pid, pin);
            }
			exit(1);
		}
        else continue;
	}
	
	char resFN[20];
	sprintf(resFN, "stu_cli_res_%d.txt", 0);
    resFP = fopen(resFN, "wb");
	if(0 == resFP){
		printf("[cli](%d) child exits, failed to open file \"stu_cli_res_0.txt\"!\n", pid);
		exit(-1);
	}

    connfd = socket(PF_INET, SOCK_STREAM, 0);

    while (1 == 1){
		int res;
		res = connect(connfd, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
		if(res == 0){
			char ipString[20]={0};
            //bprintf(resFP, "[cli](%d) server[%s:%d] is connected!\n", pid, inet_ntop(AF_INET, &serverAddress.sin_addr, ipString, sizeof(ipString)), ntohs(serverAddress.sin_port));
			if(echo_rqt(connfd, 0) == 0)
				break;
		}
		else
			break;	
	}
    close(connfd);
	//bprintf(resFP, "[cli](%d) connfd is closed!\n", pid);
	//bprintf(resFP, "[cli](%d) parent process is going to exit!\n", pid);
    if(fclose(resFP)) return 0;
	printf("[cli](%d) stu_cli_res_0.txt is closed!\n", pid);
	return 0;
}
//---------------------------------------------------------------------
void initInt(int *a, int b){
	*a = b;
}
void sig_pipe(int SigNumber) {
	pid_t p = getpid();
    SigType = SigNumber;
    //bprintf(resFP, "[srv](%d) SIGPIPE is coming!\n", p);
}

void chld(int SigNumber){
    SigType = SigNumber;
    pid_t pid = getpid();
	pid_t chldPid = 0;
    int status;
    //bprintf(resFP, "[cli](%d) SIGCHLD is coming!\n", pid);

    while ((chldPid = waitpid(-1, &status, WNOHANG)) > 0)
        printf("[srv](%d) server child(%d) terminated.", pid, chldPid);
}


int echo_rqt(int sockfd, int pin){
	pid_t pid = getpid();
	int lenthOfH;
	int lenthOfN;
	int hPin = pin;
	int nPin;
	char fn_td[10] = {0};
	char buffer[COMAND_LENTH+9] = {0};
	int res = 0;

	initInt(&lenthOfH,0);
	initInt(&lenthOfN,0);
	initInt(&hPin, pin);
	initInt(&nPin, htonl(pin));

	sprintf(fn_td, "td%d.txt", pin);
	FILE * fp_td = fopen(fn_td, "r");
	if(!fp_td){
		//bprintf(resFP, "[cli](%d) Test data read error!\n", hPin);
		return 0;
	}

    while (fgets(buffer+8, COMAND_LENTH, fp_td)) {
		hPin = pin;
		nPin = htonl(pin);
		if(strncmp(buffer+8, "exit", 4) == 0){
			printf("[cli](%d) \"exit\" is found!\n", hPin);
			break;
		}

		memcpy(buffer, &nPin, 4);
		lenthOfH = strnlen(buffer+8, COMAND_LENTH);
		lenthOfN = htonl(lenthOfH);
		memcpy(buffer+4, &lenthOfN, 4);
        if(buffer[lenthOfH+7] == '\n') buffer[lenthOfH+8-1] = '\0';
        write(sockfd, buffer, lenthOfH+8);
		memset(buffer, 0, sizeof(buffer));
        read(sockfd, &nPin, 4);
        read(sockfd, &lenthOfN, 4);
        lenthOfH = ntohl(lenthOfN);
        read(sockfd, buffer, ntohl(lenthOfN));
        //bprintf(resFP, "[echo_rep](%d) %s\n", pid, buffer);
    }
	return 0;
}
