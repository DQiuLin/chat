#include <iostream>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

const int BUFF_SIZE = 256;
const int SERVER_PORT = 8888;
const char SERVER_IP[] = "127.0.0.1";

using namespace std;

void clientRead(int sockid, bool &flag)
{
    char buf[BUFF_SIZE];
    memset(buf, 0, sizeof(buf));
    int rLen = read(sockid, buf, sizeof(buf));
    if ((strcmp(buf, "quit\n") == 0) || (strcmp(buf, "exit\n") == 0))
    {
        printf("对方下线.\n");
        flag = false;
        return;
    }
    if (rLen > 0)
    {
        printf("收到消息：%s", buf);
    }
}

void clientWrite(int sockid, bool &flag)
{
    char buf[BUFF_SIZE];
    memset(buf, 0, sizeof(buf));
    int wLen = read(0, buf, sizeof(buf));
    if ((strcmp(buf, "quit\n") == 0) || (strcmp(buf, "exit\n") == 0))
    {
        printf("下线成功.\n");
        write(sockid, buf, sizeof(buf));
        flag = false;
        return;
    }
    if (wLen > 0)
    {
        write(sockid, buf, sizeof(buf));
        memset(buf, 0, strlen(buf));
    }
}

int main()
{
    int sockid = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    if (connect(sockid, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        printf("connect failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    int flags = fcntl(sockid, F_GETFL, 0);
    fcntl(sockid, F_SETFL, flags | O_NONBLOCK);
    flags = fcntl(0, F_GETFL, 0);
    fcntl(0, F_SETFL, flags | O_NONBLOCK);
    printf("\n******** 欢迎进入聊天室 ********\n");
    bool flag = true;
    while (flag)
    {
        clientRead(sockid, flag);
        clientWrite(sockid, flag);
    }
    close(sockid);
    return 0;
}