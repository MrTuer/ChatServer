#include "usermodel.hpp"
#include "db.h"
// User表的增加方法
bool UserModel::insert(User &user)
{
    // 1.组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "insert into User(name,password,state) values('%s','%s','%s')",
            user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取插入成功 的用户生成的主键id
            user.setId(mysql_insert_id(mysql.getconnection()));
            return true; // 注册成功
        }
    }
    return false;
    // 注册失败
}

User UserModel::query(int id)
{
    // 1.组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "select * from User where id =%d", id);
    printf("query start!  id = %d\n", id);
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        printf("query end!\n");
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
        }
    }

    return User();
}

// 更新用户状态信息
bool UserModel::updateState(User user)
{
    // 1.组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "update  User set state ='%s' where id =%d", user.getState().c_str(), user.getId());

    MySQL mysql;
    if (mysql.connect())
    {

        printf("update start!\n");
        if (mysql.update(sql))
        {

            printf("update end!\n");
            return true;
        }
    }
    return false;
}
// 重置客户信息
void UserModel::resetState()
{
    // 1.组装SQL语句
    char sql[1024] = {0};
    sprintf(sql, "update  User set state ='offline' where state ='online'");
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    } 
}