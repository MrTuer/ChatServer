#include "chatserver.hpp"
#include "chatservice.hpp"
#include<iostream>
#include<signal.h>
using namespace std;

void reseHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "command invalid! example: ./ChatServer 127.0.0.1 6000" << endl;
        exit(-1);
    }
    //处理客户端 ctrl + c 退出后 user的状态信息。
    char *ip = argv[1];            // 记录IP地址
    uint16_t port = atoi(argv[2]); // 记录端口

    signal(SIGINT,reseHandler);
    EventLoop loop;
    InetAddress addr(ip,port);
    ChatServer server(&loop,addr,"ChatServer");

    server.start();
    loop.loop();

    return 0;
}
