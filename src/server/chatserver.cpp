#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"
#include<string>
#include<functional>
using namespace std;
using namespace placeholders;
using json = nlohmann::json;

 ChatServer::ChatServer(EventLoop * loop,
                        const InetAddress &listenAddr,
                        const string &nameArg)
        :_server(loop,listenAddr,nameArg),_loop(loop)
{
    //注册连接回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this,_1));
    //注册消息回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3));

    //设置线程数
    _server.setThreadNum(8);
    

}
//启动服务
void ChatServer::start(){
    _server.start();
}
//上报连接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    //客户端下线
    if(!conn->connected())
    {
        ChatService :: instance()->ClientCloseException(conn);
        conn->shutdown();
    }

}
//上报读写事件的相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                        Buffer *buffer,
                        Timestamp time)
 {
    string buf = buffer -> retrieveAllAsString();
    //进行数据的解码  反序列化
    json js = json::parse(buf);
    //目的就是 完全解耦网络模块的代码和业务模块的代码
    //通过js["msgid"] 获取 => 业务handler  => conn  js  time
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    //回调 消息对应的绑定好的事件处理器，来执行相应的业务处理
    msgHandler(conn,js,time);

 } 
