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

#include <time.h>

#include <pthread.h>

#define ACK_SIZE 5
#define READ_MAX 100

int client_sock, server_sock, server_client_sock;
struct sockaddr_in server_addr, client_server_addr;

/* Info for client */
int client_server_port = 7001;
char client_server_ipaddr[] = "192.168.0.1";
/* Info for server */
int server_port = 7002;


int sendMsg(char* msg, size_t msg_size){
    return write(server_client_sock, msg, msg_size);
}

int ackBB(){
    return sendMsg("ACK\0", 4);
}



typedef struct {
    int server_alive;

} ArgTH;



void *server_th(void *arg){
    ArgTH* ath = (ArgTH*) arg; 
    while(ath->server_alive){

    }

    return NULL;
}

typedef struct {
    
    BBrequest* bb_request
} Reserv


typedef struct {
    int type;
    char* ipPhone1;
    int portPhone1;
    char* ipPhone2;
    int portPhone2;
    int bandwidth;
} BBrequest;



/**
 *  Exemple de donnee
 *  <type>,<source>,<destination>,<port_dst>,<debit>
 *  type: 1 ou 0
 *  destination/soruce : adresse ip : x.x.x.x
 *  debit : en kilo-octet
 * */
BBrequest* parsing(char* msg, size_t msg_size){
    BBrequest* ptr_bbr = malloc(sizeof(BBrequest));

// parse TODO
    return ptr_bbr; 
}


void process_bb_request(BBrequest* bb_request){
    //process TODO
    
}


int main(int argc, char **argv) {
    /* client */
    memset(&client_server_addr, 0, sizeof(client_server_addr));

    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("client socket creation failed");
    }
    client_server_addr.sin_family = AF_INET;
    client_server_addr.sin_port = client_server_port;
    inet_aton(client_server_ipaddr, &client_server_addr.sin_addr);
    if (connect(client_sock, (struct sockaddr *) &client_server_addr, sizeof(client_server_addr)) == -1)
        perror("Error client connect");

    /* Server */
    struct sockaddr_in *ptr_server_client_addr = malloc(sizeof(struct sockaddr_in));
    
    memset(&server_addr, 0, sizeof(server_addr));
	memset(ptr_server_client_addr, 0, sizeof(server_addr));

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error server socket ");
	}

    server_addr.sin_family = AF_INET;
	server_addr.sin_port = server_port;
	server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in)) == -1)
    {
        perror("error server bind ");
    }

    if (listen(server_sock, 50) == -1)
    {
        perror("Error listen ");
    }

    unsigned int received_msg_size;
    size_t receiv_msg_size = 0;
    char msg[READ_MAX];
    BBrequest* ptr_bbrqst = NULL;

    while(1){
        if ((server_client_sock = accept(server_sock, (struct sockaddr *) ptr_server_client_addr, &received_msg_size)) == -1) {
            perror("Error server accept");
        }

        if ((receiv_msg_size = read(server_client_sock, msg, READ_MAX - 1)) < 0) {
            perror("server read error");
        }

        ptr_bbrqst = parsing(msg, receiv_msg_size);
        if(ptr_bbrqst != NULL){
            process_bb_request(ptr_bbrqst);
            free(ptr_bbrqst);
        }

        
    }

    /*pthread_t tid;
    ArgTH argServe;
    pthread_create(&tid, NULL, server_th, (void *)argServe); 

    pthread_exit(NULL); */



}
