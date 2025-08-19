#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 1024

pthread_mutex_t lock;
pthread_cond_t cond_var;
int sock;

int server_running = 1;
typedef struct queue{
    int sock;
    struct queue * next;
} queue;

queue * head = NULL;
void enqueue(int client_sock){

    queue * node = (queue *)malloc(sizeof(queue));
    node->sock = client_sock;
    node->next = NULL;
    pthread_mutex_lock(&lock);
    if(head == NULL){
        head = node;
    }
    else{
        queue * temp = head;
        while(temp->next != NULL){
            temp = temp->next;
        }
        temp->next=node;
    }
    pthread_cond_signal(&cond_var);
    pthread_mutex_unlock(&lock);
}

int dequeue(){
    pthread_mutex_lock(&lock);
    if(head == NULL)    {
        pthread_cond_wait(&cond_var, &lock);
    }
    if(server_running != 1){
        pthread_mutex_unlock(&lock);
        return -1;  
    }
    queue * node = head;
    head = head->next;
    pthread_mutex_unlock(&lock);
    int socket = node->sock;
    free(node);
    return socket;
}
void * fun(){
    while(1){
        if(server_running != 1){
            break;
        }

        char *response = "World";
        char buffer[BUF_SIZE] = {0};
        int client_sock = dequeue();
        if(client_sock == -1){
            break;
        }
        int n;
        if((n = read(client_sock, buffer, BUF_SIZE)) < 0 ){
            printf("Read failed \n");
            exit(-1);
        }
        buffer[n] = '\0';
        printf("Message received: %s\n", buffer);
        if(strcmp(buffer, "hello") == 0){
            if(send(client_sock, response, strlen(response), 0) < 0){
                printf("Sent failed\n");
                exit(-1);
            }
            printf("Message sent !!\n");
        }
        else{
            printf("hello not received !!\n");
        }
        close(client_sock);
    }
    return NULL;

}
void handle_sigint(int sig){
    printf("\nCaught signal %d (SIGINT). Shutting down server ...\n", sig);
    pthread_mutex_lock(&lock);
    server_running = 0;
    pthread_mutex_unlock(&lock);
    pthread_cond_broadcast(&cond_var);
    close(sock);
}

int main(int argc, char *argv[])
{
    if(argc != 3){
        printf("Please pass the port number and thread pool size in the command line argument");
        return -1;
    }
    int port = atoi(argv[1]);
    int pool_size = atoi(argv[2]);
    int new_sock;
    struct sockaddr_in serv_addr;
    int addr_len = sizeof(serv_addr);
    int opt = 1;
    
    pthread_t th[pool_size];
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Socket creation failed !!\n");
        return -1;
    }

    // Set socket options to reuse address and port
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if(bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        printf("Bind failed\n");
        close(sock);
        return -1;

    }

    if(listen(sock, 4096) < 0){
        printf("Listen failed\n");
        close(sock);
        return -1;
    }
    signal(SIGINT, handle_sigint);
    for(int i=0; i<pool_size; i++){
        if(pthread_create(th+i, NULL, fun, NULL)!=0){
            printf("Thread creation failed..\n");
            close(sock);
            exit(-1);
        }
    }


    for(;;){
        if(server_running != 1){
            break;
        }
        if((new_sock = accept(sock, (struct sockaddr*)&serv_addr, (socklen_t *) &addr_len)) < 0 ){
            if(server_running != 1){
                break;
            }
            printf("Accept failed\n");
            return -1;
        }
        enqueue(new_sock);
        
    }  
    for(int i=0; i<pool_size; i++){
        pthread_join(th[i], NULL);
    }  
    close(sock);
    while(head != NULL){
        queue * node = head;
        head = head->next;
        free(node);
    }

    return 0;
}