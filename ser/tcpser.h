#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

 #include <unistd.h>

#include <cstdio>

#include "tcpsock.h"

class TcpServer
{
public:
    TcpServer();
    ~TcpServer();

    //绑定自己的IP地址和端口号 ，并且监听
    int listen(std::string ip,unsigned short port = 12000);

    TcpSocket* accept();

    int sockfd();
private:
    int m_fd;
};

#endif // TCPSERVER_H
