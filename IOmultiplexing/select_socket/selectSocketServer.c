#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_PORT 9999
static char *ipaddr = "192.168.233.101";

int main()
{

    int lsock;
    struct sockaddr_in serverAddr;
    socklen_t socklen = sizeof(serverAddr);

    //! 1.创建监听socket
    lsock = socket(AF_INET, SOCK_STREAM, 0);
    if (lsock == -1)
    {
        printf("服务socket创建失败\n");
        exit(0);
    }
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_pton(AF_INET, ipaddr, &serverAddr.sin_addr.s_addr);

    // 设置端口复用
    int opt = 1;
    setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //! 绑定地址和端口和协议
    int ret = bind(lsock, (struct sockaddr *)&serverAddr, socklen);
    if (ret == -1)
    {
        printf("绑定bind失败\n");
        exit(0);
    }

    ret = listen(lsock, 128);
    if (ret == -1)
    {
        printf("监听listen失败\n");
        exit(0);
    }

    //! select多路复用
    int maxfd = lsock;
    fd_set read_set;
    fd_set read_tmp;
    FD_ZERO(&read_set);
    FD_SET(lsock, &read_set);

    int csock;
    struct sockaddr_in client_addr;
    int i, len;
    // todo char buf[1024]={0};
    char buf[1024];

    while (1)
    {
        read_tmp = read_set;

        //! select 添加检测文件描述符，以及阻塞
        int num = select(maxfd + 1, &read_tmp, NULL, NULL, NULL);
        if (num == -1)
        {
            printf("select检测失败\n");
            exit(0);
        }
        if (FD_ISSET(lsock, &read_tmp))
        {
            csock = accept(lsock, (struct sockaddr *)&client_addr, &socklen);

            // todo test
            char ip[24];
            printf("ip%s,port%d\n",
                   //    ntohl(client_addr.sin_addr.s_addr),       //! 返回值不便于输出
                   inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip, sizeof(ip)), //! 可以转为点分十进制，便于输出观看
                   ntohs(client_addr.sin_port));

            if (csock == -1)
            {
                printf("通信socket创建失败\n");
                continue;
            }
            maxfd = maxfd > csock ? maxfd : csock;
            FD_SET(csock, &read_set);

            if (num == 1) //! 改进，说明select只检测到了用于监听的 lsock，那么可进入下次循环了
            {
                continue;
            }
        }

        // for (i = 0; i < maxfd + 1; i++)      //! 没必要从 0 开始循环判断，下面改进从 lsock+1 开始
        for (i = lsock + 1; i < maxfd + 1; i++) //! 改进，用于通信的socket文件描述符一定是比 lsock 要大的，因为文件描述符分配时是递增的
        {
            //! 检测到csock即可以socket通信了，此时 i=csock
            if (i != lsock && FD_ISSET(i, &read_tmp))
            {
                memset(buf, 0, sizeof(buf));
                len = read(i, buf, sizeof(buf));
                if (len == 0)
                {
                    printf("客户端断开了连接\n");
                    FD_CLR(i, &read_set);
                    close(i);
                    continue;
                }
                else if (len == -1)
                {
                    printf("接收数据出错\n");
                    continue;
                }
                else
                {
                    // todo test
                    printf("收到了数据:%s\n", buf);
                    write(i, buf, len);
                }
            }
            // FD_CLR(i, &read_set);
            // close(i);
        }
    }

    //! 关闭 发出TCP FIN 挥手报文
    close(lsock);
    return 0;
}

// todo 关于以上几点细节
//! 1.  ntohl(client_addr.sin_addr.s_addr)      返回值不便于输出
//!     inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip, sizeof(ip))    可以转为点分十进制，便于输出观看

//! 2.  for (i = 0; i < maxfd + 1; i++)     没必要从 0 开始循环判断，改进从 lsock+1 开始
//!     for (i = lsock + 1; i < maxfd + 1; i++)     改进，用于通信的socket文件描述符一定是比 lsock 要大的，因为文件描述符分配时是递增的

//! 3.  在 FD_ISSET(lsock,&read_set)检测成功后，直接判断    int num = select(maxfd + 1, &read_tmp, NULL, NULL, NULL)    返回的 num
//!     若是直接为 1，就可以 continue 进行下次循环了

// todo 关于select 缺点
//! 1.  只能轮询来检测通信socket的文件描述符，可以进行代码方面的优化，用数组来存储 accept() 后
//!     返回的 csock，然后下面比较的时候，不从 read_set（即 fd_set 读缓冲集合）轮询，而是从设定上限可变的数组中轮询判断
//!     当然这只适用于 连接请求数稀疏的情况

//! 2.  同上，连接数密集时，上限是 1024,虽然可以通过 ulimit  更改系统的 文件描述符 分配的上限，但是较为麻烦

// todo 关于select的 优点
//! 1.  可以跨平台 linux/windows/macOS
