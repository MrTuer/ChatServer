#include "chatservice.hpp"
#include "public.hpp"
#include "friendmodel.hpp"
#include "user.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <muduo/base/Logging.h>
using namespace muduo;
using namespace std;
// 获取单例对象的接口函数
ChatService *ChatService::instance()
{

    static ChatService service;
    return &service;
}
// 注册消息对对应的Handler回调操作
ChatService::ChatService()
{
    // 是业务的核心 oop操作 都有这样的概念思想。
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});
    // 群组业务 回调声明
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});
    //连接redis服务器
    if(_redis.connect())
    {
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubsercribeMessage,this,_1,_2));
    }
}

// 获取消息对应的一个处理类型
MsgHandler ChatService::getHandler(int msgid)
{
    // 记录错误日志 msgid 没有对应的事件处理器回调
    auto it = _msgHandlerMap.find(msgid);
    // 不使用中括号的原因是为了避免原先没有 由于中括号查询新建了一对的情况。
    if (it == _msgHandlerMap.end())
    {
        // 返回一个默认处理器
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time)
        {
            // 空操作 输出一下 值捕获就行
            LOG_ERROR << "msgid:" << msgid << " can not find handler!";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}

// 登录业务 id password stste
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    string pwd = js["password"];
    User user = _usermodel.query(id);

    if (user.getId() == id && user.getPwd() == pwd)
    {
        if (user.getState() == "online")
        {
            // 已经登陆
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "用户已经登陆";
            conn->send(response.dump());
        }
        else
        {
            // 记录用户在线的连接 准备发送信息
            // STL 线程不安全 需要自己加个互斥锁
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnectMap.insert({id, conn});
                // lock_guard<mutex> unlock(_connMutex);
            }

            //id用户登录完成后 向redis订阅channnel(id)
            _redis.subscribe(id);

            // 登录成功 更新用户状态信息 state offline => online
            user.setState("online");
            _usermodel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            // 查询用户是否由离线信息
            vector<string> vec = _offlineMsgModel.query(id);
            if (!vec.empty())
            {
                // 直接使用json库对STL容器进行序列化
                response["offlinemsg"] = vec;
                // 读取离线消息后 把该用户的所有离线消息消除
                _offlineMsgModel.remove(id);
            }

            // 查询用户的好友信息并返回
            vector<User> userVc = _friendmodel.query(id);
            if (!userVc.empty())
            {
                vector<string> vec2;
                for (User &user : userVc)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            // 查询用户群组信息并返回
            vector<Group> groupuserVec = _groupModel.queryGroup(id);
            if (!groupuserVec.empty())
            {
                vector<string> groupV;
                for (Group &group : groupuserVec)
                {
                    json grpjson;
                    grpjson["id"] = group.getId();
                    grpjson["groupname"] = group.getName();
                    grpjson["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for (GroupUser &guser : group.getUsers())
                    {
                        json js;
                        js["id"] = guser.getId();
                        js["name"] = guser.getName();
                        js["state"] = guser.getState();
                        js["role"] = guser.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;
                    groupV.push_back(grpjson.dump());
                }
                response["groups"] = groupV;
            }

            conn->send(response.dump());
        }
    }
    else
    {
        // 用户不存在
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户名或密码错误";
        conn->send(response.dump());
    }
}
// 注册业务 name password
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);

    bool state = _usermodel.insert(user);
    if (state)
    {
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        // 注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

void ChatService::reset()
{
    // 重置用户信息
    _usermodel.resetState();
}

// 客户端注销处理处理
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int user_id = js["id"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnectMap.find(user_id);
        if (it != _userConnectMap.end())
        {
            _userConnectMap.erase(it);
        }
    }
    User user(user_id, "", "", "offline");
    // 更新用户状态 
    _usermodel.updateState(user);
    //跳回首页面
}
// 客户端异常退出的处理
void ChatService::ClientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(_connMutex);

        for (auto it = _userConnectMap.begin(); it != _userConnectMap.end(); ++it)
        {
            if (it->second == conn)
            {
                // 从 map 删除用户的链表信息
                user.setId(it->first);

                _userConnectMap.erase(it);
                break;
            }
        }
    }

    //用户注销 相当于下线 从redis中取消订阅通道
    _redis.unsubscribe(user.getId());

    if (user.getId() != -1)
    {
        // 更新用户状态
        user.setState("offline");
        _usermodel.updateState(user);
    }
}

// 一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["to"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnectMap.find(toid);

        if (it != _userConnectMap.end())
        {
            // toid 在线 直接转发信息 服务求主动推送信息给 目标id用户

            it->second->send(js.dump());
            return;
        }
    }
    //查看对方是否在线 如果在线 证明不是有一个服务器，但在线，需要进行跨服务器通信。
    User user = _usermodel.query(toid);
    if(user.getState() == "online")
    {
        _redis.publish(toid,js.dump());
        return;
    }
    // toid 不在线 存储到离线消息队列中。
    _offlineMsgModel.insert(toid, js.dump());
}

// 添加好友功能
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    // 存储好友
    _friendmodel.insert(userid, friendid);
}

// 创建群
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];
    // 存储新创建的群的信息
    Group group(-1, name, desc);
    if (_groupModel.createGroup(group))
    {
        _groupModel.addGroup(userid, group.getId(), "Creator");
    }
}

// 加入群
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "Normal");
}

// //群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);

    lock_guard<mutex> lock(_connMutex);
    for (int id : useridVec)
    {
        auto it = _userConnectMap.find(id);
        if (it != _userConnectMap.end())
        {
            // 转发群消息
            it->second->send(js.dump());
        }
        else
        {
            
         // 查询toid是否在线
            User user = _usermodel.query(id);
            if (user.getState() == "online")
            {
                _redis.publish(id, js.dump());
            }
            else
            {
            // 存储离线群消息
            _offlineMsgModel.insert(id, js.dump());
            }
        }
    }
}

//从redis消息队列中获取订阅的消息
void ChatService::handleRedisSubsercribeMessage(int userid,string msg)
{   
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnectMap.find(userid);
    if(it != _userConnectMap.end())
    {
        it ->second->send(msg);
        return;
    }

    //存储离线信息
    _offlineMsgModel.insert(userid,msg);

}