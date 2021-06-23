#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#define bprintf(fp, format, ...) \
	    if(fp == NULL){printf(format, ##__VA_ARGS__);} 	\
	    else{printf(format, ##__VA_ARGS__);	\
	    fprintf(fp, format, ##__VA_ARGS__);fflush(fp);}	
#define LOG 1024

int SigToUser;
int SigType = 0;
int SigExit = 0;
FILE* resFP = NULL;	
FILE* newFP;
pid_t recentPid;

void sig_int(int SigNumber);
void sig_pipe(int SigNumber);
void chld(int SigNumber);
void initInt(int *a, int b);
int sigHandlerInstall();
int echo_rep(int sockfd);


int main(int argc, char* argv[]){
	if(argc != 3){
		printf("Usage:Please input like: %s <IP> <PORT>\n", argv[0]);
		return -1;
	}
	pid_t pid = getpid();
	char IPaddress[20]={0};
	char FnResult[20]={0};
	int r;
	initInt(&r, sigHandlerInstall());
    if (r != 0) return r;
	resFP = fopen("stu_srv_res_p.txt", "wb");
	if(!resFP){
		printf("[srv](%d) failed to open file \"stu_srv_res_p.txt\"!\n", pid);
		return r;
	}
	
	socklen_t clientAddressLenth;
	struct sockaddr_in serverAddress, clientAddress;
    int fdListen, fdConnection;

    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
    serverAddress.sin_port = htons(atoi(argv[2]));
    inet_ntop(AF_INET, &serverAddress.sin_addr, IPaddress, sizeof(IPaddress));
    bprintf(resFP, "[srv](%d) server[%s:%d] is initializing!\n", pid, IPaddress, (int)ntohs(serverAddress.sin_port));
    fdListen = socket(PF_INET, SOCK_STREAM, 0);
    if (fdListen < 0) return 0;
    r = bind(fdListen, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
    if (r) return r;
    r = listen(fdListen, LOG);
	while(!SigExit){
        clientAddressLenth = sizeof(clientAddress);
	    fdConnection = accept(fdListen, (struct sockaddr*) &clientAddress, &clientAddressLenth);
		if(fdConnection == -1 && errno == EINTR){
			if(SigType == SIGINT)
				break;
			continue;
		}
        inet_ntop(AF_INET, &clientAddress.sin_addr, IPaddress, sizeof(IPaddress));
        //bprintf(resFP, "[srv](%d) client[%s:%d] is accepted!\n", pid, IPaddress, (int)ntohs(clientAddress.sin_port));
		if(!fork()){
			pid = getpid();
			sprintf(FnResult, "stu_srv_res_%d.txt", pid);
			resFP = fopen(FnResult, "wb");
			if(!resFP){
				printf("[srv](%d) child exits, failed to open file \"stu_srv_res_%d.txt\"!\n", pid, pid);
				exit(-1);
			}
            bprintf(resFP, "[srv](%d) child process is created!\n", pid);
            printf("[srv](%d) stu_srv_res_%d.txt is opened!\n", pid, pid);
            close(fdListen);
            bprintf(resFP, "[srv](%d) fdListen is closed!\n", pid);
			int pin;
			initInt(&pin, echo_rep(fdConnection));
			
			if(pin < 0){
				//bprintf(resFP, "[srv](%d) PIN error!\n", pid);
				exit(-1);
			}
			char FnResult_n[20]={0};
			sprintf(FnResult_n, "stu_srv_res_%d.txt", pin);
			if(!rename(FnResult, FnResult_n)){
				//bprintf(resFP, "[srv](%d) res file rename done!\n", pid);
			}
			else{			
				//bprintf(resFP, "[srv](%d) child exits, res file rename failed!\n", pid);
			}
            close(fdConnection);
            //bprintf(resFP, "[srv](%d) fdConnection is closed!\n", pid);
            //bprintf(resFP, "[srv](%d) child process is going to exit!\n", pid);
            if(!fclose(resFP)) printf("[srv](%d) stu_srv_res_%d.txt is closed!\n", pid, pin);
            exit(1);
		}
		else{
		    close(fdConnection);
            continue;
		}
	}
    close(fdListen);
	//bprintf(resFP, "[srv](%d) fdListen is closed!\n", pid);
	//bprintf(resFP, "[srv](%d) parent process is going to exit!\n", pid);
    if(!fclose(resFP))return 0;
	printf("[srv](%d) stu_srv_res_p.txt is closed!\n", pid);
	return 0;
}
//------------------------------------------------------------------------------------------------------
void sig_int(int SigNumber) {	
    SigExit = 1;
    SigType = SigNumber;
    //bprintf(resFP, "[srv](%d) SIGINT is coming!\n", getpid());
}
void sig_pipe(int SigNumber) {	
    SigType = SigNumber;
    //bprintf(resFP, "[srv](%d) SIGPIPE is coming!\n", getpid());
}
void initInt(int *a, int b){
	*a = b;
}
void chld(int SigNumber){
    SigType = SigNumber;
    pid_t pid = getpid();
	pid_t chldPid = 0;
    int s;
    //bprintf(resFP, "[cli](%d) SIGCHLD is coming!\n", pid);
    while((chldPid = waitpid(-1, &s, WNOHANG)) > 0)
        bprintf(resFP, "[cli](%d) client child(%d) terminated!.\n", pid, chldPid);
}

int sigHandlerInstall(){
	int r;
	initInt(&r,1);
	struct sigaction sActPipe, sActPipeOld;
	sActPipe.sa_handler = sig_pipe;
	sActPipe.sa_flags = 0;
	sActPipe.sa_flags = SA_RESTART | sActPipe.sa_flags;
	sigemptyset(&sActPipe.sa_mask);
	r = sigaction(SIGPIPE, &sActPipe, &sActPipeOld);
	if(r) return -1;
    struct sigaction sActChld, sActChldOld;
    sActChld.sa_handler = chld;
    sActChld.sa_flags = 0;
    sActChld.sa_flags |= SA_RESTART;
    sigemptyset(&sActChld.sa_mask);
    r = sigaction(SIGCHLD, &sActChld, &sActChldOld);
    if (r) return -2;
    struct sigaction sActInt, sActIntOld;
    sActInt.sa_handler = sig_int;
    sActInt.sa_flags = 0;
    sigemptyset(&sActInt.sa_mask);
    r = sigaction(SIGINT, &sActInt, &sActIntOld);
    if (r) return -3;

	return 0;
}

int echo_rep(int sockfd){
	int lenthOfH;
	int lenthOfN;	
    int hPin;
	int nPin; 
	int r;
	char *echoRepBuffer = NULL;
	pid_t pid = getpid();
	initInt(&lenthOfH, -1);
	initInt(&lenthOfN, -1);
	initInt(&hPin, -1);
	initInt(&nPin, -1);
	initInt(&r, 0);

    while(1>0){
		while(1>0){
			r = read(sockfd, &nPin, sizeof(nPin));
			if(r < 0){
				//bprintf(resFP, "[srv](%d) read pin_n return %d and errno is %d!\n", pid, r, errno);
				if(errno == EINTR){
					if(SigType == SIGINT)
						return hPin;
					continue;
				}
				return hPin;
			}
			if(r == 0){
				return hPin;
			}
            hPin = ntohl(nPin);
			break;				
		}

		while(1>0){
			r = read(sockfd, &lenthOfN, sizeof(lenthOfN));
			if(r < 0){
				//bprintf(resFP, "[srv](%d) read len_n return %d and errno is %d\n", pid, r, errno);
				if(errno == EINTR){
					if(SigType == SIGINT)
						return lenthOfH;
					continue;
				}
				return lenthOfH;
			}
			if(r==0){
				return lenthOfH;
			}
            lenthOfH = ntohl(lenthOfN);
			break;
		}

        int readAmount;
		int lenthToRead;		
		initInt(&readAmount, 0);
		initInt(&lenthToRead, lenthOfH);

		echoRepBuffer = (char*)malloc(lenthOfH * sizeof(char)+8); 

		while(1>0){
            r = read(sockfd, &echoRepBuffer[readAmount]+8, lenthToRead);
			if(r < 0){
				//bprintf(resFP, "[srv](%d) read data return %d and errno is %d,\n", pid, r, errno);
				if(errno == EINTR){
					if(SigType == SIGINT){
						free(echoRepBuffer);
						return hPin;
					}
					continue;
				}
				free(echoRepBuffer);
				return hPin;
			}
			if(!r){
				free(echoRepBuffer);
				return hPin;
			}

            readAmount += r;
            if(readAmount == lenthOfH) break;
            else if(readAmount < lenthOfH) lenthToRead = lenthOfH - readAmount;
            else{
                free(echoRepBuffer);
                return hPin;
            }
		}
        //bprintf(resFP, "[echo_rqt](%d) %s\n", pid, echoRepBuffer+8);
		memcpy(echoRepBuffer, &nPin, 4);
		memcpy(echoRepBuffer+4, &lenthOfN, 4);
        write(sockfd, echoRepBuffer, lenthOfH+8);
        free(echoRepBuffer);
    }
    return hPin;
}