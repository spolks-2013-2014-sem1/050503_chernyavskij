#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <fstream> 
#include <iostream>
using namespace std;

void error(const char *msg)
{
    cout<<"\n"<<msg<<"\n";
    exit(0);
}

int main(int argc, char *argv[])
{
    if(argc!=2)
        error("Enter argument");
    char *name=argv[1]; 
    int sock, length, client;
    socklen_t fromlen;
    struct sockaddr_in server;
    struct sockaddr_in from;
    FILE *f;
    char buf[128];
    sock=socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) error("Opening socket");
    length = sizeof(server);
    bzero(&server,length);
    server.sin_family=AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port=htons(3000);
    if (bind(sock,(struct sockaddr *)&server,length)<0) 
        error("Binding");
    fromlen = sizeof(struct sockaddr_in);
    while(1){
	int i=0;
        listen(sock, 1);
        client = accept(sock, (struct sockaddr *) &from, &fromlen);
        if (client < 0)error("Accept faled");
        f=fopen(name,"r");
        if(f==NULL)error("Cant open file");
        send(client,basename(name),sizeof(basename(name)),0);
        int nbytes = recv( client, buf, sizeof(buf), 0 );
        int dpart=0;
        dpart=atoi(buf);
        cout<<"\nDownload parts:"<<dpart<<"\n"<<endl;
        while (!feof(f)) {
            int b=fread(buf,1,sizeof(buf),f);
            int size=ftell(f);
            cout<<"\rbytes read:"<<b<<", part:"<<i<<" pos:"<<size;
            if(b!=0&&(dpart-1)<i)
                send(client,buf,b,0);
            i++;
	}
       fclose(f);
       close(client);
    }
    return 0;
}

