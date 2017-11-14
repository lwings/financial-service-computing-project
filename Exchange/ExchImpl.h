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
    ExchImpl( const string &CN, const string &IP, const int PORT, const int LISTENQ=6666 ) :
            ClientName(CN), IP(IP), PORT(PORT), LISTENQ(LISTENQ) {
        pthread_attr_init(&threadAttr);
        create_socket();
        tcp_connect();
    }
    ~ExchImpl() {}

private:
    int PORT;
    int LISTENQ;
    string IP;
    string ClientName;

    pthread_mutex_t stockMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_attr_t threadAttr;
    int sockfd;
    pthread_t recv_tid , send_tid , randstock_tid;
    struct sockaddr_in servaddr;

    void *RandomStock(void *fd);
    void *recv_message(void *fd);
    void create_socket();
    void tcp_connect();

public:
    map<string ,stockType> stock_map;
    void run();
};


#endif //FSC_EXCHIMPL_H
