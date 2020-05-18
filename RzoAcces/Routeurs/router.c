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

#include <sys/time.h>
#include <time.h>
#include <string.h>

#include <pthread.h>

#include <event.h>

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
    struct BBrequest* bb_request;
} Reserv;


typedef struct {
    int type;
    char* ipPhone1;
    char* ipPhone2;
    int portPhone2;
    int bandwidth;
} BBrequest;

typedef struct {
    BBrequest *bbrequest;
    struct tm *req_time;
} TableRequests;

#define REQ_TABLE_LENGTH 100
TableRequests req_table[REQ_TABLE_LENGTH];

/**
 *  Parse a string containning the request of the BB.
 *  @return: NULL if the parsing failed, a filled BBrequest structure else
 *  @comment: 
 *  Exemple de donnee
 *  <type>,<@IPsource>,<@IPdestination>,<port_dst>,<debit>
 *  type: 1 ou 0
 *  destination/soruce : adresse ip : x.x.x.x
 *  debit : en kilo-octet
 * */
BBrequest* parsing(char* msg, size_t msg_size){
    BBrequest* ptr_bbr = malloc(sizeof(BBrequest));
    char* token;

    //Guard
    msg[msg_size-1] = '\0';

    //Type
    token = strtok(msg, ";");
    if (token == NULL) return NULL;
    ptr_bbr->type = atoi(token);
    
    //@IP1
    token = strtok(msg, ";");
    if(token == NULL) return NULL;
    ptr_bbr->ipPhone1 = malloc(strlen(token)*sizeof(char));
    strcpy(ptr_bbr->ipPhone1, token);


    //@IP2
    token = strtok(msg, ";");
    if(token == NULL) return NULL;
    ptr_bbr->ipPhone1 = malloc(strlen(token)*sizeof(char));
    strcpy(ptr_bbr->ipPhone1, token);

    //Port dest 2
    token = strtok(msg, ";");
    if(token == NULL) return NULL;
    ptr_bbr->portPhone2 = atoi(token);

    //Bandwidth
    token = strtok(msg, ";");
    if(token == NULL) return NULL;
    ptr_bbr->bandwidth = atoi(token);

    return ptr_bbr; 
}

int compare_request(BBrequest *bb_request1,BBrequest *bb_request2) {
    if ((bb_request1 == NULL) || (bb_request2 == NULL)){
        return 0;
    }
    if ((bb_request1->type == bb_request2->type) && (strcmp(bb_request1->ipPhone1,bb_request2->ipPhone1)) && (strcmp(bb_request1->ipPhone2,bb_request2->ipPhone2)) && (bb_request1->portPhone2 == bb_request2->portPhone2)) {
        return 1;
    } else {
        return 0;
    }
}

int compare_time_10_OK (struct tm *req_tm) {
    time_t t = time(NULL);
    struct tm curr_tm = *localtime(&t);

    int cur_minutes = curr_tm.tm_year*525600 + curr_tm.tm_mon*43800 + curr_tm.tm_mday*1440 + curr_tm.tm_hour*60 + curr_tm.tm_min;
    int req_minutes = req_tm->tm_year*525600 + req_tm->tm_mon*43800 + req_tm->tm_mday*1440 + req_tm->tm_hour*60 + req_tm->tm_min;

    if ((cur_minutes - req_minutes) <= 10) {
        return 1;
    } else {
        return 0;
    }
}

void copy_bb_request(BBrequest *dest, BBrequest *source) {
    dest->type = source->type;
    strcpy(dest->ipPhone1,source->ipPhone1);
    strcpy(dest->ipPhone2,source->ipPhone2);
    dest->portPhone2 = source->portPhone2;
    dest->bandwidth = source->bandwidth;
}


void process_bb_request(BBrequest* bb_request){
    if (bb_request->type == 0) {

        int present = 0;
        for (int i=0;i<REQ_TABLE_LENGTH;i++) {
            if (compare_request(req_table[i].bbrequest,bb_request)) {
                time_t t = time(NULL);
                struct tm * curr_tm = malloc(sizeof(struct tm));
                *curr_tm = *localtime(&t);
                free(req_table[i].req_time);
                req_table[i].req_time = curr_tm;
                present = 1;
            }
        }

        if (!present) {
            int j = 0;
            while((req_table[j].bbrequest!=NULL) && (j<REQ_TABLE_LENGTH)){
                j++;
            }
            if (j<REQ_TABLE_LENGTH) {
                req_table[j].bbrequest = malloc(sizeof(BBrequest));
                req_table[j].req_time = malloc(sizeof(struct tm));
                copy_bb_request(req_table[j].bbrequest, bb_request);
                time_t t = time(NULL);
                struct tm * curr_tm = malloc(sizeof(struct tm));
                *curr_tm = *localtime(&t);
                req_table[j].req_time = curr_tm;

                //Appels système

            }
        }

    } else if (bb_request->type == 1) {
        for (int i=0;i<REQ_TABLE_LENGTH;i++) {
             if (compare_request(req_table[i].bbrequest,bb_request)) {
                 //Appels système

                 free(req_table[i].bbrequest);
                 free(req_table[i].req_time);
                 req_table[i].bbrequest = NULL;
                 req_table[i].req_time = NULL;
             }
        }
    }
}

void handler_bb_request(int fd, short event, void *arg) {
    for (int i=0;i<REQ_TABLE_LENGTH;i++) {
        if (req_table[i].bbrequest!=NULL) {
            if (compare_time_10_OK(req_table[i].req_time)) {
                //Appels système
                free(req_table[i].bbrequest);
                free(req_table[i].req_time);
                req_table[i].bbrequest = NULL;
                req_table[i].req_time = NULL;
            }
        }
    }
}


int main(int argc, char **argv) {
    struct event ev;
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;

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

    event_init();
    evtimer_set(&ev, handler_bb_request, NULL);
    evtimer_add(&ev, &tv);
    event_dispatch();

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