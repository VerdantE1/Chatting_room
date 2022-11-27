# Chatting_room
Linux_tiny_Chatting_room/聊天室程序


基于linux平台制作的简单聊天室程序，可以通过修改宏定义USER_LIMIT进而修改支持的用户个数

不涉及任何外部库，直接g++编译即可

涉及到:socket编程，零拷贝文件描述符数据splice，IO多路复用。



进入聊天室：


![image](https://user-images.githubusercontent.com/78347791/204136095-f2ef14c3-93f3-459d-9b31-9d81669a0bf5.png)

当有其他人进入聊天室：

![image](https://user-images.githubusercontent.com/78347791/204136115-c23ba257-d5c1-4188-a764-3e47f23ddb6b.png)


发送信息（没加键盘对应的中断回调，不小心多按了sorry)：

![image](https://user-images.githubusercontent.com/78347791/204136156-29b65f98-6d89-4a29-96ff-c35f0ccf137a.png)

另一个用户接收
![image](https://user-images.githubusercontent.com/78347791/204136205-789160f4-3237-4a69-9e33-5e62d889fe47.png)

当聊天室满了会提示：

![image](https://user-images.githubusercontent.com/78347791/204136281-f0377798-e41c-48b1-bb92-0bb2e8b923c3.png)








对于本服务器程序，有很多改善的地方。比如我们可以开多进程或多线程将核利用起来实现负载均衡，其次，可以关注到服务器读到的数据是可以不用处理的，我们可以用与客户端同样的方式，利用splice进行零拷贝将数据发送到对端，还可以利用shmget同样可以优化性能。
