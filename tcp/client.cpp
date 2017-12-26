#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#define MYPORT  7000
#define BUFFER_SIZE 1024
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
    char s[1024];
    int a;
    std::cout << "输入想要建立连接的端口号以及IP地址:" << std::endl;
    scanf("%d",&a);
    getchar();
    scanf("%s",s);
    servaddr.sin_port = htons(a);  ///服务器端口，利用htons将主机字节顺序转换为网路字节数序从而进行数据包的传送
    servaddr.sin_addr.s_addr = inet_addr(s);  ///服务器ip
    
    //连接服务器，成功返回0，错误返回-1
    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect");
        exit(1);
    }
    
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
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        /*等待聊天*/
        retval = select(maxfd+1, &rfds, NULL, NULL, &tv);//int select(int maxfdp,fd_set *readfds,fd_set *writefds,fd_set *errorfds,struct timeval*timeout);  监视我们需要的文件的文件描述符的变化情况——读写或是异常
        if(retval == -1){
            printf("select出错，客户端程序退出\n");
            break;
        }else if(retval == 0){//超时
            printf("客户端没有任何输入信息，并且服务器也没有信息到来，waiting...\n");
            continue;
        }else{//文件可进行读写或者出错
            /*服务器发来了消息*/
            if(FD_ISSET(sock_cli,&rfds)){
                char recvbuf[BUFFER_SIZE];
                long len;
                len = recv(sock_cli, recvbuf, sizeof(recvbuf),0);
                printf("%s", recvbuf);
                memset(recvbuf, 0, sizeof(recvbuf));
            }
            /*用户输入信息了,开始处理信息并发送*/
            if(FD_ISSET(0, &rfds)){
                char sendbuf[1024];
//                scanf("%s",sendbuf);
                fgets(sendbuf, sizeof(sendbuf), stdin);
                send(sock_cli, sendbuf, strlen(sendbuf),0); //发送
                memset(sendbuf, 0, sizeof(sendbuf));
            }
        }
    }
    
    close(sock_cli);
    return 0;
}
