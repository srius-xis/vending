#include <string>
#include <vector>

#include <pthread.h>

#include <cstring>

#include "tcpser.h"
#include "tcpsock.h"

struct Host
{
    pthread_t tid;
    std::string ip;
    unsigned port;
};

std::vector<Host> head;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *task(void *arg)
{
    pthread_detach(pthread_self());
    TcpSocket *client = (TcpSocket *)arg;

    Host host = (Host){pthread_self(), client->ip(), client->port()};

    pthread_mutex_lock(&mutex);
    head.push_back(host);
    pthread_mutex_unlock(&mutex);

    char str[1025] = {'\0'};
    while (1)
    {
        memset(str, 0, 1025);
        client->recv(str, 1024);
        // if (!strlen(str))
        std::cout << "client ip: " << client->ip() << "; port: " << client->port() << ";\n massage:\n{\n\n"
                  << str << "\n}\n";
    }
}

int main(int argc, char const *argv[])
{
    TcpServer ser;
    ser.listen("192.168.2.29", 54213);
    fd_set set;
    int ready = -1;
    while (1)
    {
        FD_ZERO(&set);
        FD_SET(ser.sockfd(), &set);
        if (select(ser.sockfd() + 1, &set, NULL, NULL, NULL) > 0)
        {
            if (FD_ISSET(ser.sockfd(), &set))
            {
                TcpSocket *client = ser.accept();
                pthread_t tid;
                pthread_create(&tid, NULL, task, (void *)client);
            }
        }
    }
    return 0;
}
