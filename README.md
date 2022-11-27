# Chatting_room
Linux_tiny_Chatting_room/聊天室程序

进入聊天室：
![image](https://user-images.githubusercontent.com/78347791/204136070-fb4d3ccc-d0c1-4de5-b24d-5756877e6316.png)


基于linux平台制作的简单聊天室程序，可以通过修改宏定义USER_LIMIT进而修改支持的用户个数

不涉及任何外部库，直接g++编译即可

涉及到:socket编程，零拷贝文件描述符数据splice，IO多路复用。


对于本服务器程序，有很多改善的地方。比如我们可以开多进程或多线程将核利用起来实现负载均衡，其次，可以关注到服务器读到的数据是可以不用处理的，我们可以用与客户端同样的方式，利用splice进行零拷贝将数据发送到对端，还可以利用shmget同样可以优化性能。
