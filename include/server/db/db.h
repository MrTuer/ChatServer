#ifndef DB_H
#define DB_H
#include<mysql/mysql.h>
#include<string>
#include<muduo/base/Logging.h>
#include<string>
using namespace std;



class MySQL
{
public:
    //初始化连接
    MySQL();
    //释放数据库的连接
    ~MySQL();
    //连接数据库
    bool connect();
    // update
    bool update(string sql);
    //查询操作
    MYSQL_RES *query(string sql);
    // 获取连接
    MYSQL *getconnection();






private:
    

    MYSQL *_conn;



};


#endif