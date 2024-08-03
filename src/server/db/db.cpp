#include "db.h"
#include<muduo/base/Logging.h>
#include<string>
// 初始化连接
static string server = "127.0.0.1";
static  string user = "root";
static string password = "123456";
static string dbname = "chat";
MySQL::MySQL()
{
    _conn = mysql_init(nullptr);
}
// 释放数据库的连接
MySQL::~MySQL()
{
    if (_conn != nullptr)
    {
        mysql_close(_conn);
    }
}
// 连接数据库
bool MySQL:: connect()
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
                                  password.c_str(),dbname.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        // c/c++ ，默认的编码字符是ASCII 如果不设置 就会出现乱码
        mysql_query(_conn, "set name gbk");
        LOG_INFO << "connect mysql success";
    }
    else
    {
        LOG_INFO << "connect mysql fauile";
    }
    return p;
}
// update
bool MySQL::update(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "更新失败！";
        return false;
    }
    return true;
}
MYSQL_RES* MySQL::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "查询失败！";
        return nullptr;
    }
    return mysql_use_result(_conn);
}
MYSQL * MySQL::getconnection()
{
    return _conn;
}