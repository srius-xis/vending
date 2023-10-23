#include "tcpser.h"

TcpServer::TcpServer()
{
    // 建立套接字
    m_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_fd == -1)
    {
        perror("socket error");
        return;
    }
}

TcpServer::~TcpServer()
{
}

int TcpServer::listen(std::string ip, unsigned short port)
{
    // 端口复用
    int enable = 1;
    setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

    // 绑定
    struct sockaddr_in ser_in;
    ser_in.sin_family = AF_INET;
    ser_in.sin_port = htons(port);
    // in_addr_t inet_addr(const char *cp);  本地IP---》网络IP
    ser_in.sin_addr.s_addr = inet_addr(ip.data());

    int ret = bind(m_fd, (struct sockaddr *)&ser_in, sizeof(ser_in));
    if (ret == -1)
    {
        perror("bind error");
        return -1;
    }
    // 监听
    ret = ::listen(m_fd, 20);
    if (ret == -1)
    {
        perror("listen error");
        return -1;
    }

    return ret;
}

TcpSocket *TcpServer::accept()
{
    struct sockaddr_in addr;
    unsigned int addrlen = sizeof(struct sockaddr_in);

    int newClientFd = ::accept(m_fd, (struct sockaddr *)&addr, &addrlen);

    // char *inet_ntoa(struct in_addr in);
    std::cout << "new client connect ip:" << inet_ntoa(addr.sin_addr) << " port" << ntohs(addr.sin_port) << std::endl;

    return new TcpSocket(newClientFd, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
}

int TcpServer::sockfd()
{
    return m_fd;
}
