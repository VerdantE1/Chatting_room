/***************************************************************************
聊天室客户端
功能：1.从标准输入接收数据发送到服务器
功能：2.打印从服务器收到的数据
功能：3.超出一定时间没连上就退出
by Frankie

****************************************************************************/
#include<stdio.h>
#include<sys/socket.h>
#include<time.h>
#include<strings.h>
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <poll.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define _GNU_SOURCE 1
#define BUF_SIZE 300

int Timeout_Connect(const char* ip, int port, int time)
{
    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);

    struct timeval timeout;
    timeout.tv_sec = time;
    timeout.tv_usec = 0;
    socklen_t len = sizeof(timeout);
    ret = setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, len);
    assert(ret != -1);

    ret = connect(sockfd, (struct sockaddr*)&address, sizeof(address));
    if (ret == -1)
    {
        if (errno == EINPROGRESS)
        {
            printf("connecting timeout\n");
            return -1;
        }
        printf("Can't connect Server,Please try it agian.\n");
        return -1;
    }

    return sockfd;
}


int main(int argc, char* argv[])
{
    
    argc = 2;
	if (argc <= 2)
	{
		printf("Pls put in two args： 1.IP 2.Port \n");
	}

    //连接服务器
    //int port = atoi(argv[2]);
    //int sockfd = Timeout_Connect(argv[1], port,10);

    int sockfd = Timeout_Connect("192.168.154.128", 12347,10);
    //监听标准输入与服务端
    struct pollfd fds[2];
    memset(fds, 0, sizeof fds);
    fds[0].fd = 0, fds[0].events = POLLIN, fds[0].revents = 0;
    fds[1].fd = sockfd, fds[1].events = POLLIN | POLLRDHUP, fds[1].revents = 0;

    //用户读缓存
    char RDBUF[BUF_SIZE];


    int pipefd[2];
    int ret = pipe(pipefd);
    if (ret < 0)
    {
        perror("PIPE creat:");
    }


    while (1)
    {
        ret = poll(fds, 2, -1);
        if (ret < 0)
        {
            printf("Exception:Poll error\n");
            break;
        }
        if (fds[1].revents & POLLRDHUP)
        {
            printf("Server has closed your connection\n");
            break;
        }
        if (fds[1].revents & POLLIN)
        {
            memset(RDBUF, '\0', sizeof RDBUF);
            recv(sockfd, RDBUF, BUF_SIZE-1,0);
            printf("%s\n", RDBUF);
        }
        if (fds[0].revents & POLLIN)
        {
            splice(0, 0, pipefd[1], NULL, 32768, SPLICE_F_MORE);
            splice(pipefd[0], NULL, sockfd, 0, 32768, SPLICE_F_MORE);
        }

    }
    close(sockfd);


}