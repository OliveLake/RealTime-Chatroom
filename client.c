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
#define LENGTH_NAME 31
#define LENGTH_MSG 101
#define LENGTH_SEND 201


void str_trim_lf (char* arr, int length) {
    int i;
    for (i = 0; i < length; i++) { // trim \n
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }

}

//首先先宣告我們所會用到到的變數
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char nickname[LENGTH_NAME] = {};

void Exit(int sig) {
    flag = 1;
}

//這個function我們用來接收messeage
void RecvMsgController() {
    char receiveMessage[LENGTH_SEND] = {};
    while (1) {
        int receive = recv(sockfd, receiveMessage, LENGTH_SEND, 0);
        if (receive > 0) {
            printf("\r%s\n", receiveMessage);
            printf("\r%s", "> ");
            fflush(stdout);
        } else if (receive == 0) {
            break;
        } 
    }
}
//這個func是用來傳送資料
void SendMsgController() {
    char message[LENGTH_MSG] = {};
    while (1) {
        printf("\r%s", "> ");
        fflush(stdout);
        while (fgets(message, LENGTH_MSG, stdin) != NULL) {
            str_trim_lf(message, LENGTH_MSG);
            if (strlen(message) == 0) {
                printf("\r%s", "> ");
                fflush(stdout);
            } else {
                break;
            }
        }
        send(sockfd, message, LENGTH_MSG, 0);
        if (strcmp(message, "/exit") == 0) {
            break;
        }

    }
    flag = 1;
}

int main()
{
    signal(SIGINT, Exit);

    //把輸入的name直接存入資料中
    printf("Please enter your name: ");
    if (fgets(nickname, LENGTH_NAME, stdin) != NULL) {
        str_trim_lf(nickname, LENGTH_NAME);
        //這個func是用來把一筆一筆的資料存到message array裡
    }
     //若輸入的message只有一個char或是message的char超過30個則exit
    if (strlen(nickname) < 2 || strlen(nickname) >= LENGTH_NAME-1) {
        printf("\nName must be more than one and less than thirty characters.\n");
        exit(EXIT_FAILURE);
    }

    //建立一個socket
    //Socket 就是一個網路上的通訊端點，使用者或應用程式只要連接到 Socket 
    //便可以和網路上任何一個通訊端點連線，Socket 之間通訊就如同作業系統內程序（Process）之間通訊一樣
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1) {
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
    server_info.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_info.sin_port = htons(8888);

    //和sever做連接
    //sockfd為前面 socket 的返回值，即 sfd server_info為struct指標變量，儲存遠程的server IP與socket訊息,s_addrlen為變量長度
    //並用0跟-1去判斷是否可執行
    int err = connect(sockfd, (struct sockaddr *)&server_info, s_addrlen);
    if (err == -1) {
        printf("Connection to Server error!\n");
        exit(EXIT_FAILURE);
    }
    
     //在client 跟 sever 中顯示
    getsockname(sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
    getpeername(sockfd, (struct sockaddr*) &server_info, (socklen_t*) &s_addrlen);
    printf("Connect to Server: %s:%d\n", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));
    printf("You are: %s:%d\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

    //用新的套接字發送數據给指定的遠端主機
    send(sockfd, nickname, LENGTH_NAME, 0);

    //建thread並呼叫前面所定義的func去semd message
    pthread_t send_msg_thread;
    if (pthread_create(&send_msg_thread, NULL, (void *) SendMsgController, NULL) != 0) {
        printf ("Create pthread error!\n");
        exit(EXIT_FAILURE);
    }

    //建thread並呼叫前面所定義的func去receive message
    pthread_t recv_msg_thread;
    if (pthread_create(&recv_msg_thread, NULL, (void *) RecvMsgController, NULL) != 0) {
        printf ("Create pthread error!\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if(flag) {
            printf("\nThank you\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}