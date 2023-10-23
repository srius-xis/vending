#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>

#include <stdio.h>

class TcpSocket
{
public:
    TcpSocket();
    TcpSocket(std::string ip, unsigned int port);
    TcpSocket(int sockfd, std::string ip, unsigned int port);

    // 连接服务器
    int connectHost(std::string ip, unsigned short port);

    // 发送数据
    int write(void *data, int len);
    int send(std::string data);

    // 接收数据
    int recv(void *data, int len);
    // int read(std::string data);

    std::string ip();
    unsigned int port();

    ~TcpSocket();

private:
    int m_fd;
    std::string m_ip;
    unsigned int m_port;
};

#endif // TCPSOCKET_H
