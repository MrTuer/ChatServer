#include "offlinemessagemodel.hpp"
#include "db.h"

// 存储用户的离线信息
void OfflineMessageModel::insert(int userid, string msg)
{
    // 1.组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "insert into OfflineMessage values(%d, '%s')", userid, msg.c_str());
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}
// 删除用户的离线信息
void OfflineMessageModel::remove(int userid)
{
    // 1.组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where user_id =%d", userid);
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}
// 查询用户的离线信息
vector<string> OfflineMessageModel::query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select message from OfflineMessage where user_id =%d", userid);
    vector<string> vec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr)
        {

            // 把 离线信息放入vec中
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}