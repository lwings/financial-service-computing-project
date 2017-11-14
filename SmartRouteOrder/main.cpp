#include <iostream>
#include <stdio.h>
#include <string.h>
#include "Order.h"
#include "OrderBook.h"

#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <vector>
#include "../public/Message.h"

using namespace std;
const int MAX_LINE = 2048;
const int PORT_1 = 20003;
const int PORT_2 = 20004;
const int PORT_3 = 20005;
const int BACKLOG = 10;
const int LISTENQ = 6666;
const int MAX_CONNECT = 20;


/*处理接收客户端消息函数*/
void *recv_message(void *fd)
{
    int sockfd = *(int *)fd;
    mktDataType cmsgtest1;
    while(1)
    {
        int n;
        if((n = recv(sockfd , &cmsgtest1 ,sizeof( mktDataType) , 0)) == -1)
        {
            perror("recv error.\n");
            exit(1);
        }//if

        printf("\n1234\n");
        printf("\n %s stock_name %s price  %d  num%d", cmsgtest1.clientName, cmsgtest1.stockName,cmsgtest1.price,cmsgtest1.num);
    }//while
}

void fix_orders(Order* o, int const level, int const qty, int const curQty)
{
    if(curQty>=qty)
    {
        return;
    }
    double beta = double(qty)/curQty;
    int cum = 0;
    int beta_cum = 0;
    for(int i = 0;i<level;++i)
    {
        int s = (o+i)->qty;
        cum +=s;
        (o+i)->qty = int(cum*beta)-beta_cum;
        beta_cum+=(o+i)->qty;
    }
}

void ShowSmartOrder(Order* o, int const level)
{
    printf("%-6s%-10s%-10s%-10s%-10s\n","SIDE","TICKER","QUANTITY","PRICE","EXCHANGE");
    for(int i = 0;i<level;++i)
    {
        if((o+i)->exch=="NYSE")
        {
            printf("%-6c%-10s%-10d%-10.2f%-10s\n",(o+i)->side,(o+i)->ticker.c_str(),(o+i)->qty,double((o+i)->priceInt)/1000, (o+i)->exch.c_str());
        }
    }
    for(int i = 0;i<level;++i)
    {
        if((o+i)->exch=="NASDAQ")
        {
            printf("%-6c%-10s%-10d%-10.2f%-10s\n",(o+i)->side,(o+i)->ticker.c_str(),(o+i)->qty,double((o+i)->priceInt)/1000, (o+i)->exch.c_str());
        }
    }
    for(int i = 0;i<level;++i)
    {
        if((o+i)->exch=="IEX")
        {
            printf("%-6c%-10s%-10d%-10.2f%-10s\n",(o+i)->side,(o+i)->ticker.c_str(),(o+i)->qty,double((o+i)->priceInt)/1000, (o+i)->exch.c_str());
        }
    }
}

Order* find_orders(OrderBook const& ob, Order const& order, int& level)
{
    char side = order.side;
    int px = order.priceInt;
    int size = order.qty;
    int cumQty = 0;
    Order* ret;

    if(side=='B'){
        int bid_size = ob.bidlevel();
        ret = new Order[bid_size];
        for(int i = 0;i<bid_size;++i)
        {
            if(cumQty>=size || px<ob.bid[i].priceInt)
                break;
            if(cumQty + ob.bid[i].qty>size)
            {
                ret[i] = Order('b', size-cumQty,ob.bid[i].priceInt, ob.bid[i].ticker,ob.bid[i].exch);
                cumQty = size;
                level ++;
            }
            else if (cumQty + ob.bid[i].qty<=size)
            {
                ret[i] = Order('b', ob.bid[i].qty,ob.bid[i].priceInt, ob.bid[i].ticker,ob.bid[i].exch);
                cumQty += ob.bid[i].qty;
                level ++;
            }
        }
    }
    else{
        int ask_size = ob.asklevel();
        ret = new Order[ask_size];
        for(int i = ask_size-1;i>=0;--i)
        {
            if(cumQty>=size || px>ob.ask[i].priceInt)
                break;
            if(cumQty + ob.ask[i].qty>size)
            {
                ret[ask_size-1-i] = Order('s', size-cumQty,ob.ask[i].priceInt, ob.ask[i].ticker,ob.ask[i].exch);
                cumQty = size;
                level ++;
            }
            else if (cumQty + ob.ask[i].qty<=size)
            {
                ret[ask_size-1-i] = Order('s', ob.ask[i].qty,ob.ask[i].priceInt, ob.ask[i].ticker,ob.ask[i].exch);
                cumQty += ob.ask[i].qty;
                level ++;
            }
        }
    }
    fix_orders(ret, level, size, cumQty);
    return ret;
}

