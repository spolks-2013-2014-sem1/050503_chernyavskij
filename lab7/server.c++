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
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>

using namespace std;

#define BUFSIZE 1024
FILE *f;
char *name;
void error(const char *msg)
{
    cout<<"\n"<<msg<<"\n";
    exit(0);
}
int connectUDP(uint port)
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

bool sendFileUDP(int sock,int dpart,struct sockaddr_in from)
{
    socklen_t fromlen=sizeof(from);
    char buf[BUFSIZE];
    int n=0,b=0,size=0;
    while (!feof(f)) {
        b=fread(buf,1,sizeof(buf),f);
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

int startUDPserver(uint port)
{
    struct sockaddr_in from;
    socklen_t fromlen=sizeof(from);
    char buf[BUFSIZE];
    int sock=connectUDP(port);
    while(1){  
        bzero(&from,sizeof(from));
        cout<<"\nWait connect";
        memset(buf,0,sizeof(buf));
        recvfrom(sock, buf, 16, 0,(struct sockaddr *) &from,&fromlen);
        cout<<buf;
        sendto(sock,name,sizeof(name),0,(struct sockaddr *) &from,fromlen);
        recv(sock, buf, sizeof(buf), 0);       
        int dpart=atoi(buf);
        cout<<"\nDownload parts:"<<dpart; 
        fseek(f, 0L, SEEK_END);
        if(dpart!=(ftell(f)/BUFSIZE)){   
       		fseek(f, 0L, SEEK_SET);
        	if(!sendFileUDP(sock,dpart,from))
        	{
        	    cout<<"\nPacket lost";
        	    continue;
        	}
    	}
        cout<<"\nDownload compleat";
        sendto(sock,"a",1,0,(struct sockaddr *) &from,fromlen);  
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

void* sendFileTCP(void* socketDescriptor)
{
	pthread_detach(pthread_self());
    int sock = (intptr_t) socketDescriptor;
	long size=0;
	intptr_t error=-1;
	FILE *file=fopen(name,"r");
	char buf[BUFSIZE];
	if(send(sock,basename(name),sizeof(basename(name)),0)<0)
		return (void*)error;
	if(recv( sock, buf, sizeof(buf), 0 )<0)
		return (void*)error;
	int dpart=atoi(buf),i=0;
	cout<<"\nDownload parts:"<<dpart;
	fseek(file, 0L, SEEK_END);
    if(dpart!=(ftell(file)/BUFSIZE)){
		fseek(file, 0L, SEEK_SET);
		while (!feof(file)) {
		    int b=fread(buf,1,sizeof(buf),file);
    	    if(b!=0&&dpart<=0)
		        if(send(sock,buf,b,0)<0)
					return (void*)error;
			dpart--;
    	    size+=b;
    	    i++;
    	    if(i%1024==0&&dpart<=0){
    	    	cout<<"\nBytes read:"<<(size/(1024*1024));
    	        if(send(sock,"q",1,MSG_OOB)<0)
					return (void*)error;
    	        sleep(1);
    	    } 
		}
	}	
	fclose(file);
	close(sock);
	return NULL;
}

int startTCPserver(uint port)
{
	int sock=connectTCP(port);
    socklen_t fromlen;
    struct sockaddr_in from;   
    fromlen = sizeof(struct sockaddr_in);
    while(1){  
		fseek(f, 0L, SEEK_SET);
        intptr_t client = accept(sock, (struct sockaddr *) &from, &fromlen);
		pthread_t th;
        pthread_create(&th, NULL, sendFileTCP, (void*)client);     
    }
    fclose(f);
    return 0;
}

int main(int argc, char *argv[])
{
    if(argc!=3)
        error("Enter arguments: port filepath");
    f=fopen(argv[2],"r");
    if(f==NULL)error("Cant open file");
	int s;
	name=argv[2];
	cout<<"\n1-TCPserver\n2-UDPserver\nSet: ";
    cin>>s;
    if(s==1)
		startTCPserver(atoi(argv[1]));
	if(s==2)
		startUDPserver(atoi(argv[1]));
	return 0;
}
