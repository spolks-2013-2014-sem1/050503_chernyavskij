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

int main(int argc, char *argv[])
{
    if(argc!=3)
        error("Enter arguments: port filepath");
    char *name=argv[2]; 
    int sock, length, client;
    socklen_t fromlen;
    struct sockaddr_in server;
    struct sockaddr_in from;
    FILE *f=fopen(name,"r");
    if(f==NULL)error("Cant open file");
    char buf[BUFSIZE];
    sock=socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) error("Opening socket");
    length = sizeof(server);
    bzero(&server,length);
    server.sin_family=AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    uint port=atoi(argv[1]);
    server.sin_port=htons(port);
    if (bind(sock,(struct sockaddr *)&server,length)<0) 
        error("Binding error");
    fromlen = sizeof(struct sockaddr_in);
    while(1){  
        fseek(f, 0L, SEEK_SET);
	int i=0;
        listen(sock,1);
        client = accept(sock, (struct sockaddr *) &from, &fromlen);
        send(client,basename(name),sizeof(basename(name)),0);
        recv( client, buf, sizeof(buf), 0 );
        int dpart=atoi(buf);
        cout<<"\nDownload parts:"<<dpart<<"\n";
        int size=0;
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
       cout<<"Download compleat";
       close(client);
    }
    fclose(f);
    return 0;
}
