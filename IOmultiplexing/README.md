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

# poll

- poll 与 select 基本无异
- poll采用 结构数组，select采用位图
- poll没有最大文件描述符的限制
- poll 只支持 linux 系统，select支持linux,windows,mac 操作系统

# epoll

 **select和poll的问题就是 fdset拷贝和轮询**
 
 而epoll解决了这个问题，极大的提高了效率
