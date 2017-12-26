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
#include "../public/Instrument.h"

using namespace std;
const int MAX_LINE = 2048;
const int PORT = 20003;
const int PORT_X = 20005;
const int BACKLOG = 10;
const int LISTENQ = 6666;
const int MAX_CONNECT = 20;
const int EXCHANGE_COUNT = 3;
const int INSTRUMENT_COUNT = 4;
const int MAX_PRICELEVEL = 20;

typedef std::map<std::string, pxlv*> plm;
typedef std::map<std::string, plm*> xpm;

std::string AllExch[EXCHANGE_COUNT];
std::string AllTickers[INSTRUMENT_COUNT];

xpm *xp_map = new xpm();

void init_xp_map()
{
    AllExch[0] = "NYSE";
    AllExch[1] = "NASDAQ";
    AllExch[2] = "IEX";

    AllTickers[0] = "IBM.US";
    AllTickers[1] = "MS.US";
    AllTickers[2] = "PTR.US";
    AllTickers[3] = "BABA.US";

    for (int i = 0; i < EXCHANGE_COUNT; ++i) {
        std::string exch = AllExch[i];
        (*xp_map)[exch] = new plm();
        for (int j = 0; j < INSTRUMENT_COUNT; ++j) {
            std::string ticker = AllTickers[j];
            (*(*xp_map)[exch])[ticker] = new pxlv[MAX_PRICELEVEL];
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

        for (int i = 0; i < MAX_PRICELEVEL; ++i) {
            std::string exch = cmsgtest1.clientName;
            std::string ticker = cmsgtest1.stockName;
//            std::cout << exch << "@" << ticker << ": " << cmsgtest1.price[i] << " -> " << cmsgtest1.num[i] << std::endl;
            (*(*xp_map)[exch])[ticker][i].price = cmsgtest1.price[i];
            (*(*xp_map)[exch])[ticker][i].qty = cmsgtest1.num[i];
//            std::cout << "[" << exch << "@" << ticker << "]: " << (*(*xp_map)[exch])[ticker][i].price << " -> " << (*(*xp_map)[exch])[ticker][i].qty << std::endl;
        }
    }//while
}

void fix_orders(Order* o, int const level, int const qty, int const curQty)
{
    if(level==0)
        return;
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
        (o+i)->leavesQty = int(cum*beta)-beta_cum-(o+i)->qty;
        (o+i)->qty = int(cum*beta)-beta_cum;
        beta_cum+=(o+i)->qty;
    }
}

void ShowSmartOrder(Order* o, int const level)
{
    printf("%-6s%-10s%-10s%-10s%-10s%-10s\n","SIDE","TICKER","QUANTITY","LEAVESQTY","PRICE","EXCHANGE");
    if(level==0)
        return;
    for(int i = 0;i<level;++i)
    {
        if((o+i)->exch=="NYSE")
        {
            printf("%-6c%-10s%-10d%-10d%-10.2f%-10s\n",(o+i)->side,(o+i)->ticker.c_str(),(o+i)->qty,(o+i)->leavesQty,double((o+i)->priceInt)/1000, (o+i)->exch.c_str());
        }
    }
    for(int i = 0;i<level;++i)
    {
        if((o+i)->exch=="NASDAQ")
        {
            printf("%-6c%-10s%-10d%-10d%-10.2f%-10s\n",(o+i)->side,(o+i)->ticker.c_str(),(o+i)->qty,(o+i)->leavesQty,double((o+i)->priceInt)/1000, (o+i)->exch.c_str());
        }
    }
    for(int i = 0;i<level;++i)
    {
        if((o+i)->exch=="IEX")
        {
            printf("%-6c%-10s%-10d%-10d%-10.2f%-10s\n",(o+i)->side,(o+i)->ticker.c_str(),(o+i)->qty,(o+i)->leavesQty,double((o+i)->priceInt)/1000, (o+i)->exch.c_str());
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

    if(side=='S'){
        int bid_size = ob.bidlevel();
        ret = new Order[bid_size];
        for(int i = 0;i<bid_size;++i)
        {
            if(cumQty>=size || px>ob.bid[bid_size-1-i].priceInt)
                break;
            if(cumQty + ob.bid[bid_size-1-i].qty>size)
            {
                ret[i] = Order('s', size-cumQty, 0,ob.bid[bid_size-1-i].priceInt, ob.bid[bid_size-1-i].ticker,ob.bid[bid_size-1-i].exch);
                cumQty = size;
                level ++;
            }
            else if (cumQty + ob.bid[bid_size-1-i].qty<=size)
            {
                ret[i] = Order('s', ob.bid[bid_size-1-i].qty, 0,ob.bid[bid_size-1-i].priceInt, ob.bid[bid_size-1-i].ticker,ob.bid[bid_size-1-i].exch);
                cumQty += ob.bid[bid_size-1-i].qty;
                level ++;
            }
        }
    }
    else{
        int ask_size = ob.asklevel();
        ret = new Order[ask_size];
        for(int i = 0;i<ask_size;++i)
        {
            if(cumQty>=size || px<ob.ask[i].priceInt)
                break;
            if(cumQty + ob.ask[i].qty>size)
            {
                ret[i] = Order('b', size-cumQty, 0, ob.ask[i].priceInt, ob.ask[i].ticker,ob.ask[i].exch);
                cumQty = size;
                level ++;
            }
            else if (cumQty + ob.ask[i].qty<=size)
            {
                ret[i] = Order('b', ob.ask[i].qty, 0, ob.ask[i].priceInt, ob.ask[i].ticker,ob.ask[i].exch);
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
    servaddr_1.sin_port = htons(PORT);
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
//    std::cout << "before init_xp_map" << std::endl;
    init_xp_map();
//    std::cout << "after init_xp_map" << std::endl;

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

        std::string side;
        int qty;
        double price;
        std::string ticker;
        std::cout << " >> ";
        std::cin >> side;
        if(side=="BBO")
        {
            std::cin >> ticker;
            for (int i = 0; i < EXCHANGE_COUNT; ++i) {
                std::string exch = AllExch[i];
                printf("\n %s@%s \n BestBid: %d => %d \n BestOffer: %d => %d\n", ticker.c_str(), exch.c_str(),
                       (*(*xp_map)[exch])[ticker][9].price, (*(*xp_map)[exch])[ticker][9].qty,
                       (*(*xp_map)[exch])[ticker][10].price, (*(*xp_map)[exch])[ticker][10].qty);
            }
        }
        else if(side=="OrderBook")
        {
            std::cin >> ticker;
            OrderBook* nyse = get_orderbook_by_exch(ticker, AllExch[0], xp_map);
            OrderBook* nq = get_orderbook_by_exch(ticker, AllExch[1], xp_map);
            OrderBook* iex = get_orderbook_by_exch(ticker, AllExch[2], xp_map);
            nyse->showall();
            nq->showall();
            iex->showall();
        }
        else
        {
            std::cin >> ticker;
            std::cin >> qty;
            std::cin >> price;
            std::string exch = "NONE";
            Order* o = new Order(side[0], qty, 0, int(1000*price), ticker, exch);
            OrderBook* nyse = get_orderbook_by_exch(ticker, AllExch[0], xp_map);
            OrderBook* nq = get_orderbook_by_exch(ticker, AllExch[1], xp_map);
            OrderBook* iex = get_orderbook_by_exch(ticker, AllExch[2], xp_map);
            OrderBook* orderbook = intergrate_orderbooks(*nyse, *nq, *iex);
            int level;
            Order* result = find_orders(*orderbook, *o, level);
            ShowSmartOrder(result, level);
        }

    }
}
