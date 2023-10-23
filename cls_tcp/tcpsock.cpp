#include "tcpsock.h"

TcpSocket::TcpSocket()
    : m_fd(-1), m_ip(""), m_port(0)
{
    m_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_fd == -1)
    {
        perror("socket error");
        return;
    }
}

TcpSocket::TcpSocket(std::string ip, unsigned int port)
    : m_fd(-1), m_ip(""), m_port(0)
{
    m_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_fd == -1)
    {
        perror("socket error");
        return;
    }
    m_ip = ip;
    m_port = port;
    
    int enable = 1;
    setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
}

TcpSocket::TcpSocket(int sockfd, std::string ip, unsigned int port)
{
    m_fd = sockfd;
    m_ip = ip;
    m_port = port;

    int enable = 1;
    setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
}

int TcpSocket::connectHost(std::string ip, unsigned short port)
{
    struct sockaddr_in ser_in;
    ser_in.sin_family = AF_INET;
    ser_in.sin_port = htons(port);
    // in_addr_t inet_addr(const char *cp);  本地IP---》网络IP
    ser_in.sin_addr.s_addr = inet_addr(ip.data());

    int ret = connect(m_fd, (struct sockaddr *)&ser_in, sizeof(struct sockaddr_in));
    if (ret == -1)
    {
        std::cout << "connect to host error";
        return -1;
    }

    std::cout << "connect to host success" << std::endl;
    return 0;
}

int TcpSocket::write(void *data, int len)
{
    int ret = ::write(m_fd, data, len);
    if (ret <= 0)
    {
        perror("write error");
    }

    return ret;
}

int TcpSocket::send(std::string data)
{
    int ret = ::write(m_fd, data.data(), data.size());
    if (ret <= 0)
    {
        perror("write error");
    }

    return ret;
}

int TcpSocket::recv(void *data, int len)
{
    int ret = ::read(m_fd, data, len);
    if (ret == 0)
    {
        std::cout << "server close" << std::endl;
        close(m_fd);
        m_fd = 0;
    }

    return ret;
}

std::string TcpSocket::ip()
{
    return m_ip;
}

unsigned int TcpSocket::port()
{
    return m_port;
}

TcpSocket::~TcpSocket()
{
    if (m_fd > 0)
    {
        ::close(m_fd);
    }
}