int main() {

if (true) {
    //声明套接字
    int listenfd_1, connfd_1, listenfd_2, connfd_2, listenfd_3, connfd_3;
    socklen_t clilen;
    //声明线程ID
    pthread_t recv_tid_1, recv_tid_2, recv_tid_3, send_tid;

    //定义地址结构
    struct sockaddr_in servaddr_1, servaddr_2, servaddr_3, cliaddr;

    /*(1) 创建套接字*/
    if ((listenfd_1 = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket error.\n");
        exit(1);
    }//if
    if ((listenfd_2 = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket error.\n");
        exit(1);
    }//if
    if ((listenfd_3 = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket error.\n");
        exit(1);
    }//if
    /*(2) 初始化地址结构*/
    bzero(&servaddr_1, sizeof(servaddr_1));
    servaddr_1.sin_family = AF_INET;
    servaddr_1.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr_1.sin_port = htons(PORT_1);
    bzero(&servaddr_2, sizeof(servaddr_2));
    servaddr_2.sin_family = AF_INET;
    servaddr_2.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr_2.sin_port = htons(PORT_2);
    bzero(&servaddr_3, sizeof(servaddr_3));
    servaddr_3.sin_family = AF_INET;
    servaddr_3.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr_3.sin_port = htons(PORT_3);
    /*(3) 绑定套接字和端口*/
    if (bind(listenfd_1, (struct sockaddr *) &servaddr_1, sizeof(servaddr_1)) < 0) {
        perror("bind error.\n");
        exit(1);
    }//if

    /*(4) 监听*/
    if (listen(listenfd_1, LISTENQ) < 0) {
        perror("listen error.\n");
        exit(1);
    }//if

    if (bind(listenfd_2, (struct sockaddr *) &servaddr_2, sizeof(servaddr_2)) < 0) {
        perror("bind error.\n");
        exit(1);
    }//if

    if (listen(listenfd_2, LISTENQ) < 0) {
        perror("listen error.\n");
        exit(1);
    }//if

    if (bind(listenfd_3, (struct sockaddr *) &servaddr_3, sizeof(servaddr_3)) < 0) {
        perror("bind error.\n");
        exit(1);
    }//if

    if (listen(listenfd_3, LISTENQ) < 0) {
        perror("listen error.\n");
        exit(1);
    }//if
//client1
    clilen = sizeof(cliaddr);
    if ((connfd_1 = accept(listenfd_1, (struct sockaddr *) &cliaddr, &clilen)) < 0) {
        perror("accept error.\n");
        exit(1);
    }//if
    printf("server: got connection from %s\n", inet_ntoa(cliaddr.sin_addr));

    /*创建子线程处理该客户链接接收消息*/
    if (pthread_create(&recv_tid_1, NULL, recv_message, &connfd_1) == -1) {
        perror("pthread create error.\n");
        exit(1);
    }//if
//client2
    clilen = sizeof(cliaddr);
    if ((connfd_2 = accept(listenfd_2, (struct sockaddr *) &cliaddr, &clilen)) < 0) {
        perror("accept error.\n");
        exit(1);
    }//if
    printf("server: got connection from %s\n", inet_ntoa(cliaddr.sin_addr));

    /*创建子线程处理该客户链接接收消息*/
    if (pthread_create(&recv_tid_2, NULL, recv_message, &connfd_2) == -1) {
        perror("pthread create error.\n");
        exit(1);
    }//if
//client3
    clilen = sizeof(cliaddr);
    if ((connfd_3 = accept(listenfd_3, (struct sockaddr *) &cliaddr, &clilen)) < 0) {
        perror("accept error.\n");
        exit(1);
    }//if
    printf("server: got connection from %s\n", inet_ntoa(cliaddr.sin_addr));

    /*创建子线程处理该客户链接接收消息*/
    if (pthread_create(&recv_tid_3, NULL, recv_message, &connfd_3) == -1) {
        perror("pthread create error.\n");
        exit(1);
    }//if
}

    std::string command;
    std::string AllExch[3];
    AllExch[0] = "NYSE";
    AllExch[1] = "NASDAQ";
    AllExch[2] = "IEX";

    while(true)
    {
        char side;
        int qty;
        double price;
        std::string ticker;
        std::cin>>side;
        std::cin>>ticker;
        std::cin>>qty;
        std::cin>>price;
        std::string exch = "NONE";
        Order* o = new Order(side, qty, int(1000*price), ticker, exch);
        OrderBook* nyse = get_orderbook_by_exch(ticker, AllExch[0]);
        OrderBook* nq = get_orderbook_by_exch(ticker, AllExch[1]);
        OrderBook* iex = get_orderbook_by_exch(ticker, AllExch[2]);
        OrderBook* orderbook = intergrate_orderbooks(*nyse, *nq, *iex);

        nq->showall();

        int level;
        Order* result = find_orders(*orderbook, *o, level);
        ShowSmartOrder(result, level);

    }
    return 0;
}
