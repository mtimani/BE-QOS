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
#include<stdarg.h>
#include <pthread.h>

//#include <event.h>

#include<signal.h>

/** Functions definitions **/
#define CONCAT_BUF_SIZE 500
char concat_buf[CONCAT_BUF_SIZE]; //Int to String buffer
char* concat(char* str1, char* str2);
char* concat3(char* str1, char* str2, char* str3);
char* concat5(char* str1, char* str2, char* str3, char* s4, char* s5);
char* concat6(char* str1, char* str2, char* str3, char* s4, char* s5, char* s6);
char* concat7(char* str1, char* str2, char* str3, char* s4, char* s5, char* s6, char* s7);
char* concat8(char* str1, char* str2, char* str3, char* s4, char* s5, char* s6, char* s7, char* s8);
char* concat9(char* str1, char* str2, char* str3, char* s4, char* s5, char* s6, char* s7, char* s8, char* s9);
char* sconcat(int num, ...);

#define ACK_SIZE
#define READ_MAX 100

#define BB_IP_ADDR "127.0.0.1"

int client_sock, server_sock, server_client_sock;
struct sockaddr_in server_addr, client_server_addr;

/* Info for client */
int client_server_port = 7002; // The port on which the client will respond 
char client_server_ipaddr[] = BB_IP_ADDR;

void init_client(){
    memset(&client_server_addr, 0, sizeof(client_server_addr));

    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("client socket creation failed");
    }
    client_server_addr.sin_family = AF_INET;
    client_server_addr.sin_port = client_server_port;
    inet_aton(client_server_ipaddr, &client_server_addr.sin_addr);
}


/* Info for server */
int server_port;
struct sockaddr_in *ptr_server_client_addr;
void init_server(int port){

    server_port = port;
    ptr_server_client_addr = malloc(sizeof(struct sockaddr_in));
    
    memset(&server_addr, 0, sizeof(server_addr));
	memset(ptr_server_client_addr, 0, sizeof(server_addr));

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error server socket ");
        exit(EXIT_FAILURE);
	}

    server_addr.sin_family = AF_INET;
	server_addr.sin_port = server_port;
	server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in)) == -1)
    {
        perror("error server bind ");
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 50) == -1)
    {
        perror("Error listen ");
        exit(EXIT_FAILURE);
    }
}

int connect_server(){
    int conn = connect(client_sock, (struct sockaddr *) &client_server_addr, sizeof(client_server_addr));
    if(conn == -1){
        perror("Error client connect");
        exit(-1);
    }
    return conn;
}


int sendMsg(char* msg, size_t msg_size){
    return write(server_client_sock, msg, msg_size);
}

