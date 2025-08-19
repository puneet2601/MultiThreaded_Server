#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/epoll.h>

#define BUF_SIZE 1024
#define PORT 8082
#define MAX_EVENTS 10

int main(int argc, char *argv[])
{

    int sock, new_sock, epoll_fd, event_count, fd, activity;
    struct sockaddr_in serv_addr;
    int addr_len = sizeof(serv_addr);
    char *message = "World";
    char buffer[BUF_SIZE] = {0};
    int n;
    struct epoll_event event, events[MAX_EVENTS];

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
    
    if(listen(sock, 5)< 0){
        printf("Listed failed\n");
        return -1;
    }
    if((epoll_fd = epoll_create1(0)) == -1){
        printf("Epoll creation failed\n");
        return -1;
    }

    event.events = EPOLLIN;
    event.data.fd = sock;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &event) == -1){
        printf("Epoll ctl failed\n");
        return -1;
    }
    for(;;){
        event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for(int i=0; i<event_count; i++){
            if(events[i].data.fd == sock){
                if((new_sock = accept(sock, (struct sockaddr *) &serv_addr, (socklen_t *) &addr_len)) < 0){
                    printf("Accept error\n");
                    return -1;
                }
                printf("New connection accepted, socket fd: %d\n", new_sock);
                event.events = EPOLLIN;
                event.data.fd = new_sock;
                if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_sock, &event) == -1){
                    printf("Epoll ctl failed\n");
                    return -1;
                }

            }
            else{
                int fd = events[i].data.fd;
                n = read(fd, buffer, BUF_SIZE);
                if(n == 0){
                    printf("Client disconnected, socket fd: %d\n", fd);
                    close(fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                }
                else{
                    // printf("Received message: %s\n", buffer);
                    if(strcmp(buffer, "hello") == 0){
                        send(fd, message, strlen(message), 0);
                        // printf("Sent message: %s\n", message);
                    }
                }
            }
        }

    }
    
    close(sock);
    close(epoll_fd);
    return 0;
}