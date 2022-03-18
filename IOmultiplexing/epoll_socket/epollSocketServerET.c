#include <stdio.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

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
    //! 监听socket不设为边沿触发模式，意义不大
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
        char buf[5];

        int num = epoll_wait(epollid, evs, evs_size, -1);
        for (i = 0; i < num; i++)
        {
            curfd = evs[i].data.fd;
            if (curfd == lsock)
            {
                // 7. 建立新的通信socket连接
                csock = accept(lsock, NULL, NULL);

                //! 边沿触发模式 设置 文件属性非阻塞
                int flag = fcntl(csock, F_GETFL);
                flag = flag | O_NONBLOCK;
                fcntl(csock, F_SETFL, flag);
                //! 设置通信socket为边沿触发模式
                ev.events = EPOLLIN | EPOLLET;
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
                //! 循环读数据，直至读缓冲区为空
                while (1)
                {
                    len = read(curfd, buf, sizeof(buf));
                    if (len == 0)
                    {
                        printf("客户端断开了连接\n");
                        epoll_ctl(epollid, EPOLL_CTL_DEL, curfd, NULL);
                        close(curfd);
                    }
                    else if (len == -1)
                    {
                        //! 设置非阻塞后，有两种情况了，一是数据读完不可读了，二是出错
                        if (errno = EAGAIN)
                        {
                            printf("数据已读完\n");

                            // todo break or continue?
                            break;
                        }
                        else
                        {
                            perror("read\n");
                            exit(0);
                        }
                    }
                    else
                    {
                        printf("client say: %s\n", buf);
                        write(curfd, buf, len);
                    }
                }
            }
        }
    }
    close(lsock);
    return 0;
}

// todo 关于epoll ET模式的一些细节
//! 1.  因为边缘模式触发这种机制，需要 循环 read()/recv() 读取缓冲区内容，
//!     当然另外一种方法就是加大一次读的容量（并不可取），那么当缓冲区读完清空时，
//!     默认是阻塞的，所以要设置该缓冲区文件为非阻塞模式
//! 边沿触发模式 设置 文件属性非阻塞
// int flag = fcntl(csock, F_GETFL);
// flag = flag | O_NONBLOCK;
// fcntl(csock, F_SETFL, flag);
// //! 设置通信socket为边沿触发模式
// ev.events = EPOLLIN | EPOLLET;

//! 2.  还是因为上述原因，在检查 read()/recv() 返回值时，要判断值为 -1 出错的情况
//!     利用 errno 这个全局变量来判断，是正常读完清空，还是出现异常
