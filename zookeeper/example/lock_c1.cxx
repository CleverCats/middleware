#include "zk_sync.h"
#include <iostream>
// 常量
#define HOST "172.17.0.1:2181" // 服务器地址和端口
int main(int argc, char const *argv[])
{
    ZookeeperClient zk(HOST);
    zk.create_node("/lock","");
    int k = 0;
    while (true)
    {
        zk.lock();
        sleep(3);
        zk.lock();
        sleep(3);
        zk.lock();
        sleep(3);
        std::cout <<++k <<" ======== c_1: get lock thread ======" << std::endl;
        zk.unlock();
    }
    
   
    /* code */
    return 0;
}
