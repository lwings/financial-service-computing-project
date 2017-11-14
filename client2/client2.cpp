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
using namespace std;
const int MAX_LINE = 2048;
const int PORT =20003;
const int BACKLOG = 10;
const int LISTENQ = 6666;
const int MAX_CONNECT = 20;

typedef struct stock{
    int num;
    int price;
} stockType;

map<string ,stockType> stock_map;

typedef struct clientMessage{
    int operation;
    char clientName[1024] ;
    char stockName[1024] ;
    int price;
    int num;
} cmsgType;

typedef struct serverMessage{
    int operation;
    char stockName[1024] ;
    int  num;
} smsgType;


// 同步锁
pthread_mutex_t stockMutex=PTHREAD_MUTEX_INITIALIZER;
//随机模拟股票价格变动
int price=0;
void *RandomStock(void *fd)
{
stock_map["aaa"].num=500;
stock_map["bbb"].num=500;
stock_map["ccc"].num=500;
stock_map["ddd"].num=500;
    stock_map["aaa"].price=50;
    stock_map["bbb"].price=50;
    stock_map["ccc"].price=50;
    stock_map["ddd"].price=50;
    srand(time(NULL));
    while(1)
    {
        pthread_mutex_lock(&stockMutex);
        stock_map["aaa"].price=rand()%100;
        stock_map["bbb"].price=rand()%100;
        stock_map["ccc"].price=rand()%100;
        stock_map["ddd"].price=rand()%100;
        pthread_mutex_unlock (&stockMutex);
        sleep(1);
    }
}

/*处理接收服务器消息函数*/
void *recv_message(void *fd)
{
    cmsgType cmsg;
    int sockfd = *(int *)fd;
    while(1)
    {

        smsgType smsrecv;
        int n;
        if((n = recv(sockfd , &smsrecv ,sizeof(smsgType) , 0)) == -1)
        {
            perror("recv error.\n");
            exit(1);
        }//if
        pthread_mutex_lock(&stockMutex);
       if (smsrecv.operation==0)
       {
        strcpy(cmsg.clientName,"client2");
        strcpy(cmsg.stockName,smsrecv.stockName);
        cmsg.price=stock_map[cmsg.stockName].price;
         cmsg.num=stock_map[cmsg.stockName].num;
         cmsg.operation=0;
       }
       if(smsrecv.operation==1)
       {
        strcpy(cmsg.clientName,"client2");
        strcpy(cmsg.stockName,smsrecv.stockName);
        stock_map[cmsg.stockName].num= stock_map[cmsg.stockName].num-smsrecv.num;
        cmsg.price=stock_map[cmsg.stockName].price;
         cmsg.num=stock_map[cmsg.stockName].num;
         cmsg.operation=1;
       }
        pthread_mutex_unlock (&stockMutex);
        if((n = send(sockfd , &cmsg ,sizeof(cmsgType) , 0)) == -1)
        {
            perror("recv error.\n");
            exit(1);
        }//if
    }//while
}


int main(int argc , char **argv)
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
    cmsgType csmgtest_1;

    while(fgets( csmgtest_1.stockName , MAX_LINE , stdin) != NULL)
    {
        strcpy(csmgtest_1.clientName , "Client_2 ");
        if(strcmp(csmgtest_1.clientName, "exit\n") == 0)
        {
            printf("byebye.\n");
            memset(csmgtest_1.stockName , 0 , MAX_LINE);
            strcpy(csmgtest_1.stockName, "byebye.");
            send(sockfd , &csmgtest_1 , sizeof(cmsgType), 0);
            close(sockfd);
            exit(0);
        }//if

        pthread_mutex_lock(&stockMutex);
        printf("\n the price is %d now \n",price);
        csmgtest_1.price=price;
        pthread_mutex_unlock (&stockMutex);

        if(send(sockfd ,&csmgtest_1 , sizeof(cmsgType) , 0) == -1)
        {
            perror("send error.\n");
            exit(1);
        }//

    }//while
}
