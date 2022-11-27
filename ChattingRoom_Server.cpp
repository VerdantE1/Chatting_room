/***************************************************************************
聊天室服务器
功能：1.从对端接收用户数据并维护
功能：2.广播用户发送的报文
限制5个用户连接
by Frankie

****************************************************************************/
#define _GNU_SOURCE 1
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <poll.h>

#define USER_LIMIT 5
#define BUFFER_SIZE 300
#define FD_LIMIT 65535

struct client_data
{
    sockaddr_in address;
    char* write_buf;
    char buf[BUFFER_SIZE];
};

int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}



int main(int argc, char* argv[])
{
    if (argc <= 2)
    {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);




    //初始化环境
    client_data* users = new client_data[FD_LIMIT];
    pollfd fds[USER_LIMIT + 1];
    int user_counts = 0;
    fds[0].fd = listenfd, fds[0].events = POLLIN | POLLERR, fds[0].revents = 0;
    for (int i = 1;i <= USER_LIMIT;i++)
    {
        fds[i].fd = -1;
        fds[i].events = POLLIN | POLLERR;
        fds[i].revents = 0;
    }


    //事件循环
    while (1)
    {
        ret = poll(fds, user_counts + 1, -1);
        if (ret < 0)
        {
            printf("POLL FAILED!\n");
            break;
        }

        for (int i = 0;i < user_counts + 1;i++)
        {
            //监听到lfd上的连接事件
            if ((fds[i].fd == listenfd) && (fds[i].revents & POLLIN))
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
                if (connfd < 0)
                {
                    printf("errno is: %d\n", errno);
                    continue;
                }
                if (user_counts >= USER_LIMIT)
                {
                    //printf("Refuse the %d Connection\n",connfd);
                    send(connfd, "Sorry,The Person is full\n", 30, 0);
                    close(connfd);
                    continue;
                }
                user_counts++;
                users[connfd].address = client_address;
                setnonblocking(connfd);
                fds[user_counts].fd = connfd;
                fds[user_counts].events = POLLIN | POLLERR | POLLRDHUP;
                fds[user_counts].revents = 0;
                for (int j = 1;j < user_counts;j++) { send(fds[j].fd, "Coming a new Person\n", 30, 0); } //向其他人说明
                send(connfd, "Welcome to my chatting room!\n\n\nYou can say what ever you want!\n", 80, 0);
            }

            //出现错误事件
            else if (fds[i].revents & POLLERR)
            {
                printf("get an error from %d\n", fds[i].fd);
                char errors[100];
                memset(errors, '\0', 100);
                socklen_t length = sizeof(errors);
                if (getsockopt(fds[i].fd, SOL_SOCKET, SO_ERROR, &errors, &length) < 0)
                {
                    printf("get socket option failed\n");
                }
                continue;
            }


            else if (fds[i].revents & POLLRDHUP)
            {
                users[fds[i].fd] = users[fds[user_counts].fd]; //将用户信息向前移,下个连接进来user_counts处会被覆盖
                close(fds[i].fd);
                fds[i] = fds[user_counts]; //i被删除，现在将最后面的用户数据移到这里，所以要重新在处理一次，故将i--
                i--; user_counts--;
                printf("A Client Left\n");
            }

            else if (fds[i].revents & POLLIN)
            {
                int connfd = fds[i].fd;
                memset(users[connfd].buf, '\0', BUFFER_SIZE);
                ret = recv(connfd, users[connfd].buf, BUFFER_SIZE - 1, 0);
                //printf("Get %d bytes of client data %s from %d\n", ret, users[connfd].buf, connfd);
                //读时出现错误
                if (ret < 0)
                {
                    if (errno != EAGAIN)
                    {
                        close(connfd);
                        users[fds[i].fd] = users[fds[user_counts].fd];
                        fds[i] = fds[user_counts];
                        i--;user_counts--;
                    }
                }
                else {
                    for (int j = 1;j <= user_counts;j++)
                    {
                        if (fds[j].fd == connfd) continue;
                        fds[j].events &= ~POLLIN;     //准备下次写事件，同时防止逻辑混乱，暂时~POLLIN
                        fds[j].events |= POLLOUT;
                        users[fds[j].fd].write_buf = users[connfd].buf; //要通知每个人，置指针指向发送者的缓冲区
                    }
                }

            }

            else if (fds[i].revents & POLLOUT)
            {
                int connfd = fds[i].fd;
                if (!users[connfd].write_buf)
                {
                    continue;
                }
                ret = send(connfd, users[connfd].write_buf, strlen(users[connfd].write_buf), 0);
                users[connfd].write_buf = NULL;
                fds[i].events |= ~POLLOUT;
                fds[i].events |= POLLIN;
            }

        }

    }
    delete[] users;
    close(listenfd);
    return 0;



}