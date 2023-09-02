#include <iostream>
#include <assert.h>
#include "zk_sync.h"
// 常量
#define HOST "172.17.0.1:2181" // 服务器地址和端口
int main(int argc, char const *argv[])
{
    ZookeeperClient zk(HOST);
    bool res = false;
    res = zk.create_node("/test", "");
    assert(res);

    res = zk.create_node("/test/tmp", "hellow");
    assert(res);
    std::cout<<"===== creat node successfully ====="<<std::endl;

    std::string zstr = zk.read_node("/test/tmp");
    std::cout<<"===== read node info: "<<zstr<<" ====="<<std::endl;

    res = zk.update_node("/test/tmp", "world");
    assert(res);

    std::cout<<"===== update node successfully ====="<<std::endl;

    zstr = zk.read_node("/test/tmp");
    std::cout<<"===== read info: "<<zstr<<" ====="<<std::endl;

    res = zk.delete_node("/test/tmp");
    assert(res);

    res = zk.delete_node("/test");
    assert(res);

    std::cout<<"===== delete node successfully ====="<<std::endl;
    /* code */
    return 0;
}
