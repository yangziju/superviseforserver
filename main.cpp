#include "common.h"
#include "sock.h"

#include <tr1/unordered_map>
using namespace std::tr1;
using namespace std;
using std::string;

const int MAX_BUF_SIZE = 1024;

void print_addr(struct sockaddr_in &cliaddr)
{
    printf("received from %s at PORT %d\n",
           inet_ntoa(cliaddr.sin_addr),
           ntohs(cliaddr.sin_port));
}

void delete_fd(int *client, int &sockfd, int &efd)
{
    for (int j = 0; j < OPEN_MAX; j++)
    {
        if (client[j] == sockfd)
        {
            client[j] = -1;
            break;
        }
    }
    int res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
    if (res == -1)
        perr_exit("epoll_ctl");

    Close(sockfd);
    printf("client[%d] closed connection\n", sockfd);
}

void do_listen_fd(int *client, int &connfd, struct epoll_event &tep, int &efd)
{
    int j;
    for (j = 0; j < OPEN_MAX; j++)
    {
        if (client[j] < 0)
        {
            client[j] = connfd; /* save descriptor */
            break;
        }
    }

    if (j == OPEN_MAX)
        cout << "warning: too many clients" << endl;

    tep.events = EPOLLIN;
    tep.data.fd = connfd;
    int res = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep);
    if (res == -1)
        perr_exit("epoll_ctl");
}

void send_data(int *client, char *buf, int &sockfd)
{
    int recv_len = ((head_t *)buf)->pack_size;
    for (int j = 0; j < OPEN_MAX; j++)
    {
        if (client[j] >= 0 /*&& client[j] != sockfd*/)
        {
            Writen(client[j], buf, recv_len + 1);
        }
    }
}

int main(int argc, const char *argv[])
{
    int i, j, listenfd, connfd, sockfd;
    int nready, efd, res;
    ssize_t n;
    char buf[MAX_BUF_SIZE];
    socklen_t clilen;
    int client[OPEN_MAX];
    struct sockaddr_in cliaddr, servaddr;
    struct epoll_event tep, ep[OPEN_MAX];
    clilen = sizeof(cliaddr);
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    Listen(listenfd, 50);

    for (i = 0; i < OPEN_MAX; i++)
        client[i] = -1;

    efd = epoll_create(OPEN_MAX);
    if (efd == -1)
        perr_exit("epoll_create");

    tep.events = EPOLLIN;
    tep.data.fd = listenfd;

    res = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tep);
    if (res == -1)
        perr_exit("epoll_ctl");

    while (1)
    {
        nready = epoll_wait(efd, ep, OPEN_MAX, -1); /* 阻塞监听 */
        if (nready == -1)
            perr_exit("epoll_wait");

        for (i = 0; i < nready; i++)
        {
            if (!(ep[i].events & EPOLLIN))
                continue;

            if (ep[i].data.fd == listenfd)
            {
                connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);

                print_addr(cliaddr);

                do_listen_fd(client, connfd, tep, efd);
            }
            else
            {
                cout << "read event arrived" << endl;
                sockfd = ep[i].data.fd;
                n = Read(sockfd, buf, MAX_BUF_SIZE);
                if (n == 0)
                    delete_fd(client, sockfd, efd);
                else
                    send_data(client, buf, sockfd); // 群发
            }
        }
    }

    close(listenfd);
    close(efd);
    return 0;
}