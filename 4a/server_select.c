#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

#define BUF_SIZE 1024
#define PORT 8081
#define MAX_CLIENTS 1024

int main(int argc, char *argv[])
{

    int sock, new_sock, client_sockets[MAX_CLIENTS], maxfd, fd, activity;
    struct sockaddr_in serv_addr;
    int addr_len = sizeof(serv_addr);
    char *message = "World";
    char buffer[BUF_SIZE] = {0};
    int n;
    fd_set readfds;
    for(int i = 0; i<MAX_CLIENTS; i++){
        client_sockets[i] = 0;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket creation error\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Bind failed\n");
        return -1;
    }
    
    if(listen(sock, 10)< 0){
        printf("Listed failed\n");
        return -1;
    }

    for(;;){
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        maxfd = sock;
        for(int i=0; i<MAX_CLIENTS; i++){
            fd = client_sockets[i];
            if(fd > 0){
                FD_SET(fd, &readfds);
            }
            if(fd>maxfd)
                maxfd= fd;
        }

        activity = select(maxfd+1, &readfds, NULL, NULL, NULL);
        if(activity < 0){
            printf("Select Error\n");
            return -1;
        }
        if(FD_ISSET(sock, &readfds)){
            if((new_sock = accept(sock, (struct sockaddr *)&serv_addr, (socklen_t *) &addr_len)) < 0){
                printf("Accept error");
                return -1;
            }
            printf("New connection, socket fd: %d, ip: %s, port: %d\n",
                   new_sock, inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));
            for(int i=0; i<MAX_CLIENTS; i++){
                if(client_sockets[i] == 0){
                    client_sockets[i] = new_sock;
                    break;
                }
            }
        }
        for(int i=0; i<MAX_CLIENTS; i++){
            fd = client_sockets[i];
            if(FD_ISSET(fd, &readfds)){
                if((n = read(fd, buffer, BUF_SIZE)) == 0){
                    getpeername(fd, (struct sockaddr *)&serv_addr, (socklen_t *)&addr_len);
                    printf("Host disconnected, ip: %s, port: %d\n",
                           inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));
                    close(fd);
                    client_sockets[i] = 0;
                }

                else{
                    // printf("Message received: %s\n", buffer);
                    if(strcmp(buffer, "hello") == 0){
                        send(fd, message, strlen(message), 0);
                        // printf("Message sent: %s\n", message);
                    }
                    else{
                        printf("hello not recieved\n");
                    }
                }
            }
        }

    }
    

    return 0;
}