//
// Created by ChiuPhonic on 2017/12/26.
//

#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <stdio.h>
#include <string.h>
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

#include "../SmartRouteOrder/Order.h"
#include "../SmartRouteOrder/OrderBook.h"
#include "../public/Message.h"
#include "../public/Instrument.h"


#define PORT 7000
#define IP "127.0.0.1"
#define BUFFER_SIZE 1024

const int EXCHANGE_COUNT = 3;
const int INSTRUMENT_COUNT = 4;
const int MAX_PRICELEVEL = 20;
typedef std::map<std::string, pxlv*> plm;
typedef std::map<std::string, plm*> xpm;

std::string AllExch[EXCHANGE_COUNT];
std::string AllTickers[INSTRUMENT_COUNT];

std::map<std::string ,stockType> stock_map;

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

int main(int argc,char *argv[])
{
    int sock_cli;
    fd_set rfds;
    struct timeval tv;//设置时间
    int retval, maxfd;
    
    ///定义sockfd
    sock_cli = socket(AF_INET,SOCK_STREAM, 0);
    ///定义sockaddr_in
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);  ///服务器端口，利用htons将主机字节顺序转换为网路字节数序从而进行数据包的传送
    servaddr.sin_addr.s_addr = inet_addr(IP);  ///服务器ip
    
    //连接服务器，成功返回0，错误返回-1
    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect");
        exit(1);
    }
    init_xp_map();
    while(1){
        /*把可读文件描述符的集合清空*/
        FD_ZERO(&rfds);
        /*把标准输入的文件描述符加入到集合中*/
        FD_SET(0, &rfds);
        maxfd = 0;
        /*把当前连接的文件描述符加入到集合中*/
        FD_SET(sock_cli, &rfds);
        /*找出文件描述符集合中最大的文件描述符*/
        if(maxfd < sock_cli)
            maxfd = sock_cli;
        /*设置超时时间*/
        tv.tv_sec = 60;
        tv.tv_usec = 0;
        /*等待聊天*/
        retval = select(maxfd+1, &rfds, NULL, NULL, &tv);//int select(int maxfdp,fd_set *readfds,fd_set *writefds,fd_set *errorfds,struct timeval*timeout);  监视我们需要的文件的文件描述符的变化情况——读写或是异常
        if(retval == -1){
            printf("\n[FATAL] select error, exiting...\n");
            break;
        }else if(retval == 0){//超时
            printf("\n[TIMEOUT] no market-data received, waiting...\n");
            continue;
        }else{//文件可进行读写或者出错
            /*服务器发来了消息*/
            if(FD_ISSET(sock_cli,&rfds)){
                char recvbuf[BUFFER_SIZE];
                long len;
                len = recv(sock_cli, recvbuf, sizeof(recvbuf),0);
                printf("[CLIENT] mktdata rev\n");
                printf("%s=>", recvbuf);
                printf("%zu\n", strlen(recvbuf));
                auto md0 = mktDataType::parse(recvbuf);
                printf("ticker: %s\t", md0.ticker);
                printf("exchange: %s\n", md0.exchange);
                printf("%d @ %d\n", md0.bid_px[1], md0.bid_sz[1]);
                printf("%d @ %d\n", md0.ask_px[1], md0.ask_sz[1]);

                memset(recvbuf, 0, sizeof(recvbuf));
            }//if
            /*用户输入信息了,开始处理信息并发送*/
            if(FD_ISSET(0, &rfds)){
                char sendbuf[1024];
//                scanf("%s",sendbuf);



                std::string side;
                int qty;
                double price;
                std::string ticker;
                std::cout << " >> ";
                std::cin >> side;


                //TODO:MKT
                stock_map["IBM.US"].num=2000;
                stock_map["MS.US"].num=1500;
                stock_map["PTR.US"].num=1200;
                stock_map["BABA.US"].num=3500;

                stock_map["IBM.US"].price=148900;
                stock_map["MS.US"].price=48370;
                stock_map["PTR.US"].price=68920;
                stock_map["BABA.US"].price=182570;
                srand(time(NULL));
                stock_map["IBM.US"].price+=(rand()%500-250)*10;
                stock_map["MS.US"].price+=(rand()%200-100)*10;
                stock_map["PTR.US"].price+=(rand()%100-50)*10;
                stock_map["BABA.US"].price+=(rand()%1000-500)*10;


                for (int jk=0; jk<3; ++jk) {
                    auto exch = AllExch[jk];
                    for (int i=0; i<4; ++i) {
                        auto ticker = AllTickers[i];
                        int pricelist[20];
                        int numlist[20];
                        for (int i = 0; i < 20; ++i) {
                            int tmp = (i == 0) ? stock_map[ticker].price : pricelist[i-1];
                            pricelist[i] = tmp + (rand()%20)*10;
                            numlist[i] = (20-abs(19-2*i)+rand()%3)*100;
                            (*(*xp_map)[exch])[ticker][i].price = pricelist[i];
                            (*(*xp_map)[exch])[ticker][i].qty = numlist[i];
                        }
                    }
                }


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
                else if(side=="Subscribe")
                {
                    auto s0 = subMsgType();
                    s0.operation = '1';
                    sprintf(s0.ticker, "IBM.US");
                    auto p = s0.str();
                    for (int x=0; x<strlen(p); ++x) {
                        sendbuf[x] = p[x];
                    }
                    sendbuf[strlen(p)] = 0;

                    printf("[CLIENT] %s=>%zu", sendbuf, strlen(sendbuf));
                    send(sock_cli, sendbuf, strlen(sendbuf),0); //发送
                    memset(sendbuf, 0, sizeof(sendbuf));
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
                    nyse->showall();
                    nq->showall();
                    iex->showall();
                    Order* result = find_orders(*orderbook, *o, level);
                    ShowSmartOrder(result, level);



                    //TODO:


//                    send(sock_cli, sendbuf, strlen(sendbuf),0); //发送
//                    memset(sendbuf, 0, sizeof(sendbuf));

                }



                //fgets(sendbuf, sizeof(sendbuf), stdin);


            }//if
        }//if
    }//while
    
    close(sock_cli);
    return 0;
}
