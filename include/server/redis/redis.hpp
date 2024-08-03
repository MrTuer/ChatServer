#ifndef REDIS_H
#define REDIS_H

#include<hiredis/hiredis.h>
#include<thread>
#include<functional>
using namespace std;

class Redis
{
public:
    Redis();
    ~Redis();

    //连接Redis服务器
    bool connect();
    //向指定的redis的通道 channle发布信息
    bool publish(int channel,string message);
    //向指定的redis的通道subscribe订阅消息
    bool subscribe(int channel);
    //向指定的redis的通道subscribe取消订阅消息
    bool unsubscribe(int channnel);
    //在独立的线程中接收订阅通道中的信息
    void observer_channel_message();
    //初始化业务层上报通道消息的回调对象
    void init_notify_handler(function<void(int,string)> fn);
private:
    //hiredis 同步上下文对象 负责 publish消息
    redisContext *_publish_context;
    //hiredis 同步上下文对象 负责 subscribe消息   
    redisContext *_subscribe_context;    
    
    //回调操作，收到订阅的消息，给server层上报
    function<void(int,string)> _notify_message_handler;
};

#endif