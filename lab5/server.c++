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

bool sendFileUDP(int sock,FILE *f,int dpart,struct sockaddr_in from)
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

int startUDPserver(uint port,char* name,FILE *f)
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
        if(!sendFileUDP(sock,f,dpart,from))
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
	return sock;
}
int startTCPserver(uint port,char*name,FILE *f)
{
    int length, client;
	int sock=connectTCP(port);
    socklen_t fromlen;
    struct sockaddr_in from;
    char buf[BUFSIZE];
    fromlen = sizeof(struct sockaddr_in);
    while(1){  
	int i=0;
        listen(sock,1);
        client = accept(sock, (struct sockaddr *) &from, &fromlen);
        send(client,name,sizeof(name),0);
        recv( client, buf, sizeof(buf), 0 );
        int dpart=atoi(buf);
        cout<<"\nDownload parts:"<<dpart<<"\n";
        int size=0;
		fseek(f, 0L, SEEK_END);
        if(dpart!=(ftell(f)/BUFSIZE)){
			fseek(f, 0L, SEEK_SET);
	        while (!feof(f)) {
	            int b=fread(buf,1,sizeof(buf),f);
	            if(b!=0&&dpart<=0)
	                send(client,buf,b,0);
	            dpart--;
	            size+=b;
	            i++;
	            if(i%1024==0&&dpart<=0){
	                cout<<"Bytes read:"<<size<<"\n";
	                send(client,"q",1,MSG_OOB);
	                sleep(1);
	            } 
	       }
		}
		cout<<"Download compleat";
		close(client);
    }
    fclose(f);
    return 0;
}

int main(int argc, char *argv[])
{
    if(argc!=3)
        error("Enter arguments: port filepath");
    FILE *f=fopen(argv[2],"r");
    if(f==NULL)error("Cant open file");
	int s;
	cout<<"\n1-TCPserver\n2-UDPserver\nSet: ";
    cin>>s;
    if(s==1)
		startTCPserver(atoi(argv[1]),basename(argv[2]),f);
	if(s==2)
		startUDPserver(atoi(argv[1]),basename(argv[2]),f);
	return 0;
}
