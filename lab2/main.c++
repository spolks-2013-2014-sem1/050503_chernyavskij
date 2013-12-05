#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <arpa/inet.h>

void error(const char *msg)
{
    write(1,msg,strlen(msg));
    exit(0);
}

int main(int argc, char *argv[])
{
   int sock, length, client;
   socklen_t fromlen;
   struct sockaddr_in server;
   struct sockaddr_in from;
   char buf[1024];
   sock=socket(AF_INET, SOCK_STREAM, 0);
   if (sock < 0) error("Opening socket");
   length = sizeof(server);
   bzero(&server,length);
   server.sin_family=AF_INET;
   server.sin_addr.s_addr = INADDR_ANY;
   server.sin_port=htons(3000);
   if (bind(sock,(struct sockaddr *)&server,length)<0) 
       error("binding");
   fromlen = sizeof(struct sockaddr_in);
   while(1){
       listen(sock, 1);
       client = accept(sock, (struct sockaddr *) &from, &fromlen);
       if (client < 0) 
           error("accept faled");
       while (1) {
           memset(buf,0,1024);
           int s=read(client,buf,1024);
           if(s==0)
               break;  
           write(client,buf,strlen(buf));
       }
   close(client);
   }
   return 0;
 }

