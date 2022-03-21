#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <arpa/inet.h>
#include <event2/listener.h>
#include <sys/types.h>

//! 读缓冲区回调，把都缓冲区数据读取出来，由内核态读取到用户态，并且处理数据
void read_cb(struct bufferevent *bev, void *arg)
{
    char buf[1024] = {0};

    // 从事件读缓冲区中读取数据
    bufferevent_read(bev, buf, sizeof(buf));

    printf("client say: %s\n", buf);

    char *p = "server recvieved successfully";
    // 发数据给客户端,0成功,-1失败

    bufferevent_write(bev, p, strlen(p) + 1);
    sleep(1);
}

//! 写回调很鸡肋，因为数据写过去内核会自动发送出去，写完后回调可以检测是否发送成功
void write_cb(struct bufferevent *bev, void *arg)
{
    printf("内核已写入,用户态回调函数执行\n");
}

void event_cb(struct bufferevent *bev, short events, void *arg)
{
    if (events & BEV_EVENT_EOF)
    {
        printf("connection closed\n");
    }
    else if (events & BEV_EVENT_ERROR)
    {
        printf("event callback other error\n");
    }
    //! 释放通信的事件，出现某事件异常或正常结束断开
    bufferevent_free(bev);
    // bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    printf("bufferevent资源已被释放 BEV_OPT_CLOSE_ON_FREE");
}

// 用于监听新连接的回调函数
void cb_listener(
    struct evconnlistener *listener,
    evutil_socket_t fd,
    struct sockaddr *addr,
    int len,
    void *ptr)
{
    printf("connect new client\n");

    struct event_base *base = (struct event_base *)ptr;

    //! 来了新通信socket连接，创建添加新的事件到event_base 上
    struct bufferevent *bev;
    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

    //! bufferevent_setcb 给通信事件设置回调函数
    bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL);

    //! 设置 读缓冲使能，默认 读disable、写enable
    bufferevent_enable(bev, EV_READ);
    bufferevent_enable(bev, EV_WRITE);
}

int main(int argc, const char *argv[])
{
    struct event_base *base;
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8888);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 创建 event_base 底座
    base = event_base_new();

    // 创建专门用于监听连接的事件
    struct evconnlistener *listener;
    listener = evconnlistener_new_bind(base, cb_listener, base,
                                       LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
                                       (struct sockaddr *)&server_addr, sizeof(server_addr));

    // dispatch 检测事件，并派发相应回调
    event_base_dispatch(base);

    // 销毁用于监听的事件
    evconnlistener_free(listener);

    // 销毁 event_base
    event_base_free(base);

    return 0;
}