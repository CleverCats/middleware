#include <iostream>
#include <mysql_driver.h>  
#include <mysql_connection.h>  
#include <cppconn/driver.h>  
#include <cppconn/exception.h>  
#include <cppconn/resultset.h>  
#include <cppconn/statement.h>  
using namespace std;  
  
int main() {  
    sql::mysql::MySQL_Driver *driver;  
    sql::Connection *con;  
  
    // 创建连接驱动，并连接数据库  
    driver = sql::mysql::get_mysql_driver_instance();  
    con = driver->connect("localhost", "anni", "123456");
    if(con != nullptr)
    {
        // 选择数据库
        con->setSchema("test");
        std::cout<<"connect to mysql:test success"<<std::endl;
    }
    else
    {
        std::cout<<"connect err"<<std::endl;
        exit(1);
    }
        
    // 创建一个 SQL 语句对象  
    sql::Statement *stmt = con->createStatement();  
  
    // 创建表  
    stmt->execute("CREATE TABLE memberlist (id INT AUTO_INCREMENT PRIMARY KEY, name VARCHAR(50), age INT)");  
  
    // 增加（插入）数据  
    stmt->execute("INSERT INTO memberlist (name, age) VALUES ('Tom', 25)");  
  
    // 删除数据  
    stmt->execute("DELETE FROM memberlist WHERE age = 25");  
  
    // 修改数据  
    stmt->execute("UPDATE memberlist SET age = 30 WHERE name = 'Tom'");  
  
    // 查询数据  
    sql::ResultSet *res = stmt->executeQuery("SELECT * FROM memberlist");  
    while (res->next()) {  
        std::cout << res->getString("name") << " - " << res->getInt("age") << std::endl;  
    }  
    delete res;  
    delete stmt;  
    delete con;  
  
    return 0;  
}