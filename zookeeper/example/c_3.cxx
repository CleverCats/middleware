#include "zk_sync.h"
#include <iostream>
// 常量
#define HOST "172.17.0.1:2181" // 服务器地址和端口
int main(int argc, char const *argv[])
{
    ZookeeperClient zk(HOST);
    zk.create_node("/lock", "");
    while (true)
    {
        zk.lock();

        std::cout << "get lock c_2 thread" << std::endl;
        sleep(5);

        zk.unlock();
    }

    /* code */
    return 0;
}
