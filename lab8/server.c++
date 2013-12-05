#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <iostream>
using namespace std;
#define BUFSIZE 1024

char *name;
uint port;

void error(const char *msg)
{
    cout<<"\n"<<msg<<"\n";
    exit(0);
}

int connectUDP()
{  
    struct sockaddr_in server;
    int sock=socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) 
        error("Opening socket");
    bzero(&server,sizeof(server));
    server.sin_family=AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port=htons(port);
    if (bind(sock,(struct sockaddr *)&server,sizeof(server))<0) 
        error("Binding error");
    return sock;
}

bool sendFileUDP(int sock,struct sockaddr_in from)
{
	FILE *file=fopen(name,"r");
    socklen_t fromlen=sizeof(from);
    char buf[BUFSIZE];
    int n=0,b=0,size=0;
	sendto(sock,basename(name),strlen(basename(name)),0,(struct sockaddr *) &from,fromlen);
	recv(sock, buf, sizeof(buf), 0);       
    int dpart=atoi(buf);
    cout<<"\nDownload parts:"<<dpart; 
    while (!feof(file)) {
        b=fread(buf,1,sizeof(buf),file);
        if(b!=0&&dpart<=0){
            n=sendto(sock,buf,b,0,(struct sockaddr *) &from,fromlen);
            if(n<0)
                return false;
            n=recv(sock, buf, 1, 0);
            if (buf[0]!='q')
                return false;
        }
        dpart--;
        size+=b;
        if(size%(1024*1024)==0&&dpart<=0)
            cout<<"\nMB send:"<<size/(1024*1024);
   }
   return true;
}

int startUDPserver()
{
    struct sockaddr_in from;
    socklen_t fromlen=sizeof(from);
    char buf[BUFSIZE];
    int sock=connectUDP();
    while(1){  
        bzero(&from,sizeof(from));
        cout<<"\nWait connect";
        memset(buf,0,sizeof(buf));
        recvfrom(sock, buf, 16, 0,(struct sockaddr *) &from,&fromlen);
        cout<<buf;
		switch(fork()){
			case 0:{
				sendFileUDP(sock,from);
				cout<<"\nDownload compleat";
        		sendto(sock,"a",1,0,(struct sockaddr *) &from,fromlen); 
				exit(0);
			}
			default :{
				sleep(1);
				continue;
			}
		}
         
    }
    close(sock);
    fclose(f);
    return 0;
}

int connectTCP(uint port)
{	
	struct sockaddr_in server;
	int sock=socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) error("Opening socket");
    int length = sizeof(server);
    bzero(&server,length);
    server.sin_family=AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port=htons(port);
    if (bind(sock,(struct sockaddr *)&server,length)<0) 
        error("Binding error");
	listen(sock,8);
	return sock;
}

void sendFileTCP(int sock)
{
	FILE *file=fopen(name,"r");
	char buf[BUFSIZE];
	if(send(sock,basename(name),strlen(basename(name)),0)<0)
		error("Error send filename");
	if(recv( sock, buf, sizeof(buf), 0 )<0)
		error("Error get data");
	int dpart=atoi(buf),i=0,size=0;
	cout<<"\nDownload parts:"<<dpart;
	fseek(file, 0L, SEEK_END);
    if(dpart!=(ftell(file)/BUFSIZE)){
		fseek(file, 0L, SEEK_SET);
		while (!feof(file)) {
		    int b=fread(buf,1,sizeof(buf),file);
    	    if(b!=0&&dpart<=0)
		        if(send(sock,buf,b,0)<0)
					error("Error send file");
			dpart--;
    	    size+=b;
    	    i++;
    	    if(i%1024==0&&dpart<=0){
    	    	cout<<"\nBytes read:"<<(size/(1024*1024));
    	        if(send(sock,"q",1,MSG_OOB)<0)
					error("Error send oob msg");
    	        sleep(1);
    	    } 
		}
	}	
	fclose(file);
}

void startTCPserver()
{
    int length, client;
	int sock=connectTCP(port);
    socklen_t fromlen;
    struct sockaddr_in from;
    char buf[BUFSIZE];
    fromlen = sizeof(struct sockaddr_in);
    while(1){  
        client = accept(sock, (struct sockaddr *) &from, &fromlen);
		if(client < 0) error("Eror accepting");
		switch(fork()){
			case -1: error("Error creting process");
			case 0:  sendFileTCP(client);
			default:close(client);
			break;
		}	
	}
}

int main(int argc, char *argv[])
{
    if(argc!=3)
        error("Enter arguments: port filepath");
    FILE *f=fopen(argv[2],"r");
	if(f==NULL)
		error("Error open file");
	name=argv[2];
    if(f==NULL)error("Cant open file");
	port=atoi(argv[1]);	
	int s;
	cout<<"\n1-TCPserver\n2-UDPserver\nSet: ";
    cin>>s;
    if(s==1)
		startTCPserver();
	if(s==2)
		startUDPserver();
	return 0;
}
