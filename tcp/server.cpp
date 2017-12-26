//
// Created by ChiuPhonic on 2017/12/26.
//

#include <arpa/inet.h>//包含socket函数使用的各种协议，send(), recv()
#include <unistd.h>//调用linux系统函数的头文件(read(), write(), send(), select())
#include <iostream>
#include <thread>
#include <list>
#include "../public/Message.h"
#define PORT 7000
#define IP "127.0.0.1"

int s;
struct sockaddr_in servaddr;
socklen_t len;
std::list<int> li;

void getConn() {
    while(1){
        int conn = accept(s, (struct sockaddr*)&servaddr, &len);//第二个参数保存客户端套接字对应的IP地址和port 端口信息
        li.push_back(conn);
        printf("%d\n", conn);
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
            }else{
                char buf[1024];
                memset(buf, 0 ,sizeof(buf));
                long len = recv(*it, buf, sizeof(buf), 0);
                printf("%s", buf);
            }
        }
        sleep(1);
        
    }
}

void sendMess() {
    while(1) {
        char buf[1024];
        fgets(buf, sizeof(buf), stdin);//从文件流读取一行，送到缓冲区，使用时注意以下几点
        std::list<int>::iterator it;
        for(it=li.begin(); it!=li.end(); ++it){
            send(*it, buf, sizeof(buf), 0);
        }
    }
}

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
    std::thread t1(sendMess);
    t1.detach();
    std::thread t2(getData);
    t2.detach();

    while(1){
        
    }
}
