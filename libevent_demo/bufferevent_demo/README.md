# 单Reactor单线程模型  

- Reactor监听和分发事件 ——`event_base `
- Acceptor获取链接 —— `evconnlistener`
- Handler处理业务 —— `bufferevent`



以上是 **单Reactor单线程模型** 和实际实现流程的对应关系



实际处理流程为：

- 建立 `event_base`，即 一个Reactor对象：

  ```c
  struct event_base *base;
  base = event_base_new();
  ```

- 建立 `evconnlistener`，即 一个Acceptor对象，专门用于处理新的客户端通信socket请求，处理连接，返回客户端的 sockid

  ```c
  struct evconnlistener *listener;
  listener = evconnlistener_new_bind(base, listener_cb, base,
                                         LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
                                         (struct sockadrr *)&server_addr, sizeof(server_addr));
  ```

  参数中的 `listener_cb` 是这个 Acceptor 的回调函数，当有新连接请求时，就会调用该函数，创建 `bufferevent` 事件来处理对应的请求和后序与该客户端的通信：

  - 在 Acceptor 检测到新的连接后，会将该连接的 sockid 和 地址端口信息等传递给这个回调函数，回调函数会将这些信息绑定到新创建的 `bufferevent` 事件上，即一个Handler对象：

    ```c
    struct bufferevent *bev;
        bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    ```

    这个Handler，即 `bufferevent` 负责:

    	 1. 内核和用户态的数据交互
    	 1. 对数据的业务处理（当然这个另开一个新线程来解决，只是本文所讲的是单Reactor单线程模型）

  - 并给这个事件设置相对应的回调函数，包括读写异常处理等

    ```c
    bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL);
    ```

  - 设置 读缓冲区使能，因为新创建的 `bufferevent` 默认读缓冲区 disable，写缓冲区 enable

    ```c
    bufferevent_enable(bev, EV_READ);
    ```

  - 在正常关闭或出现异常时，关闭连接删除`bufferevent` 事件释放资源，这些处理都在上述设置回调函数中 `event_cb` 这个回调函数内实现

    ```c
    bufferevent_free(bev);
    ```

- event_base开始检测并派发事件

  ```c
  // dispatch 检测事件，并派发相应回调
  event_base_dispatch(base);
  ```

- 释放资源

  ```c
  // 销毁用于监听的事件
  evconnlistener_free(listener);
  // 销毁 event_base
  event_base_free(base);
  ```

  

# bufferevent 相关函数



## event_base 相关

### 创建和释放

```c
/**
 * Create and return a new event_base to use with the rest of Libevent.
 *
 * @return a new event_base on success, or NULL on failure.
 *
 * @see event_base_free(), event_base_new_with_config()
 */
struct event_base *event_base_new(void);


/**
  Deallocate all memory associated with an event_base, and free the base.

  Note that this function will not close any fds or free any memory passed
  to event_new as the argument to callback.

  If there are any pending finalizer callbacks, this function will invoke
  them.

  @param eb an event_base to be freed
 */
void event_base_free(struct event_base *);
```



## evconnlistener 相关



### 创建

```c
/**
   Allocate a new evconnlistener object to listen for incoming TCP connections
   on a given file descriptor.

   @param base The event base to associate the listener with.
   @param cb A callback to be invoked when a new connection arrives.  If the
      callback is NULL, the listener will be treated as disabled until the
      callback is set.
   @param ptr A user-supplied pointer to give to the callback.
   @param flags Any number of LEV_OPT_* flags
   @param backlog Passed to the listen() call to determine the length of the
      acceptable connection backlog.  Set to -1 for a reasonable default.
      Set to 0 if the socket is already listening.
   @param fd The file descriptor to listen on.  It must be a nonblocking
      file descriptor, and it should already be bound to an appropriate
      port and address.
*/
struct evconnlistener *evconnlistener_new(struct event_base *base,
    evconnlistener_cb cb, void *ptr, unsigned flags, int backlog,
    evutil_socket_t fd);
```



