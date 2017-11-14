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
#include <map>
#include "../public/Message.h"

using namespace std;
const int MAX_LINE = 2048;
const int PORT_1 = 20003;
const int PORT_2 = 20004;
const int PORT_3 = 20010;
const int BACKLOG = 10;
const int LISTENQ = 6666;
const int MAX_CONNECT = 20;
const int EXCHANGE_COUNT = 3;
const int INSTRUMENT_COUNT = 4;
const int MAX_PRICELEVEL = 20;

typedef struct price_level {
    int price;
    int qty;
} pxlv;
typedef std::map<string, pxlv*> plm;
typedef std::map<string, plm> xpm;

std::string AllExch[EXCHANGE_COUNT] = {
        "NYSE",
        "NASDAQ",
        "IEX"
};
std::string AllTickers[INSTRUMENT_COUNT] = {
        "IBM.US",
        "MS.US",
        "PTR.US",
        "BABA.US"
};

xpm xp_map;

void init_xp_map()
{
    for (int i = 0; i < EXCHANGE_COUNT; ++i) {
        xp_map[AllExch[i]] = new plm();
        for (int j = 0; j < INSTRUMENT_COUNT; ++j) {
            xp_map[AllExch[i]][AllTickers[j]] = new pxlv(MAX_PRICELEVEL);
        }
    }
}

/*处理接收客户端消息函数*/
void *recv_message(void *fd)
{
    int sockfd = *(int *)fd;
    mktDataType cmsgtest1;
    while(1)
    {
        int n;
        if((n = recv(sockfd , &cmsgtest1 , sizeof( mktDataType) , 0)) == -1)
        {
            perror("recv error.\n");
            exit(1);
        }//if
        printf(" %s@%s \n BestBid: %d => %d \n BestOffer: %d => %d\n", cmsgtest1.stockName, cmsgtest1.clientName,
               cmsgtest1.price[9], cmsgtest1.num[9], cmsgtest1.price[10], cmsgtest1.num[10]);
        for (int i = 0; i < MAX_PRICELEVEL; ++i) {
            xp_map[cmsgtest1.clientName][cmsgtest1.stockName][i].price = cmsgtest1.price[i];
            xp_map[cmsgtest1.clientName][cmsgtest1.stockName][i].num = cmsgtest1.num[i];
        }
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

    int listenfd_1, connfd_1;
    socklen_t clilen;
    pthread_t recv_tid_1, send_tid;
    struct sockaddr_in servaddr_1, cliaddr;

    /*(1) 创建套接字*/
    if ((listenfd_1 = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket error.\n");
        exit(1);
    }//if

    /*(2) 初始化地址结构*/
    bzero(&servaddr_1, sizeof(servaddr_1));
    servaddr_1.sin_family = AF_INET;
    servaddr_1.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr_1.sin_port = htons(PORT_1);
    /*(3) 绑定套接字和端口*/
    /*(4) 监听*/
    if (bind(listenfd_1, (struct sockaddr *) &servaddr_1, sizeof(servaddr_1)) < 0) {
        perror("bind error.\n");
        exit(1);
    }//if
    if (listen(listenfd_1, LISTENQ) < 0) {
        perror("listen error.\n");
        exit(1);
    }//if

    clilen = sizeof(cliaddr);
    if ((connfd_1 = accept(listenfd_1, (struct sockaddr *) &cliaddr, &clilen)) < 0) {
        perror("accept error.\n");
        exit(1);
    }//if
    printf("server [client1]: got connection from %s\n", inet_ntoa(cliaddr.sin_addr));

    /*创建子线程处理该客户链接接收消息*/
    if (pthread_create(&recv_tid_1, NULL, recv_message, &connfd_1) == -1) {
        perror("pthread create error.\n");
        exit(1);
    }//if

    std::string command;
    init_xp_map();

    while(true)
    {
        // TODO: polling market data
        clMsgType smsgtest_1;
        smsgtest_1.operation = 0;
        smsgtest_1.num = 0;
        for (int i = 0; i < INSTRUMENT_COUNT; ++i) {
            strcpy(smsgtest_1.stockName, AllTickers[i].c_str());
            // TODO: send msg to 3 different EXCH
            if(send(connfd_1, &smsgtest_1, sizeof(clMsgType), 0) == -1)
            {
                perror("polling failed.\n");
                exit(1);
            }
        }

        char side;
        int qty;
        double price;
        std::string ticker;
        std::cout << " >> ";
        std::cin >> side;
        std::cin >> ticker;
        std::cin >> qty;
        std::cin >> price;
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
}
