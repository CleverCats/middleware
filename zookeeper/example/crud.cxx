#include <iostream>
#include <assert.h>
#include "zk_sync.h"

#define HOST "60.204.204.220:2181" // 服务器地址和端口
int main(int argc, char const *argv[])
{
    ZookeeperClient zk(HOST);
    bool res = false;
    res = zk.create_node("/test", "", 0);
    assert(res);

    res = zk.create_node("/test/tmp", "hellow", 1);
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
