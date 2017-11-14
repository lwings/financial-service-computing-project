//
// Created by ChiuPhonic on 2017/11/15.
//

#include "ExchImpl.h"

void *ExchImpl::RandomStock(void *fd)
{
    /*随机模拟股票数据变动 数量&价格*/
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
void *ExchImpl::recv_message(void *fd)
{
    mktDataType mkt_data1;
    int sockfd = *(int *)fd;
    while(1)
    {
        clMsgType client_msg1;
        if(recv(sockfd , &client_msg1 ,sizeof(clMsgType) , 0) == -1)
        {
            perror("recv error.\n");
            exit(1);
        }
        pthread_mutex_lock(&stockMutex);
        if (client_msg1.operation==0)
        {
            strcpy(mkt_data1.clientName,"client1");
            strcpy(mkt_data1.stockName,client_msg1.stockName);
            mkt_data1.price=stock_map[mkt_data1.stockName].price;
            mkt_data1.num=stock_map[mkt_data1.stockName].num;
            mkt_data1.operation=0;
        }
        if(client_msg1.operation==1)
        {
            strcpy(mkt_data1.clientName,"client1");
            strcpy(mkt_data1.stockName,client_msg1.stockName);
            stock_map[mkt_data1.stockName].num= stock_map[mkt_data1.stockName].num-client_msg1.num;
            mkt_data1.price=stock_map[mkt_data1.stockName].price;
            mkt_data1.num=stock_map[mkt_data1.stockName].num;
            mkt_data1.operation=1;
        }
        pthread_mutex_unlock (&stockMutex);
        if((n = send(sockfd , &mkt_data1 , sizeof(mktDataType) , 0)) == -1)
        {
            perror("recv error.\n");
            exit(1);
        }
    }
}

void ExchImpl::create_socket() {
    /*创建套接字*/
    if((sockfd = socket(AF_INET , SOCK_STREAM , 0)) == -1)
    {
        perror("socket error");
        exit(1);
    }
}

void ExchImpl::tcp_connect() {
    /*设置链接服务器地址结构*/
    bzero(&servaddr , sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    if(inet_pton(AF_INET , IP.c_str() , &servaddr.sin_addr) < 0)
    {
        printf("inet_pton error for %s\n", IP.c_str());
        exit(1);
    }
    /*发送链接服务器请求*/
    if( connect(sockfd , (struct sockaddr *)&servaddr , sizeof(servaddr)) < 0)
    {
        perror("connect error");
        exit(1);
    }
}

void ExchImpl::run() {
    /*创建子线程处理该客户链接接收消息*/
    if(pthread_create(&recv_tid , &threadAttr, recv_message , &sockfd) == -1)
    {
        perror("recv_message_pthread create error.\n");
        exit(1);
    }
    /*创建子线程模拟股票价格变动*/
    if(pthread_create(&randstock_tid , &threadAttr , RandomStock , NULL) == -1)
    {
        perror("RandomStock_pthread create error.\n");
        exit(1);
    }
    mktDataType mkt_data1;
    while(fgets( mkt_data1.stockName , MAX_LINE , stdin) != NULL)
    {
        strcpy(mkt_data1.clientName , "Client_1 ");
        if(strcmp(mkt_data1.clientName , "exit\n") == 0)
        {
            printf("byebye.\n");
            memset(mkt_data1.stockName , 0 , MAX_LINE);
            strcpy(mkt_data1.stockName , "byebye.");
            send(sockfd , &mkt_data1 , sizeof(mktDataType) , 0);
            close(sockfd);
            exit(0);
        }
        pthread_mutex_lock(&stockMutex);
        printf("\n the price is %d now \n", mkt_data1.price);
        mkt_data1.price = mkt_data1.price;
        pthread_mutex_unlock (&stockMutex);
        if(send(sockfd , &mkt_data1 , sizeof(mktDataType) , 0) == -1)
        {
            perror("send error.\n");
            exit(1);
        }

    }
}