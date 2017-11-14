//
// Created by ChiuPhonic on 2017/11/15.
//

#ifndef FSC_EXCHIMPL_H
#define FSC_EXCHIMPL_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <map>
#include "../public/Message.h"
#include "../public/Instrument.h"
using namespace std;

const int MAX_LINE = 2048;
//const int BACKLOG = 10;
//const int MAX_CONNECT = 20;

class ExchImpl {
public:
    ExchImpl( const string &_CN, const string &_IP, const int _PORT, const int _LISTENQ=6666 ) {
        ClientName = _CN;
        IP = _IP;
        PORT = _PORT;
        LISTENQ = _LISTENQ;
        pthread_attr_init(&threadAttr);
        create_socket();
        tcp_connect();
    }
    ~ExchImpl() {}

private:
    static string ClientName;
    static string IP;
    static int PORT;
    static int LISTENQ;

    static pthread_mutex_t StockMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_attr_t threadAttr;
    int sockfd;
    pthread_t recv_tid , send_tid , randstock_tid;
    struct sockaddr_in servaddr;

    void create_socket();
    void tcp_connect();

    static void *RandomStock(void *fd);
    static void *recv_message(void *fd);

public:
    static map<string ,stockType> stock_map;
    void run();
};


#endif //FSC_EXCHIMPL_H
