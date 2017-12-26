//
// Created by ChiuPhonic on 2017/11/14.
//
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
#include <time.h>
#include <map>
#include "../public/Message.h"
#include "../public/Instrument.h"
using namespace std;
const int MAX_LINE = 2048;
const int PORT =20003;
const int BACKLOG = 10;
const int LISTENQ = 6666;
const int MAX_CONNECT = 20;

map<string ,stockType> stock_map;

// 同步锁
pthread_mutex_t stockMutex=PTHREAD_MUTEX_INITIALIZER;
//随机模拟股票价格变动
int price=0;
void *RandomStock(void *fd)
{
    stock_map["IBM.US"].num=2000;
    stock_map["MS.US"].num=1500;
    stock_map["PTR.US"].num=1200;
    stock_map["BABA.US"].num=3500;

    stock_map["IBM.US"].price=148900;
    stock_map["MS.US"].price=48370;
    stock_map["PTR.US"].price=68920;
    stock_map["BABA.US"].price=182570;
    srand(time(NULL));
    while(1)
    {
        pthread_mutex_lock(&stockMutex);
        stock_map["IBM.US"].price+=(rand()%500-250)*10;
        stock_map["MS.US"].price+=(rand()%200-100)*10;
        stock_map["PTR.US"].price+=(rand()%100-50)*10;
        stock_map["BABA.US"].price+=(rand()%1000-500)*10;
        pthread_mutex_unlock (&stockMutex);
        sleep(1);
    }
}

/*处理接收服务器消息函数*/
void *recv_message(void *fd)
{
    mktDataType cmsg;
    int sockfd = *(int *)fd;
    while(1)
    {

        clMsgType smsrecv;
        int n;
        if((n = recv(sockfd , &smsrecv ,sizeof(clMsgType) , 0)) == -1)
        {
            perror("recv error.\n");
            exit(1);
        }//if

        //// ---
        pthread_mutex_lock(&stockMutex);

        strcpy(cmsg.clientName,"NYSE");
        strcpy(cmsg.stockName,smsrecv.stockName);
        cmsg.operation = smsrecv.operation;
        for (int i = 0; i < 20; ++i) {
            int tmp = (i == 0) ? stock_map[cmsg.stockName].price : cmsg.price[i-1];
            cmsg.price[i] = tmp + (rand()%20)*10;
            cmsg.num[i] = (20-abs(19-2*i)+rand()%3)*100;
        }

        pthread_mutex_unlock (&stockMutex);
        if((n = send(sockfd , &cmsg ,sizeof(mktDataType) , 0)) == -1)
        {
            perror("recv error.\n");
            exit(1);
        }//if

        //// ---
        pthread_mutex_lock(&stockMutex);

        strcpy(cmsg.clientName,"NASDAQ");
        strcpy(cmsg.stockName,smsrecv.stockName);
        cmsg.operation = smsrecv.operation;
        for (int i = 0; i < 20; ++i) {
            int tmp = (i == 0) ? stock_map[cmsg.stockName].price : cmsg.price[i-1];
            cmsg.price[i] = tmp + (rand()%20)*10;
            cmsg.num[i] = (20-abs(19-2*i)+rand()%3)*100;
        }

        pthread_mutex_unlock (&stockMutex);
        if((n = send(sockfd , &cmsg ,sizeof(mktDataType) , 0)) == -1)
        {
            perror("recv error.\n");
            exit(1);
        }//if

        //// ---
        pthread_mutex_lock(&stockMutex);

        strcpy(cmsg.clientName,"IEX");
        strcpy(cmsg.stockName,smsrecv.stockName);
        cmsg.operation = smsrecv.operation;
        for (int i = 0; i < 20; ++i) {
            int tmp = (i == 0) ? stock_map[cmsg.stockName].price : cmsg.price[i-1];
            cmsg.price[i] = tmp + (rand()%20)*10;
            cmsg.num[i] = (20-abs(19-2*i)+rand()%3)*100;
        }

        pthread_mutex_unlock (&stockMutex);
        if((n = send(sockfd , &cmsg ,sizeof(mktDataType) , 0)) == -1)
        {
            perror("recv error.\n");
            exit(1);
        }//if
    }//while
}


int mainn(int argc , char **argv)
{
    /*声明套接字和链接服务器地址*/
    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    int sockfd;
    pthread_t recv_tid , send_tid , randstock_tid;
    struct sockaddr_in servaddr;
    /*判断是否为合法输入*/
    if(argc != 2)
    {
        perror("usage:tcpcli <IPaddress>");
        exit(1);
    }//if

    /*(1) 创建套接字*/
    if((sockfd = socket(AF_INET , SOCK_STREAM , 0)) == -1)
    {
        perror("socket error");
        exit(1);
    }//if

    /*(2) 设置链接服务器地址结构*/
    bzero(&servaddr , sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    if(inet_pton(AF_INET , argv[1] , &servaddr.sin_addr) < 0)
    {
        printf("inet_pton error for %s\n",argv[1]);
        exit(1);
    }//if

    /*发送链接服务器请求*/
    if( connect(sockfd , (struct sockaddr *)&servaddr , sizeof(servaddr)) < 0)
    {
        perror("connect error");
        exit(1);
    }

    /*创建子线程处理该客户链接接收消息*/
    if(pthread_create(&recv_tid , &threadAttr, recv_message, &sockfd) == -1)
    {
        perror("recv_message_pthread create error.\n");
        exit(1);
    }
    /*创建子线程模拟股票价格变动*/
    if(pthread_create(&randstock_tid , &threadAttr ,RandomStock, NULL) == -1)
    {
        perror("RandomStock_pthread create error.\n");
        exit(1);
    }

    sleep(360000);
//    mktDataType csmgtest_1;
//
//    while(fgets( csmgtest_1.stockName , MAX_LINE , stdin) != NULL)
//    {
//        strcpy(csmgtest_1.clientName , "client1");
//        if(strcmp(csmgtest_1.clientName, "exit\n") == 0)
//        {
//            printf("byebye.\n");
//            memset(csmgtest_1.stockName , 0 , MAX_LINE);
//            strcpy(csmgtest_1.stockName, "byebye.");
//            send(sockfd , &csmgtest_1 , sizeof(mktDataType), 0);
//            close(sockfd);
//            exit(0);
//        }//if
//
//        pthread_mutex_lock(&stockMutex);
//        printf("\n the price is %d now \n",csmgtest_1.price);
//        csmgtest_1.price=price;
//        pthread_mutex_unlock (&stockMutex);
//
//        if(send(sockfd ,&csmgtest_1 , sizeof(mktDataType) , 0) == -1)
//        {
//            perror("send error.\n");
//            exit(1);
//        }//
//
//    }//while
}

