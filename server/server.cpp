#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include<vector>
using namespace std;
const int MAX_LINE = 2048;
const int PORT_1 = 20003;
const int PORT_2 = 20004;
const int BACKLOG = 10;
const int LISTENQ = 6666;
const int MAX_CONNECT = 20;

typedef struct clientMessage{
    int operation;
    char clientName[1024] ;
    char stockName[1024] ;
    int price;
     int num;
} cmsgType;

typedef struct severMessage{
    int operation;
    char stockName[1024] ;
    int  num;
} smsgType;

/*处理接收客户端消息函数*/
void *recv_message(void *fd)
{
    int sockfd = *(int *)fd;
     cmsgType cmsgtest1;
    while(1)
    {

        int n;
        if((n = recv(sockfd , &cmsgtest1 ,sizeof( cmsgType) , 0)) == -1)
        {
            perror("recv error.\n");
            exit(1);
        }//if


//        printf("\n1234\n");
//        printf("\n %s stock_name %s price  %d  num%d", cmsgtest1.clientName, cmsgtest1.stockName,cmsgtest1.price,cmsgtest1.num);
    }//while
}

int main()
{

    //声明套接字
    int listenfd_1 , connfd_1,listenfd_2 , connfd_2;
    socklen_t clilen;
    //声明线程ID
    pthread_t recv_tid_1 , recv_tid_2 , send_tid;

    //定义地址结构
    struct sockaddr_in servaddr_1 ,servaddr_2 , cliaddr;

    /*(1) 创建套接字*/
    if((listenfd_1 = socket(AF_INET , SOCK_STREAM , 0)) == -1)
    {
        perror("socket error.\n");
        exit(1);
    }//if
    if((listenfd_2 = socket(AF_INET , SOCK_STREAM , 0)) == -1)
    {
        perror("socket error.\n");
        exit(1);
    }//if
    /*(2) 初始化地址结构*/
    bzero(&servaddr_1 , sizeof(servaddr_1));
    servaddr_1.sin_family = AF_INET;
    servaddr_1.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr_1.sin_port = htons(PORT_1);
    bzero(&servaddr_2 , sizeof(servaddr_2));
    servaddr_2.sin_family = AF_INET;
    servaddr_2.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr_2.sin_port = htons(PORT_2);
    /*(3) 绑定套接字和端口*/
    if(bind(listenfd_1 , (struct sockaddr *)&servaddr_1 , sizeof(servaddr_1)) < 0)
    {
        perror("bind error.\n");
        exit(1);
    }//if

    /*(4) 监听*/
    if(listen(listenfd_1 , LISTENQ) < 0)
    {
        perror("listen error.\n");
        exit(1);
    }//if

    if(bind(listenfd_2 , (struct sockaddr *)&servaddr_2 , sizeof(servaddr_2)) < 0)
    {
        perror("bind error.\n");
        exit(1);
    }//if

    if(listen(listenfd_2 , LISTENQ) < 0)
    {
        perror("listen error.\n");
        exit(1);
    }//if
//client1

    clilen = sizeof(cliaddr);
    if((connfd_1 = accept(listenfd_1 , (struct sockaddr *)&cliaddr , &clilen)) < 0)
    {
        perror("accept error.\n");
        exit(1);
    }//if
    printf("server: got connection from %s\n", inet_ntoa(cliaddr.sin_addr));

    /*创建子线程处理该客户链接接收消息*/
    if(pthread_create(&recv_tid_1 , NULL , recv_message, &connfd_1) == -1)
    {
        perror("pthread create error.\n");
        exit(1);
    }//if


//        client2

    clilen = sizeof(cliaddr);
    if((connfd_2 = accept(listenfd_2, (struct sockaddr *)&cliaddr , &clilen)) < 0)
    {
        perror("accept error.\n");
        exit(1);
    }//if
    printf("server: got connection from %s\n", inet_ntoa(cliaddr.sin_addr));

    /*创建子线程处理该客户链接接收消息*/
    if(pthread_create(&recv_tid_2 , NULL , recv_message, &connfd_2) == -1)
    {
        perror("pthread create error.\n");
        exit(1);
    }//if


    /*处理服务器发送消息*/
    smsgType smsgtest_1;
    while(fgets(smsgtest_1.stockName , MAX_LINE , stdin) != NULL)
    {

            smsgtest_1.operation=0;

//        if(strcmp(smsgtest_1.stockName , "exit\n") == 0)
//        {
//            printf("byebye.\n");
//            memset(smsgtest_1.stockName , 0 , MAX_LINE);
//            strcpy(smsgtest_1.stockName, "byebye.");
//            send(connfd_1 , &smsgtest_1, sizeof(smsgType) , 0);
//            send(connfd_2 ,&smsgtest_1, sizeof(smsgType) , 0);
//            close(connfd_1);
//            close(connfd_2);
//            exit(0);
//        }//if

        if(send(connfd_1 , &smsgtest_1,sizeof(smsgType) , 0) == -1)
        {
            perror("send error.\n");
            exit(1);
        }//if
        if(send(connfd_2 , &smsgtest_1,sizeof(smsgType), 0) == -1)
        {
            perror("send error.\n");
            exit(1);
        }//if
    }//while
}
