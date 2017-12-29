//
// Created by ChiuPhonic on 2017/12/26.
//

#include <arpa/inet.h>//包含socket函数使用的各种协议，send(), recv()
#include <unistd.h>//调用linux系统函数的头文件(read(), write(), send(), select())
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <vector>
#include <list>
#include "../public/Message.h"
#include "../AdvOrderBook.h"
#define PORT 7000
#define IP "127.0.0.1"

int s;
struct sockaddr_in servaddr;
socklen_t len;
std::list<int> li;


//std::vector<std::string> subscribed_list(4);

//std::string nyse = "NYSE";
//std::string nasd = "NASDAQ";
//std::string iex = "IEX";
//AdvExchange exch1;// = AdvExchange::AdvExchange(nyse);
//AdvExchange exch2;// = AdvExchange::AdvExchange(nasd);
//AdvExchange exch3;// = AdvExchange::AdvExchange(iex);
//
//void init_static()
//{
//    subscribed_list[0] = "IBM.US";
//    subscribed_list[1] = "MS.US";
//    subscribed_list[2] = "PTR.US";
//    subscribed_list[3] = "MSFT.US";
//
//    exch1 = AdvExchange::AdvExchange(nyse);
//    exch2 = AdvExchange::AdvExchange(nasd);
//    exch3 = AdvExchange::AdvExchange(iex);
//}


void getConn() {
    while(1) {
        int conn = accept(s, (struct sockaddr*)&servaddr, &len);//第二个参数保存客户端套接字对应的IP地址和port 端口信息
        li.push_back(conn);
        printf("%d\n", conn);
    }
}

void send_mkt_data() {
    printf("[SERVER] into new thread!! \n");
    while(1) {

//        for (std::vector<int>::iterator itr=subscribed_list.begin(); itr!=subscribed_list.end(); ++itr) {
//            auto ticker = *itr;
//            auto md = mktDataType();
//            strncpy(md.ticker, ticker, strlen(ticker.c_str()));
//            strncpy(md.exchange, "NYSE", 4);
//
//            for(auto iter = exch1.books.begin();iter!=exch1.books.end();iter++)
//            {
//                printf("Ticker: %s\n", iter->first.c_str());
//                iter->second->showAll();
//            }
//        }
        auto md = mktDataType();
        // TODO: generate md
        strncpy(md.ticker, "IBM.US", 6);
        strncpy(md.exchange, "NYSE", 4);
        md.bid_px[0] = 1;
        md.bid_sz[0] = 1;
        md.ask_px[0] = 1;
        md.ask_sz[0] = 1;
        md.bid_px[1] = 100500;
        md.bid_sz[1] = 300;
        md.ask_px[1] = 100600;
        md.ask_sz[1] = 500;
        auto p = md.str();
        char buf[1024];
        for (int x=0; x<strlen(p); ++x) {
            buf[x] = p[x];
        }
        buf[strlen(p)] = 0;
        std::list<int>::iterator it;
        for (it=li.begin(); it!=li.end(); ++it) {
            send(*it, buf, sizeof(buf), 0);
        }
        printf("%s=>", buf);
        printf("%zu\n", strlen(buf));
        printf("[SERVER] send mkt!! \n");
        sleep(1);
    }
}

void getData() {
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    while(1) {
        std::list<int>::iterator it;
        for(it=li.begin(); it!=li.end(); ++it){
            fd_set rfds;
            FD_ZERO(&rfds);
            int maxfd = 0;
            int retval = 0;
            FD_SET(*it, &rfds);
            if(maxfd < *it){
                maxfd = *it;
            }
            retval = select(maxfd+1, &rfds, NULL, NULL, &tv);//实现非阻塞式的通信，即需要等待时间的发生，一旦执行一定返回，返回的结果不同以表示函数执行的结果
            if(retval == -1){
                printf("select error\n");
            }else if(retval == 0) {
                //
            }else{
                char buf[1024];
                memset(buf, 0 ,sizeof(buf));
                long len = recv(*it, buf, sizeof(buf), 0);
                printf("%s\n", buf);
                printf("%zu\n", strlen(buf));
                char msg_type = buf[0];
                switch (msg_type) {
                    case 'S':
                        printf("[SERVER] subscribe rev\n");
                        char msg[10];
                        snprintf(msg, 10+1, "%s", buf);
                        printf("[SERVER] %s=>%zu", msg, strlen(msg));
                        auto s0 = subMsgType::parse(msg);
                        printf("[%c]\n", s0.operation);
                        if (s0.operation == '1') {
//                            std::string ticker = s0.ticker;
//                            bool flag = false;
//                            for (std::vector<int>::iterator itr=subscribed_list.begin(); itr!=subscribed_list.end(); ++itr) {
//                                if (*itr == ticker) {
//                                    flag = true;
//                                    break;
//                                }
//                            }
//                            if (!flag) {
//                                subscribed_list.push_back(ticker);
//                            }
                            std::thread t_sub(send_mkt_data);
                            t_sub.detach();
                        }
                        break;
//                    case 'O':
//                        break;
//                    default:
//                        break;
                }
            }
        }
        sleep(1);
        
    }
}

//void sendMess() {
//    while(1) {
//        char buf[1024];
//        fgets(buf, sizeof(buf), stdin);//从文件流读取一行，送到缓冲区，使用时注意以下几点
//        std::list<int>::iterator it;
//        for(it=li.begin(); it!=li.end(); ++it){
//            send(*it, buf, sizeof(buf), 0);
//        }
//    }
//}

int main() {
    s = socket(AF_INET, SOCK_STREAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(IP);
    if(bind(s, (struct sockaddr* ) &servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        exit(1);
    }
    if(listen(s, 20) == -1) {
        perror("listen");
        exit(1);
    }

    len = sizeof(servaddr);
    std::thread t(getConn);
    t.detach();
//    std::thread t1(sendMess);
//    t1.detach();
    std::thread t2(getData);
    t2.detach();

    while(1){
        
    }
}
