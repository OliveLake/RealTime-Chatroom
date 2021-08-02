#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "server.h"
#define LENGTH_NAME 31
#define LENGTH_MSG 101
#define LENGTH_SEND 201

//首先跟client一樣先定義要用到的全域變數 
int server_sockfd = 0, client_sockfd = 0;
ClientList *root, *now;

void clientController(void *p_client);
void breakPoint(int sig);
void sendAll(ClientList *np, char tmp_buffer[]);


int main()
{
    signal(SIGINT, breakPoint);

    //建立一個socket
    server_sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (server_sockfd == -1) {
        printf("Fail to create a socket.");
        exit(EXIT_FAILURE);
    }

   // 建立Socket所用到的資訊
    struct sockaddr_in server_info, client_info;
    int s_addrlen = sizeof(server_info);
    int c_addrlen = sizeof(client_info);
    memset(&server_info, 0, s_addrlen);
    memset(&client_info, 0, c_addrlen);
    server_info.sin_family = PF_INET;
    server_info.sin_addr.s_addr = INADDR_ANY;
    server_info.sin_port = htons(8888);

    // Bind and Listen
    bind(server_sockfd, (struct sockaddr *)&server_info, s_addrlen);
    listen(server_sockfd, 5);

     //這裡是用來將Server的IP輸出
    getsockname(server_sockfd, (struct sockaddr*) &server_info, (socklen_t*) &s_addrlen);
    printf("Start! Server is on %s:%d\n", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));

      //初始化linklist讓client運用
    root = newNode(server_sockfd, inet_ntoa(server_info.sin_addr));
    now = root;

    while (1) {
        client_sockfd = accept(server_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);

        //這裡是用來將Client的IP輸出
        getpeername(client_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
        printf("Client %s:%d come in room.\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

        //將新的client insert到 client 的linklist中
        ClientList *c = newNode(client_sockfd, inet_ntoa(client_info.sin_addr));
        c->prev = now;
        now->link = c;
        now = c;

        pthread_t id;
        if (pthread_create(&id, NULL, (void *)clientController, (void *)c) != 0) {
            perror("Create pthread error!\n");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
void clientController(void *p_client) {
     //初始化一些會用到的資料
    int leave_flag = 0;
    char nickname[LENGTH_NAME] = {};
    char recv_buffer[LENGTH_MSG] = {};
    char send_buffer[LENGTH_SEND] = {};
    ClientList *np = (ClientList *)p_client;

    //把client的名字傳送到時，如果名字長度等小於2或大於30則不執行,也就是可以解釋成沒有收到input
    //經socket接收的名字小於等於0也不執行,也就是可以解釋成沒有收到input
    //recv是用新的套接字来接收遠端主機傳來的數據，並把數據存到由参数 buf 指向的内存空間


    if (recv(np->data, nickname, LENGTH_NAME, 0) <= 0 || strlen(nickname) < 2 || strlen(nickname) >= LENGTH_NAME-1) {
        printf("%s didn't input name.\n", np->ip);
        leave_flag = 1;
    } else {
        strncpy(np->name, nickname, LENGTH_NAME);
        printf("%s(%s)(%d) join the chatroom.\n", np->name, np->ip, np->data);
        sprintf(send_buffer, "%s(%s) join the chatroom.", np->name, np->ip);
    }

    // Conversation
    while (1) {
        if (leave_flag) {
            break;
        }
        if (!strcmp(recv_buffer, "/nick")){   //connecting:client itself show socketfd
            sprintf(send_buffer, "new nickname :  ");
            send(np->data, send_buffer, LENGTH_SEND, 0);
            strncpy(np->name, nickname, LENGTH_NAME);
            sprintf(send_buffer, "%s is your new name.\n", np->name);
            send(np->data, send_buffer, LENGTH_SEND, 0);
        }
        if (!strcmp(recv_buffer, "/connecting")){   //connecting:client itself show socketfd
            sprintf(send_buffer, "Successfully connected. socketfd: %d", root->data);
            send(np->data, send_buffer, LENGTH_SEND, 0);
            
        }
        if (!strcmp(recv_buffer, "/help")){   //help:all client show help
            sprintf(send_buffer, "<< /help     Show help\r\n");
            send(np->data, send_buffer, LENGTH_SEND, 0);
            sprintf(send_buffer, "<< /exit     Quit chatroom\r\n");
            send(np->data, send_buffer, LENGTH_SEND, 0);
            sprintf(send_buffer, "<< /connecting     Server test\r\n");
            send(np->data, send_buffer, LENGTH_SEND, 0);
            
        }

        //如果client 有傳 message 則server會顯示出把這個message傳到其餘每個client socket
        //並且顯示出 IP 位置和 client 名稱
        //若有client離開聊天室則顯示出是哪個client離開聊天室




        int receive = recv(np->data, recv_buffer, LENGTH_MSG, 0);
        if (receive > 0) {
            if (strlen(recv_buffer) == 0) {
                continue;
            }
            sprintf(send_buffer, "%s：%s from %s", np->name, recv_buffer, np->ip);
        } else if (receive == 0 || strcmp(recv_buffer, "/exit") == 0) {
            printf("%s(%s)(%d) leaves the chatroom.\n", np->name, np->ip, np->data);
            sprintf(send_buffer, "%s(%s) leaves the chatroom.", np->name, np->ip);
            leave_flag = 1;
        } else {
            printf("Fatal Error: -1\n");
            leave_flag = 1;
        }
        sendAll(np, send_buffer);
    }

     //這個部分是在把 node 移除
    close(np->data);
    if (np == now) { // remove an edge node
        now = np->prev;
        now->link = NULL;
    } else { // remove a middle node
        np->prev->link = np->link;
        np->link->prev = np->prev;
    }
    free(np);
}
void breakPoint(int sig) {
    ClientList *tmp;
    while (root != NULL) {
        printf("\nClose socketfd: %d\t Bye!", root->data);
        close(root->data); // close all socket include server_sockfd
        tmp = root;
        root = root->link;
        free(tmp);
    }
    exit(EXIT_SUCCESS);
}

void sendAll(ClientList *np, char tmp_buffer[]) {
    ClientList *tmp = root->link;
    while (tmp != NULL) {
        if (np->data != tmp->data) { // all clients except itself.
            printf("Send to sockfd... %d: \"%s\" \n", tmp->data, tmp_buffer);
            send(tmp->data, tmp_buffer, LENGTH_SEND, 0);
        }
        tmp = tmp->link;
    }
}