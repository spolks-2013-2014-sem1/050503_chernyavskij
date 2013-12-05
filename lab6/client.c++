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
#include <signal.h>
#include <fcntl.h>
 
using namespace std;

#define BUFSIZE 1024 

void error(const char *msg)
{
    cout<<"\n"<<msg<<"\n";
    exit(0);
}

int connectUDP(uint port)
{   
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");   
    int server = socket(AF_INET, SOCK_DGRAM, 0);  
    if(server<0)
        error("Socket error");
    if (connect(server, (sockaddr *)&server_addr, sizeof(server_addr)))
        error("Connect error");
    return server;
}

bool getFileUDP(int sock,FILE *f,struct sockaddr_in from)
{  
    socklen_t fromlen=sizeof(from);
    int nbytes=0;
    long getBytes=0;
    char buf[BUFSIZE];
    while (1) {
        nbytes =recv(sock, buf, sizeof(buf), 0);     
        if (nbytes==1&&buf[0]=='a'){
            cout<<"\nDisconnect.Download comlete!\n";
            return true;
        }  
        if (nbytes < 0)
            return false;
        fwrite(buf,nbytes,1,f);
        getBytes+=nbytes; 
        if(getBytes%(1024*1024)==0)
            cout<<"\nMB get: "<<getBytes/(1024*1024);
        if(sendto(sock,"q",1,0,(struct sockaddr *) &from,fromlen)<0)
            return false;
    } 
}

int startUDPclient(uint port)
{	
    char buf[BUFSIZE],name[16],dparts[10];
    int dpart=0;
    struct sockaddr_in server,from;
    socklen_t serverlen=sizeof(server),fromlen;
    FILE *newfile,*oldfile;
    int sock=connectUDP(port);
    if(send(sock,"client connected",16,0)<0)
        error("Connect eroor");
    if(recvfrom(sock, name, sizeof(name), 0,(struct sockaddr *) &server,&serverlen)<0)
        error("Get name error");
    oldfile=fopen(name,"r+");
    if(oldfile != NULL)  
    {
        cout<<"\nFile is found";
        while (!feof(oldfile)) {
            int b=fread(buf,1,sizeof(buf),oldfile);
            dpart++;
	}
        newfile=oldfile;
    }
    else  newfile = fopen(name, "w");
     dpart--;
    sprintf(dparts,"%i",dpart);
    if(sendto(sock,dparts,strlen(dparts),0,(struct sockaddr *) &server,serverlen )<0)
        error("Send download parts error");
    cout<<"\nDownload parts:"<<dpart;;
    if (newfile == NULL) 
        error("Create file error");
    long getBytes=0;
    if(!getFileUDP(sock,newfile,server))
        error("Packet lost");
    fclose(newfile);
    close(sock);
    return 0;
}

void singnal_handler(int signo)
{}
int connectTCP(uint port)
{
	int server = socket(AF_INET, SOCK_STREAM, 0);
    if(server<0)error("Socket error");
    sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  
    if (connect(server, (sockaddr *)&dest_addr, sizeof(dest_addr)))
        error("Connect error");
    return server;
}
int startTCPclient(uint port)
{	
    char buf[BUFSIZE];
    FILE *f;
    char name[8];
    memset(name,0,sizeof(name));
    int sock=connectTCP(port);
    struct sigaction signal;
    signal.sa_handler = singnal_handler;
    sigaction(SIGURG, &signal, NULL);
    fcntl(sock, F_SETOWN, getpid());
    if(recv(sock, name, sizeof(name), 0 )<0)
        error("Get name error");
	f = fopen(name, "w");
    if (f == NULL) 
        error("Create file error");
    long getBytes=0;
    while (1) {
        if(sockatmark(sock) == 1){
            recv(sock, &buf, 1, MSG_OOB);           
            cout<<"\nDownload bytes:"<<(getBytes/(1024*BUFSIZE));  
        }
        int nbytes =recv(sock, buf, sizeof(buf), 0 );
        if (nbytes == 0)
        {
            cout<<"\nDisconnect.Download comlete!\n";
            break;
        }
        if (nbytes < 0)
            error("Not get byte");
        fwrite(buf,nbytes,1,f);
        getBytes+=nbytes;
    }
    fclose(f);
    close(sock);
    return 0;
}
int main(int argc, char *argv[])
{
	if(argc<2)  error("Enter arguments: port");
	int s;
	cout<<"\n1-TCPclient\n2-UDPclient\nSet: ";
	cin>>s; 
	if(s==1)
		startTCPclient(atoi(argv[1]));
	if(s==2)
		startUDPclient(atoi(argv[1]));
    return 0;	
}
