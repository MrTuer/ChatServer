#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>
using namespace std;
using namespace muduo;
using namespace muduo::net;
#include "usermodel.hpp" 
#include "friendmodel.hpp" 
#include "redis/redis.hpp"

#include "groupmodel.hpp"
#include "offlinemessagemodel.hpp"
#include "json.hpp"
using json = nlohmann::json;
// 消息id对应的事件回调  方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;
// 使用单例模式
//
//
//
// 聊天服务器业务类
class ChatService
{
public:
    // 获取单例对象的接口函数
    static ChatService *instance();
    // 登录业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 注册业务
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 一对一发送信息业务
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 添加好友功能
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //客户端异常退出处理
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);


    //群有业务
    //创建群
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //加入群
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //群组聊天
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 获取消息对应的一个处理类型
    MsgHandler getHandler(int msgid);
    // 客户端异常退出的处理
    void ClientCloseException(const TcpConnectionPtr &conn);
    // 服务器有异常退出的重置方法
    void reset();
 
    //从redis消息队列中获取订阅的消息
    void handleRedisSubsercribeMessage(int,string);

private:
    ChatService();
    // 存储消息id 和 其对应的业务处理方法。
    unordered_map<int, MsgHandler> _msgHandlerMap; // 消息id对应的操作。

    // 存储在线用户的通信连接和id
    unordered_map<int, TcpConnectionPtr> _userConnectMap;
    // 定义互斥锁 保证线程安全
    mutex _connMutex;
    // 数据操作类对象
    UserModel _usermodel;
    OfflineMessageModel _offlineMsgModel;
    FriendModel _friendmodel;
    GroupModel _groupModel;
    
    
    //redis
    Redis _redis;

};

#endif 