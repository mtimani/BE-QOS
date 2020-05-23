#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

int client_sock, server_sock, server_client_sock;
struct sockaddr_in server_addr, client_server_addr;

/* Info for client */
int client_server_port; // The port on which the client will respond 
char client_server_ipaddr[] = "127.0.0.1";

void init_client(int port){
  client_server_port = port;
  memset(&client_server_addr, 0, sizeof(client_server_addr));

  if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("client socket creation failed");
  }
  client_server_addr.sin_family = AF_INET;
  client_server_addr.sin_port = client_server_port;
  if(inet_aton(client_server_ipaddr, &client_server_addr.sin_addr) == 0)
  {    
    printf("Invalid addr\n");
    exit(-1);
  }
  printf("Init client finished \n");
}

int main(int argc, char** argv){
    char buf[200];
    int ok=1;
    if(argc < 2)
    {
      printf("Port needed \n");
      exit(EXIT_FAILURE);
    }
    init_client(atoi(argv[1]));
    printf("Connect to %s at %d \n", client_server_ipaddr, client_server_port);
    if (connect(client_sock, (struct sockaddr *) &client_server_addr, sizeof(client_server_addr)) == -1)
        perror("Error client connect");
    else{
      
      while(ok){
        printf("Write : ");
        scanf("%s", buf); // Not secure
        write(client_sock, buf, strlen(buf));
        if(strcmp(buf, "exit") == 0)
          ok = 0;
      }
    }
    
    
}