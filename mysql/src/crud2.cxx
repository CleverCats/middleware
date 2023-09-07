// 引入cppconn库
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

// 定义数据库连接参数
#define DB_HOST "localhost"
#define DB_USER "anni"
#define DB_PASS "123456"
#define DB_NAME "test"

using namespace std;
using namespace sql;

int main() {
    try {
        // 创建数据库驱动对象
        Driver* driver = get_driver_instance();
        // 连接数据库
        Connection* conn = driver->connect(DB_HOST, DB_USER, DB_PASS);
        // 选择数据库
        conn->setSchema(DB_NAME);
        // 创建语句对象
        Statement* stmt = conn->createStatement();
        // 执行SQL语句，返回结果集对象
        ResultSet* rs = stmt->executeQuery("SELECT * FROM users");
        // 遍历结果集，打印每一行数据
        while (rs->next()) {
            cout << rs->getInt("id") << "\t";
            cout << rs->getString("name") << "\t";
            cout << rs->getString("email") << endl;
        }
        // 释放结果集对象
        delete rs;
        // 创建预处理语句对象，用于插入数据
        PreparedStatement* pstmt = conn->prepareStatement("INSERT INTO users(name, email) VALUES (?, ?)");
        // 设置参数值
        pstmt->setString(1, "Alice");
        pstmt->setString(2, "alice@example.com");
        // 执行SQL语句，返回影响的行数
        int rows = pstmt->executeUpdate();
        // 打印影响的行数
        cout << "Inserted " << rows << " row(s)" << endl;
        // 释放预处理语句对象
        delete pstmt;
        // 创建预处理语句对象，用于更新数据
        pstmt = conn->prepareStatement("UPDATE users SET email = ? WHERE name = ?");
        // 设置参数值
        pstmt->setString(1, "bob@gmail.com");
        pstmt->setString(2, "Bob");
        // 执行SQL语句，返回影响的行数
        rows = pstmt->executeUpdate();
        // 打印影响的行数
        cout << "Updated " << rows << " row(s)" << endl;
        // 释放预处理语句对象
        delete pstmt;
        // 创建预处理语句对象，用于删除数据
        pstmt = conn->prepareStatement("DELETE FROM users WHERE name = ?");
        // 设置参数值
        pstmt->setString(1, "Alice");
        // 执行SQL语句，返回影响的行数
        rows = pstmt->executeUpdate();
        // 打印影响的行数
        cout << "Deleted " << rows << " row(s)" << endl;
        // 释放预处理语句对象
        delete pstmt;
        // 释放语句对象
        delete stmt;
        // 关闭数据库连接
        conn->close();
    } catch (SQLException& e) {
       // 捕获异常，打印错误信息
       cout << "# ERR: SQLException in " << __FILE__;
       cout << "(" << __FUNCTION__ << ") on line "
          << __LINE__ << endl;
       cout << "# ERR: " << e.what();
       cout << " (MySQL error code: " << e.getErrorCode();
       cout << ", SQLState: " << e.getSQLState() <<
          ")" << endl;
    }
    return 0;
}
