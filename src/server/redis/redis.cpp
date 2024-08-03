
#include "redis/redis.hpp"
#include<iostream>
using namespace std;

Redis::Redis():_publish_context(nullptr),_subscribe_context(nullptr)
{

}
Redis::~Redis()
{
    if(_publish_context != nullptr)
    {
        redisFree(_publish_context);
    }
    if(_subscribe_context != nullptr)
    {
        redisFree(_subscribe_context);
    }
}

bool Redis::connect()
{
    //负责publish 订阅消息的上下文连接
    _publish_context = redisConnect("127.0.0.1",6379);
    if(nullptr == _publish_context)
    {
        cerr<<"connect redis failed!"<<endl;
        return false;
    }
    //负责subscribe 订阅消息的上下文连接
    _subscribe_context = redisConnect("127.0.0.1",6379);
    if(nullptr == _subscribe_context)
    {
        cerr<<"connect redis failed!"<<endl;
        return false;
    }
    //在单独 的 线程中监听通道上的事件  有有有有消息给业务层上报
    thread t([&](){
        observer_channel_message();
    });
    t.detach();
    cout<<"connect redis-server success!"<<endl;

    return true;
}

//向redis指定的 通道channel中发布消息
bool Redis::publish(int channel,string message)
{
    redisReply *reply = (redisReply *)redisCommand(_publish_context,"PUBLISH %d %s",channel,message.c_str());
    if(reply == nullptr)
    {
        cerr<<"Publish command faailed!"<<endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}
//向subscribe指定的 通道channel中订阅消息
bool Redis::subscribe(int channel)
{
    if(REDIS_ERR == redisAppendCommand(this->_subscribe_context,"SUBSCRIBE %d",channel));
    {
        cerr<<"subscribe command failed!"<<endl;
    }
    //redisBufferWrite 乐意循环发送缓冲区，直到缓冲区数据发送完毕（done被设置为1）
    int done = 0;
    while(!done)
    {
        if(REDIS_ERR == redisBufferWrite(this->_subscribe_context,&done))
        {
            cerr<<"sunscribe command failed!"<<endl;
            return false;
        }
    } 
    return true;
}
//向subscribe指定的 通道channel中取消订阅消息
bool Redis::unsubscribe(int channel)
{
    if(REDIS_ERR == redisAppendCommand(this->_subscribe_context,"UNSUBSCRIBE %d",channel))
    {
        cerr<<"unsubscribe command error!"<<endl;
        return false;
    }
    int done = 0;
    while(!done)
    {
        if(REDIS_ERR == redisBufferWrite(this->_subscribe_context,&done))
        {
            cerr<<"unsubscribe command failed !"<<endl;
            return false;
        }
    }
    return true;

}

//在独立的线程中订阅通道的消息
void Redis::observer_channel_message()
{
    redisReply *reply = nullptr;
    while(REDIS_OK ==redisGetReply(this->_subscribe_context,(void**)&reply))
    {
        //订阅收到的消息十一一个带三元素的数组
        if(reply != nullptr && reply ->element[2]!=nullptr &&reply->element[2]->str !=nullptr)
        {
            //给业务层上报通道上发送的消息
            _notify_message_handler(atoi(reply->element[1]->str),reply->element[2]->str);
        }
        freeReplyObject(reply);
    }
    cerr<<">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  obser _channel_message quit  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"<<endl;
}

void Redis::init_notify_handler(function<void(int,string)>fn)
{
    this -> _notify_message_handler = fn;
}