void ackBB(){
    int err = sendMsg("OK\0", 4);
    if(err == -1){
        perror("Error ack to server ");
        exit(EXIT_FAILURE);
    }
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


/**
 * Struture representing a request from the Bandwidth Broker
 * */
typedef struct {
    int type;
    char* ipPhone1;
    char* ipPhone2;
    int portPhone2;
    unsigned long long bandwidth;
} BBrequest;

typedef struct{
    BBrequest *bbrequest;
    int classid;
} BBreq_ClassID; //Identify a request with a BBrequest and its associated classid 

#define CLASS_ARR_SIZE 100
typedef struct{
    BBreq_ClassID classArray[CLASS_ARR_SIZE];
    int offset_id;
} ClassTable;

ClassTable classTable;


typedef struct {
    BBrequest *bbrequest;
    struct tm *req_time;
} TableRequests;

#define REQ_TABLE_LENGTH CLASS_ARR_SIZE // Has to be the same either there will be an segfault 
//because of array access according to the same size of both array

TableRequests req_table[REQ_TABLE_LENGTH];

void init_request_components(){
    classTable.offset_id = 20;
    for(int i=0; i<REQ_TABLE_LENGTH; i++){
        req_table[i].bbrequest = NULL;
        classTable.classArray[i].bbrequest = NULL;
    }
    
}

/**
 *  Parse a string containning the request of the BB.
 *  @return: NULL if the parsing failed, a filled BBrequest structure else
 *  @comment: The BBrequest strucutre is malloc then need to be freed
 *
 *  Exemple de donnee
 *  <type>,<@IPsource>,<@IPdestination>,<port_dst>,<debit>
 *  type: 1 ou 0
 *  destination/source : adresse ip : "x.x.x.x"
 *  debit : en kilo-octet
 * */
BBrequest* parsing(char* msg, size_t msg_size){
    BBrequest* ptr_bbr = malloc(sizeof(BBrequest));
    char* token;
    char delim[] = ";";

    //Guard
    msg[msg_size] = '\0';

    //Type
    token = strtok(msg, delim);
    if (token == NULL) return NULL;
    ptr_bbr->type = atoi(token);
    
    //@IP1
    token = strtok(NULL, delim);
    if(token == NULL) return NULL;
    ptr_bbr->ipPhone1 = malloc((1+strlen(token))*sizeof(char));
    strcpy(ptr_bbr->ipPhone1, token);


    //@IP2
    token = strtok(NULL, delim);
    if(token == NULL) return NULL;
    ptr_bbr->ipPhone2 = malloc((1+strlen(token))*sizeof(char));
    strcpy(ptr_bbr->ipPhone2, token);

    //Port dest 2
    token = strtok(NULL, delim);
    if(token == NULL) return NULL;
    ptr_bbr->portPhone2 = atoi(token);

    //Bandwidth
    token = strtok(NULL, delim);
    if(token == NULL) return NULL;
    ptr_bbr->bandwidth = atoi(token);

    return ptr_bbr; 
}

void print_bbrequest(BBrequest* bbreq){
    printf("BB request\n");
    printf("Type: %d\n", bbreq->type);
    printf("@IPsrc: %s\n", bbreq->ipPhone1);
    printf("@IPdst: %s\n", bbreq->ipPhone2);
    printf("@Port_dst: %d\n", bbreq->portPhone2);
    printf("BW: %lld\n", bbreq->bandwidth);
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



#define DEFAULT_INTERFACE "lo"
#define PREMIUM_BW "300kbit"
#define BEST_EFFORT_BW "700kbit"
#define PHONE_BW "200kbit"
#define PHONE_BW_CEIL "300kbit"



void router_init_rules(){

    char buffer[100];
    //Delete the queuing discipline in case it already exist
    printf("%s", sconcat(3, "tc qdisc del dev ",DEFAULT_INTERFACE, " root\n"));
    system(sconcat(3, "tc qdisc del dev ",DEFAULT_INTERFACE, " root"));

    //Create a new queueing discipline
    printf("%s", sconcat(3, "tc qdisc add dev ",DEFAULT_INTERFACE, " root handle 1: htb default 20\n"));
    system(sconcat(3, "tc qdisc add dev ",DEFAULT_INTERFACE, " root handle 1: htb default 20"));

    //Adding of the two main queue for premium class and best effort class
    printf("%s", sconcat(7, "tc class add dev ", DEFAULT_INTERFACE, " parent 1: classid 1:1 htb rate ", PREMIUM_BW, " ceil ", PREMIUM_BW, "\n"));
    system(sconcat(6, "tc class add dev ", DEFAULT_INTERFACE, " parent 1: classid 1:1 htb rate ", PREMIUM_BW, " ceil ", PREMIUM_BW));
    printf("%s", sconcat(7, "tc class add dev ", DEFAULT_INTERFACE, " parent 1: classid 1:2 htb rate ", BEST_EFFORT_BW, " ceil ", PREMIUM_BW, "\n"));
    system(sconcat(6, "tc class add dev ", DEFAULT_INTERFACE, " parent 1: classid 1:2 htb rate ", BEST_EFFORT_BW, " ceil ", PREMIUM_BW));

    //Definition of the type of flow that has to be handled by either best effort queue (classid=1:2 and marking 20) or  premium queue (classid=1:1 and marking 1)
    printf("%s", concat3("tc filter add dev ", DEFAULT_INTERFACE, " parent 1:0 protocol ip prio 1 handle 20 fw flowid 1:2\n"));
    system(concat3("tc filter add dev ", DEFAULT_INTERFACE, " parent 1:0 protocol ip prio 1 handle 20 fw flowid 1:2"));
    
                            //printf("%s", sconcat(3, "tc filter add dev ", DEFAULT_INTERFACE, " parent 1:0 protocol ip prio 1 handle 1 fw flowid 1:1\n"));
                            //system(sconcat(3, "tc filter add dev ", DEFAULT_INTERFACE, " parent 1:0 protocol ip prio 1 handle 1 fw flowid 1:1"));

    printf("%s", sconcat(3, "tc filter add dev ", DEFAULT_INTERFACE, " parent 1:0 protocol ip prio 1 handle 1 fw flowid 1:1\n"));
    //system(sconcat(3, "tc filter add dev ", DEFAULT_INTERFACE, " parent 1:0 protocol ip prio 1 handle 1 fw flowid 1:1"));

                    //printf("iptabkes -N default_chain_router_qos\0");
                    //system("iptabkes -N default_chain_router_qos");


    //By default all flow have to be handle by the best effort queue (marking 20)
    //Setting the dscp to 0 to avoid rogue host for self-marking
        // PRE marking
    printf("iptables -A PREROUTING -t mangle -j MARK --set-mark 20\n"); 
    system("iptables -A PREROUTING -t mangle -j MARK --set-mark 20");
        //POST DSCP
    printf("iptables -A POSTROUTING -t mangle -j DSCP --set-dscp 0\n"); 
    system("iptables -A POSTROUTING -t mangle -j DSCP --set-dscp 0");
    
            //printf("iptables -A POSTROUTING -t mangle -j DSCP --set-dscp 0 RETURN\0");
            //system("iptables -A POSTROUTING -t mangle -j DSCP --set-dscp 0 RETURN");
                
            //system("iptables -A POSTROUTING -t mangle -j DSCP --set-dscp 0");
            //system("iptables -A POSTROUTING -t mangle -j MARK --set-mark 20");

    
    
    //The flow from the Bandwidth Broker which use tcp (whose IP@ is 192.168.0.1) toward this server instance (port=server_port) has to be handle by the premium queue (marking 1)
    sprintf(buffer, "%d", server_port);
        //PRE Marking
    printf("%s\n", sconcat(4, "iptables -A PREROUTING -t mangle -s ", BB_IP_ADDR, "-p -tcp --dport ", buffer ," -j MARK --set-mark 1"));
    system(sconcat(4, "iptables -A PREROUTING -t mangle -s ", BB_IP_ADDR, "-p -tcp --dport ", buffer ," -j MARK --set-mark 1"));
        //POST DSCP
    printf("%s\n", sconcat(4, "iptables -A POSTROUTING -t mangle -s ", BB_IP_ADDR, "-p -tcp --dport ", buffer ," -j DSCP --set-dscp class EF"));
    system(sconcat(4, "iptables -A PREROUTING -t mangle -s ", BB_IP_ADDR, "-p -tcp --dport ", buffer ," -j MARK --set-mark 1 DSCP --set-dscp class EF"));

}

void router_add_rule(BBrequest *bb_request){
    int i=0;
    while(i<CLASS_ARR_SIZE && bb_request != NULL) i++;

    if(i<CLASS_ARR_SIZE){
        char buffer[100];
        classTable.classArray[i].bbrequest = bb_request;
        classTable.classArray[i].classid = classTable.offset_id + i;
        sprintf(buffer, "%d", classTable.offset_id + i);
        // Creation of the queue in the HTB QDISC
        printf("%s", sconcat(9, "tc class add dev ", DEFAULT_INTERFACE, " parent 1:1 classid 1:", buffer ," htb rate ", PHONE_BW, " ceil", PHONE_BW_CEIL, "\n"));
        system(sconcat(8, "tc class add dev ", DEFAULT_INTERFACE, " parent 1:1 classid 1:", buffer ," htb rate ", PHONE_BW, " ceil", PHONE_BW_CEIL));
        
        //Specification of the data flux 
        sprintf(buffer, "%d", bb_request->portPhone2);
            //PRE marking
        printf("%s", sconcat(7, "iptables -A PREROUTING -t mangle -I FORWARD 1 -s ", bb_request->ipPhone1, " -d ", bb_request->ipPhone2, " -p tcp --dport ", buffer, " -j MARK --set-mark 1\n"));
        system(sconcat(7, "iptables -A PREROUTING -t mangle -I FORWARD 1 -s ", bb_request->ipPhone1, " -d ", bb_request->ipPhone2, " -p tcp --dport ", buffer, " -j DSCP --set-dscp EF "));
            //POST DSCP
        printf("%s", sconcat(7, "iptables -A POSTROUTING -t mangle -I FORWARD 1 -s ", bb_request->ipPhone1, " -d ", bb_request->ipPhone2, " -p tcp --dport ", buffer, " -j DSCP --set-dscp class EF\n"));
        system(sconcat(7, "iptables -A POSTROUTING -t mangle -I FORWARD 1 -s ", bb_request->ipPhone1, " -d ", bb_request->ipPhone2, " -p tcp --dport ", buffer, " -j DSCP --set-dscp class EF"));

        printf("Rule added for %s | %s | %d \n", bb_request->ipPhone1, bb_request->ipPhone2, bb_request->portPhone2);
    }
    
}

void router_del_rule(BBrequest *bb_request){
    int i=0;
    char buffer[100];
    while(i<CLASS_ARR_SIZE && !compare_request(classTable.classArray[i].bbrequest, bb_request)) i++;

    if(i<CLASS_ARR_SIZE){
        //Deletion of the queue in HTB QDISC
        sprintf(buffer, "%d", classTable.classArray[i].classid);
        printf("%s", sconcat(9, "tc class del dev ", DEFAULT_INTERFACE, " parent 1:1 classid 1:", buffer ," htb rate ", PHONE_BW, " ceil", PHONE_BW_CEIL, "\n"));
        system(sconcat(8, "tc class del dev ", DEFAULT_INTERFACE, " parent 1:1 classid 1:", buffer ," htb rate ", PHONE_BW, " ceil", PHONE_BW_CEIL));
        
        //Deletion of the marking for the data flux
        sprintf(buffer, "%d", bb_request->portPhone2);
            //PRE marking
        printf("%s\n", sconcat(7, "iptables -D PREROUTING -t mangle -s ", bb_request->ipPhone1, " -d ", bb_request->ipPhone2, " -p tcp --dport ", buffer, " -j MARK --set-mark 1"));
        system(sconcat(7, "iptables -D PREROUTING -t mangle -s ", bb_request->ipPhone1, " -d ", bb_request->ipPhone2, " -p tcp --dport ", buffer, " -j MARK --set-mark 1"));
            //POST DSCP
        printf("%s\n", sconcat(7, "iptables -D POSTROUTING -t mangle -s ", bb_request->ipPhone1, " -d ", bb_request->ipPhone2, " -p tcp --dport ", buffer, " -j DSCP --set-dscp class EF"));
        system(sconcat(7, "iptables -D POSTROUTING -t mangle -s ", bb_request->ipPhone1, " -d ", bb_request->ipPhone2, " -p tcp --dport ", buffer, " -j DSCP --set-dscp class EF"));

        printf("Rule deleted for %s | %s | %d \n", bb_request->ipPhone1, bb_request->ipPhone2, bb_request->portPhone2);
        system("iptables -L -t mangle");
    }
}

void router_clear_rules(){
    printf("Command: %s \n", sconcat(3, "tc qdisc del dev ",DEFAULT_INTERFACE," root\n"));
    system(sconcat(3, "tc qdisc del dev ", DEFAULT_INTERFACE, " root\n"));
    printf("Command: iptables -t mangle -F\n");
    system("iptables -t mangle -F");
}

/**
 * Define for signal handling
 * */

/**
 * Handling for the SIG INTERRUPTION (SIGINT)
*/
void sigint_handler(int sig){
    printf("\nClosing of sockets\n");
    shutdown(server_client_sock, SHUT_RDWR);
    close(server_client_sock);
    shutdown(server_sock, SHUT_RDWR);
    close(server_sock);

    printf("Cleanning up of the tc commands \n");
    router_clear_rules();
    printf("Rules cleanned up ! \n");
    exit(0);
}




void process_bb_request(BBrequest* bb_request){
    if (bb_request->type == 0) {
        int i=0;
        while(i < REQ_TABLE_LENGTH && !compare_request(req_table[i].bbrequest,bb_request)) {
                time_t t = time(NULL);
                struct tm * curr_tm = malloc(sizeof(struct tm));
                *curr_tm = *localtime(&t);
                free(req_table[i].req_time);
                req_table[i].req_time = curr_tm;
        }

        if (i < REQ_TABLE_LENGTH) {
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

                router_add_rule(req_table[j].bbrequest);

            }
        }

    } else if (bb_request->type == 1) {
        for (int i=0;i<REQ_TABLE_LENGTH;i++) {
             if (compare_request(req_table[i].bbrequest,bb_request)) {
                router_del_rule(bb_request);
                free(req_table[i].bbrequest);
                free(req_table[i].req_time);
                req_table[i].bbrequest = NULL;
                req_table[i].req_time = NULL;
             }
        }
    }
}

/*void handler_bb_request(int fd, short event, void *arg) {
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
}*/


int main(int argc, char **argv) {
    if(argc < 2){
        printf("router <port>\n");
        exit(EXIT_FAILURE);
    }



    printf("Initialization of components...\n");
    signal(SIGINT, sigint_handler);
    init_request_components();
    router_init_rules();

    
    /*
    struct event ev;
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    */

    // client 
    
    //init_client();
    //connect_server();




    // Server 
    init_server(atoi(argv[1]));

    unsigned int received_msg_size;
    size_t receiv_msg_size = 0;
    char msg[READ_MAX];
    BBrequest* ptr_bbrqst = NULL;

    /*
    event_init();
    evtimer_set(&ev, handler_bb_request, NULL);
    evtimer_add(&ev, &tv);
    event_dispatch();
    */
    while(1){
        printf("Waiting of new request...\n");
        if ((server_client_sock = accept(server_sock, (struct sockaddr *) ptr_server_client_addr, &received_msg_size)) == -1) {
            perror("Error server accept");
            exit(EXIT_FAILURE);
        }

        printf("Receiving of new request...\n");
        if ((receiv_msg_size = read(server_client_sock, msg, READ_MAX - 1)) < 0) {
            perror("server read error");
            //exit(EXIT_FAILURE);
        }
        else{
            printf("New request: %s\n", msg);
            printf("Parsing...\n");
            ptr_bbrqst = parsing(msg, receiv_msg_size);

            if(ptr_bbrqst != NULL){
                printf("Parsing OK\n");
                //print_bbrequest(ptr_bbrqst);
                process_bb_request(ptr_bbrqst);
                ackBB();
                free(ptr_bbrqst);
            }else{
                printf("Error when parsing of the request\n");
            }
        }

    }

    /*pthread_t tid;
    ArgTH argServe;
    pthread_create(&tid, NULL, server_th, (void *)argServe); 

    pthread_exit(NULL); */

}



char* concat(char* str1, char* str2){
    if(concat_buf != str1)
        strncpy(concat_buf, str1, CONCAT_BUF_SIZE);

    strncat(concat_buf, str2, CONCAT_BUF_SIZE-strlen(concat_buf)-2);
    return concat_buf;
}

char* concat3(char* str1, char* str2, char* str3){
    return concat(concat(str1, str2), str3);
}


char* concat5(char* str1, char* str2, char* str3, char* s4, char* s5){
    return concat3(concat3(str1, str2, str3), s4, s5);
}

char* concat6(char* str1, char* str2, char* str3, char* s4, char* s5, char* s6){
    return concat(concat5(str1, str2, str2, s4, s5), s6);
}

char* concat8(char* str1, char* str2, char* str3, char* s4, char* s5, char* s6, char* s7, char* s8){
    return concat3(concat6(str1, str2, str2, s4, s5, s6), s7, s8);
}

char* concat7(char* str1, char* str2, char* str3, char* s4, char* s5, char* s6, char* s7){
    return concat(concat6(str1, str2, str2, s4, s5, s6), s7);
}

char* concat9(char* str1, char* str2, char* str3, char* s4, char* s5, char* s6, char* s7, char* s8, char* s9){
    return concat(concat8(str1, str2, str2, s4, s5, s6, s7, s8), s9);
}

//char* concat
char* sconcat(int num, ...){
    va_list argp;
    char* s;
    va_start(argp, num);
    strncpy(concat_buf, "", CONCAT_BUF_SIZE);
    for(int i=0; i<num; i++){
        s = va_arg(argp, char*);
        strncat(concat_buf, s, CONCAT_BUF_SIZE-strlen(concat_buf)-2);
    }
    printf("%s\n", concat_buf);
    return concat_buf;
}