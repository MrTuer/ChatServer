#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.hpp"
#include<string>
#include<vector>
using namespace std;

class GroupModel
{
public:
    //创建群组
    bool createGroup(Group &group);
    //加入群组 
    void addGroup(int userid,int groupid,string role);
    //查询用户所在群的信息
    vector<Group> queryGroup(int userid);
    // 根据指定的groupid查询群组用户id列表，主要用户群聊业务群组其他成员群发信息。
    vector<int> queryGroupUsers(int userid,int groupid);
};


#endif