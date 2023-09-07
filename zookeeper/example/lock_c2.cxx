#include "zk_sync.h"
#include <iostream>

#define HOST "60.204.204.220:2181" // 服务器地址和端口
int main(int argc, char const *argv[])
{
    ZookeeperClient zk(HOST);
    zk.create_node("/lock", "");
    int k = 0;
    while (true)
    {
        zk.lock();
        std::cout <<++k <<" ======== c_2: get lock ======" << std::endl;;
        sleep(10);
        zk.unlock();
    }

    /* code */
    return 0;
}