### 绑定

```c
/**
   Allocate a new evconnlistener object to listen for incoming TCP connections
   on a given address.

   @param base The event base to associate the listener with.
   @param cb A callback to be invoked when a new connection arrives. If the
      callback is NULL, the listener will be treated as disabled until the
      callback is set.
   @param ptr A user-supplied pointer to give to the callback.
   @param flags Any number of LEV_OPT_* flags
   @param backlog Passed to the listen() call to determine the length of the
      acceptable connection backlog.  Set to -1 for a reasonable default.
   @param addr The address to listen for connections on.
   @param socklen The length of the address.
 */
struct evconnlistener *evconnlistener_new_bind(struct event_base *base,
    evconnlistener_cb cb, void *ptr, unsigned flags, int backlog,
    const struct sockaddr *sa, int socklen);
```

- 这个过程把 socket 通信流程中 `socket()`、`bind()`、`listen()` 都一次性实现了

- 参数 `flags`  最常用的是 `LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE` ，就是断开释放资源，针对2MSL设置端口复用，完整参数如下：

```c
/** Flag: Indicates that we should not make incoming sockets nonblocking
 * before passing them to the callback. */
#define LEV_OPT_LEAVE_SOCKETS_BLOCKING	(1u<<0)
/** Flag: Indicates that freeing the listener should close the underlying
 * socket. */
#define LEV_OPT_CLOSE_ON_FREE		(1u<<1)
/** Flag: Indicates that we should set the close-on-exec flag, if possible */
#define LEV_OPT_CLOSE_ON_EXEC		(1u<<2)
/** Flag: Indicates that we should disable the timeout (if any) between when
 * this socket is closed and when we can listen again on the same port. */
#define LEV_OPT_REUSEABLE		(1u<<3)
/** Flag: Indicates that the listener should be locked so it's safe to use
 * from multiple threadcs at once. */
#define LEV_OPT_THREADSAFE		(1u<<4)
/** Flag: Indicates that the listener should be created in disabled
 * state. Use evconnlistener_enable() to enable it later. */
#define LEV_OPT_DISABLED		(1u<<5)
/** Flag: Indicates that the listener should defer accept() until data is
 * available, if possible.  Ignored on platforms that do not support this.
 *
 * This option can help performance for protocols where the client transmits
 * immediately after connecting.  Do not use this option if your protocol
 * _doesn't_ start out with the client transmitting data, since in that case
 * this option will sometimes cause the kernel to never tell you about the
 * connection.
 *
 * This option is only supported by evconnlistener_new_bind(): it can't
 * work with evconnlistener_new_fd(), since the listener needs to be told
 * to use the option before it is actually bound.
 */
#define LEV_OPT_DEFERRED_ACCEPT		(1u<<6)
/** Flag: Indicates that we ask to allow multiple servers (processes or
 * threads) to bind to the same port if they each set the option. 
 * 
 * SO_REUSEPORT is what most people would expect SO_REUSEADDR to be, however
 * SO_REUSEPORT does not imply SO_REUSEADDR.
 *
 * This is only available on Linux and kernel 3.9+
 */
#define LEV_OPT_REUSEABLE_PORT		(1u<<7)
```



### 释放

```c
/**
   Disable and deallocate an evconnlistener.
 */
void evconnlistener_free(struct evconnlistener *lev);
```



## 检测以及派发事件

```c
/**
   Event dispatching loop

  This loop will run the event base until either there are no more pending or
  active, or until something calls event_base_loopbreak() or
  event_base_loopexit().

  @param base the event_base structure returned by event_base_new() or
     event_base_new_with_config()
  @return 0 if successful, -1 if an error occurred, or 1 if we exited because
     no events were pending or active.
  @see event_base_loop()
 */
int event_base_dispatch(struct event_base *);
```

