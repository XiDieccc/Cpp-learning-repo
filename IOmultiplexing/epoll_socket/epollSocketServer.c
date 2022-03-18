#include <stdio.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#define SERVER_PORT 8888
#define SERVER_IPADDR "192.168.233.101"

int main()
{
    int lsock, csock;
    struct sockaddr_in server_addr;
    socklen_t socklen = sizeof(server_addr);
    struct epoll_event ev;
    struct epoll_event evs[1024];
    int evs_size = sizeof(evs) / sizeof(struct epoll_event);

    // 1.创建监听socket
    lsock = socket(AF_INET, SOCK_STREAM, 0);
    if (lsock == -1)
    {
        perror("socket\n");
        exit(0);
    }

    // 2.绑定监听socket的ip和端口，设置端口复用
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = ntohs(SERVER_PORT);
    // server_addr.sin_addr.s_addr = ntohl(INADDR_ANY);
    inet_pton(AF_INET, SERVER_IPADDR, &server_addr.sin_addr.s_addr);

    int opt = 1;
    setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    int ret = bind(lsock, (struct sockaddr *)&server_addr, socklen);
    if (ret == -1)
    {
        perror("bind\n");
        exit(0);
    }

    // 3. 设置监听
    ret = listen(lsock, 128);
    if (ret == -1)
    {
        perror("listen\n");
        exit(0);
    }

    // 4. 创建epoll管理实例红黑树
    int epollid = epoll_create(100);
    if (epollid == -1)
    {
        perror("epoll_create\n");
        exit(0);
    }

    // 5. 往epoll树上添加监听节点
    ev.events = EPOLLIN;
    ev.data.fd = lsock;
    ret = epoll_ctl(epollid, EPOLL_CTL_ADD, lsock, &ev);
    if (ret == -1)
    {
        perror("epoll_ctl\n");
        exit(0);
    }

    // 6. 开始持续检测通信
    while (1)
    {
        int i, curfd, len;
        char buf[1024];

        int num = epoll_wait(epollid, evs, evs_size, -1);
        for (i = 0; i < num; i++)
        {
            curfd = evs[i].data.fd;
            if (curfd == lsock)
            {
                // 7. 建立新的通信socket连接
                csock = accept(lsock, NULL, NULL);
                ev.events = EPOLLIN;
                ev.data.fd = csock;
                ret = epoll_ctl(epollid, EPOLL_CTL_ADD, csock, &ev);
                if (ret == -1)
                {
                    perror("epoll_ctl accept\n");
                    exit(0);
                }
            }
            else
            {
                // 8. 处理通信socket
                memset(buf, 0, sizeof(buf));
                len = read(curfd, buf, sizeof(buf));
                if (len == 0)
                {
                    printf("客户端断开了连接\n");
                    epoll_ctl(epollid, EPOLL_CTL_DEL, curfd, NULL);
                    close(curfd);
                }
                else if (len == -1)
                {
                    perror("read\n");
                    exit(0);
                }
                else
                {
                    printf("client say: %s\n", buf);
                    write(curfd, buf, len);
                }
            }
        }
    }
    close(lsock);
    return 0;
}