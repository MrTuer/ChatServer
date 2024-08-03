#include "groupmodel.hpp"
#include "db.h"  
#include <iostream>
using namespace std;
//创建群组
bool GroupModel::createGroup(Group &group)
{
     // 1.组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "insert into AllGroup(group_name,group_desc) values('%s','%s')",
            group.getName().c_str(), group.getDesc().c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取插入成功 的用户生成的主键id
            group.setId(mysql_insert_id(mysql.getconnection()));
            return true; // 注册成功
        }
    }
    return false;
    // 注册失败
}

//计入群组
void GroupModel::addGroup(int userid,int groupid,string role)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into GroupUser(group_id,user_id,group_role) values(%d,%d,'%s')",
            groupid, userid,role.c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    } 
}
// 查询用户所在群组信息
vector<Group> GroupModel::queryGroup(int userid)
{
    /*
    1. 先根据userid 和 groupuser表最终查询出该用户所属的群组信息
    2. 现根据群组信息，查询属于该群的所偶
    */
   char sql[1024];
   cerr<<"select groups start"<<endl;
   sprintf(sql, "select a.id,a.group_name,a.group_desc from AllGroup a inner join \
         GroupUser b on a.id = b.group_id where b.user_id=%d",
            userid);
   vector<Group> groupVec;
   MySQL mysql;
   if(mysql.connect()){
        MYSQL_RES  *res = mysql.query(sql);
        cerr<<"res1 :"<<res<<endl;
        if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res))!=nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);  
                groupVec.push_back(group);
            }
            mysql_free_result(res);
        }
   }
   for(Group &group :groupVec)
   {
        
   cerr<<"select groups start2"<<endl;
        sprintf(sql,"select a.id,a.name,a.state,b.group_role from User a  \
         inner join GroupUser b on b.user_id = a.id where b.group_id = %d"
         ,group.getId());
        MYSQL_RES *res = mysql.query(sql);
        
   cerr<<"res2 :"<<res<<endl;
         if(res != nullptr)
        {
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res))!=nullptr)
            {
                GroupUser groupUser;
                groupUser.setId(atoi(row[0]));
                groupUser.setName(row[1]);
                groupUser.setState(row[2]); 
                groupUser.setRole(row[3]);  
                group.getUsers().push_back(groupUser);
            }
            mysql_free_result(res);
        }
   }
   
   cerr<<"select groups end"<<endl;
   return groupVec;
}
//根据指定的 群id 查询群组用户id 列表，除 userid 自己 给其他成员群发信息。
vector<int> GroupModel::queryGroupUsers(int userid,int groupid)
{
    char sql[1024] = {0};
    sprintf(sql,"select user_id from GroupUser where group_id =%d and user_id !=%d",groupid,userid);
    vector<int> idVec;
    MySQL mysql ;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row ;
             while((row = mysql_fetch_row(res))!=nullptr)
             {
                idVec.push_back(atoi(row[0]));
             }
             mysql_free_result(res);
        }
    }
    return idVec;
}