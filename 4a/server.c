#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUF_SIZE 1024
#define PORT 8080

int main(int argc, char *argv[])
{

    int sock, new_sock;
    struct sockaddr_in serv_addr;
    int addr_len = sizeof(serv_addr);
    char *message = "World";
    char buffer[BUF_SIZE] = {0};

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
    for(;;){
        if((new_sock = accept(sock, (struct sockaddr *) &serv_addr, (socklen_t *)&addr_len)) < 0){
            printf("Accept failed\n");
            return -1;
        }
        printf("Entered loop: %d \n", new_sock);
        if(read(new_sock, buffer, BUF_SIZE) < 0){
            printf("Read failed\n");
            return -1;
        }
        printf("Read done\n");
        // printf("Read message: %s\n", buffer);
        if(strcmp(buffer, "hello") == 0){
            if(send(new_sock, message, strlen(message),0) < 0){
                printf("Sent failed\n");
                return -1;
            }
            // printf("Sent message: %s\n", message);
        }
        else{
            printf("Hello not received...\n");
        }
        close(new_sock);

    }
    

    return 0;
}