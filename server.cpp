#include <iostream>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <mm_malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

const int MAXN_CLIENT = 2;
const int BUFF_SIZE = 256;
const int SERVER_PORT = 8888;
const int SLEEP_TIME = 1500;

using namespace std;

typedef struct NewClient
{
    int socketId;
    char *addr;
    int port;
} newClient;

void readClient(int clientA, int clientB, newClient new_client[], bool &f)
{
    char buf[BUFF_SIZE];
    memset(buf, 0, sizeof(buf));
    if (read(new_client[clientA].socketId, buf, sizeof(buf)) > 0)
    {
        time_t t;
        time(&t);
        if ((strcmp(buf, "quit\n") == 0) || (strcmp(buf, "exit\n") == 0))
        {
            printf("ip=%s在%s申请退出.\n", new_client[clientA].addr, ctime(&t));
            write(new_client[clientB].socketId, buf, strlen(buf));
            usleep(SLEEP_TIME);
            f = false;
            return;
        }
        printf("ip=%s在%s发出消息：%s", new_client[clientA].addr, ctime(&t), buf);
        write(new_client[clientB].socketId, buf, strlen(buf));
    }
}

int main()
{
    int server_sockid = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sockaddr;
    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(SERVER_PORT);
    sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    const int on = 1;
    if (setsockopt(server_sockid, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) //设置端口可重复利用
    {
        printf("setsockopt: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    if (bind(server_sockid, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0)
    {
        printf("bind failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    if (listen(server_sockid, SOMAXCONN) < 0)
    {
        printf("listen failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    newClient new_client[MAXN_CLIENT];
    for (auto &c : new_client)
    {
        c.socketId = 0;
        c.addr = NULL;
        c.port = 0;
    }

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    for (auto &c : new_client)
    {
        int client_socket = accept(server_sockid, (struct sockaddr *)&addr, &(addr_len));
        c.socketId = client_socket;
        c.addr = inet_ntoa(addr.sin_addr);
        c.port = ntohs(addr.sin_port);
        printf("ip=%s, port=%d：连接成功！\n", c.addr, c.port);
    }

    for (auto &c : new_client) //将socket设置非阻塞
    {
        int flags = fcntl(c.socketId, F_GETFL, 0);
        fcntl(c.socketId, F_SETFL, flags | O_NONBLOCK);
    }

    printf("\n******** client聊天开始 ********\n");
    bool flag = true;
    while (flag)
    {
        readClient(0, 1, new_client, flag);
        readClient(1, 0, new_client, flag);
    }

    close(server_sockid);
    return 0;
}