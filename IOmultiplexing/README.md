# select
## select流程
默认是服务端，IO复用 `IO multiplexing`
- 1 首先创建socket，用于监听的socket的文件标识符 `lfd`
- 2 绑定 bind() 服务器IP地址以及端口，可以设置 ？？？
- 3 设置监听 listen()
- 4 用 select() 将 `lfd` 检查其读缓冲区，若有客户端请求，下一步
- 5 accept() ，再次判断 `lfd` ，然后创建通信socket文件标识符 `cfd` ，将用select()检查 `cfd` 的读写缓冲区，然后处理通信 
- 6 重复 4

 :o:  对于 2 的补充，可以设置 端口复用，避免 TCP 提前结束的一方（服务器），在最后等待 2MSL（通常是2分钟）时直接关闭程序，然后重新打开程序，发现 bind() 报错，端口已被使用，此时还在 `TIME_WAIT` 状态。

 ```cpp
// 设置端口复用
int opt = 1;
setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
```


## select缺点

- 默认支持文件描述符数量太小，1024，虽然可以调整，但是没必要，完全可以用 `epoll`
- 每次需要把 fdset 位图信息从用户态拷贝到内核，再从内核传出拷贝到用户态，耗时
- 可能大部时间 fdset 数据稀疏，还要一直遍历 fdset，这样查找，使得整体效率降低
- 只能轮询来检测通信socket的文件描述符，可以进行代码方面的优化，用数组来存储 accept() 后返回的 csock，然后下面比较的时候，不从read_set（即 fd_set 读缓冲集合）轮询，而是从设定上限可变的数组中轮询判断当然这只适用于 连接请求数稀疏的情况

# poll

- poll 与 select 基本无异
- poll采用 结构数组，select采用位图
- poll没有最大文件描述符的限制
- poll 只支持 linux 系统，select支持linux,windows,mac 操作系统

# epoll

**select和poll的问题就是 fdset拷贝和轮询**
 
而epoll解决了这个问题，极大的提高了效率

**关于epoll ET模式的一些细节**

- 因为边缘模式触发这种机制，需要 循环 read()/recv() 读取缓冲区内容，当然另外一种方法就是加大一次读的容量（并不可取），那么当缓冲区读完清空时，默认是阻塞的，所以要设置该缓冲区文件为非阻塞模式

```cpp
//! 边沿触发模式 设置 文件属性非阻塞
int flag = fcntl(csock, F_GETFL);
flag = flag | O_NONBLOCK;
fcntl(csock, F_SETFL, flag);
//! 设置通信socket为边沿触发模式
ev.events = EPOLLIN | EPOLLET;
```

- 还是因为上述原因，在检查 read()/recv() 返回值时，要判断值为 -1 出错的情况利用 errno 这个全局变量来判断，是正常读完清空，还是出现异常

```cpp
else if (len == -1)
{
    //! 设置非阻塞后，有两种情况了，一是数据读完不可读了，二是出错
    if (errno = EAGAIN)
    {
        printf("数据已读完\n");
        break;
    }
    else
    {
        perror("read\n");
        exit(0);
    }
}
```
