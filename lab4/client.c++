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

void singnal_handler(int signo)
{}

int main(int argc, char *argv[])
{	
    if(argc!=2)
        error("Enter arguments: port");
    char buf[BUFSIZE];
    int server,dpart=0;
    FILE *newfile;
    FILE *oldfile;
    char name[16];
    server = socket(AF_INET, SOCK_STREAM, 0);
    if(server<0)error("Socket error");
    sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    uint port=atoi(argv[1]);
    dest_addr.sin_port = htons(port);
    if (inet_addr("127.0.0.1") != INADDR_NONE)     
        dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  
    if (connect(server, (sockaddr *)&dest_addr, sizeof(dest_addr)))
        error("Connect error");
    struct sigaction signal;
    signal.sa_handler = singnal_handler;
    sigaction(SIGURG, &signal, NULL);
    fcntl(server, F_SETOWN, getpid());
    if(recv(server, name, sizeof(name), 0 )<0)
        error("Get name error");
    oldfile=fopen(name,"r+");
    if(oldfile != NULL)  
    {
        cout<<"\nFile is found";
        while (!feof(oldfile)) {
            int b=fread(buf,1,sizeof(buf),oldfile);
            int size=ftell(oldfile);
            dpart++;
	}
        newfile=oldfile;
    }
    else  newfile = fopen(name, "w");
    char dparts[10]; dpart--;
    sprintf(dparts,"%i",dpart);
    if(send(server,dparts,strlen(dparts),0)<=0)
        error("Send download parts error");
    cout<<"\nDownload parts:"<<dpart<<"\n";
    if (newfile == NULL) 
        error("Create file error");
    long getBytes=0;
    while (1) {
        if(sockatmark(server) == 1){
            recv(server, &buf, 1, MSG_OOB);           
            cout<<"\nDownload bytes:"<<getBytes;  
        }
        int nbytes =recv(server, buf, sizeof(buf), 0 );
        if (nbytes == 0)
        {
            cout<<"\nDisconnect.Download comlete!\n";
            break;
        }
        if (nbytes < 0)
            error("Not get byte");
        fwrite(buf,nbytes,1,newfile);
        getBytes+=nbytes;
    }
    fclose(newfile);
    close(server);
    return 0;
}
