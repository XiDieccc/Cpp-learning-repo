#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int main(void *arg)
{
    int sock;
    struct sockaddr_in server_addr;

    //! 创建服务端 用于监听的套接字
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("listening socket create failed...\n");
        exit(0);
    }

    // bzero(&server_addr, sizeof(server_addr)); // 清空地址短裤结构体信息，便于赋值

    server_addr.sin_family = AF_INET;         // 指定IP协议版本
    server_addr.sin_port = htons(8888);       // 网络字节序端口号
    server_addr.sin_addr.s_addr = INADDR_ANY; // 本地任意IP地址，每个网卡对应一个

    // inet_pton(AF_INET,"192.168.233.101",&server_addr.sin_addr.s_addr);   //指定IP地址

    // todo 指定端口复用
    int opt = 1;

    //! 绑定监听的套接字    socket = IP:port
    int ret = bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret == -1)
    {
        perror("listening socket bind failed...\n");
        exit(0);
    }

    //! 设置监听，启动监听端口
    ret = listen(sock, 128); // 同时处理的最大连接数，最大值为128
    if (ret == -1)
    {
        perror("listening socket listen failed...\n");
        exit(0);
    }

    printf("listening socket is ready to accept!\n");

    int done = 1;

    struct sockaddr_in client;
    socklen_t client_addr_len = sizeof(client);
    int client_sock, len, i;
    char client_ip[64] = {0};
    char buf[1024];

    //! 从sock的读缓冲队列中 接收连接请求，若空则阻塞
    client_sock = accept(sock, (struct sockaddr *)&client, &client_addr_len);

    printf("accept succeed, Client ip : %s\t port: %d\n",
           inet_ntop(AF_INET, &client.sin_addr.s_addr, client_ip, sizeof(client_ip)),
           ntohs(client.sin_port)); // 打印客户端IP和端口
    while (done)
    {
        memset(buf, 0, sizeof(buf));
        len = read(client_sock, buf, sizeof(buf) - 1); //读取客户端的读缓冲区数据
        // len = 0 ：对方断开连接；> 0 ：实际接受字符数；< 0 ：数据接收失败
        if (len <= 0)
        {
            perror("receive failed...\n");
            exit(0);
        }
        buf[len] = '\0'; // 字符串最后为 \0 ，防止字符乱码
        printf("receive succeed:size=%d\t data=%s\n", len, buf);

        len = write(client_sock, buf, len);
        printf("write succed:size=%d\n", len);
    }
    close(client_sock);
    close(sock);
    return 0;
}