# zookkeeper c/c++ crud+分布式锁的实现及封装

## 功能
    一、zookeeper c/c++ crud非异步接口的封装
    二、基于临时顺序节点分布式可重入锁接口的实现

## zookeeper_c依赖安装
    - wget http://archive.apache.org/dist/zookeeper/zookeeper-3.4.10/zookeeper-3.4.10.tar.gz
    - cd zookeeper-3.4.10/conf
    - cp zoo_sample.cfg zoo.cfg
    - cd zookeeper-3.4.10/src/c 
    - sudo ./configure && make && make install
    - 若出现make抛出sprintf error编译警告则去除makefile -Werror参数重新编译
    - apt install openjdk-11-jdk
    - ./zookeeper-3.4.10/bin/zkServer.sh start

## 项目编译
    - mkdir build
    - cd build
    - cmake .. && make

# 链接
    - .L./lib 
    - .I./src 

# 接口
    - #include "zk_sync.h"
```cpp
    /*
        brief 初始化zookeeper客户端
        @param host: ip:port
        @param timout: 会话超时时间(毫秒)
    */
    ZookeeperClient(const char *host, int timout = 30000);

    /*
        brief 创建持久化节点
        @param path: zk节点路径
        @param data: 存储数据
        @param statu: 节点类型
    */
    bool create_node(const char *path, const char *data, int statu=0);

    // 读取节点
    std::string read_node(const char *path);

    // 更新节点
    bool update_node(const char *path, const char *data);

    // 删除节点
    bool delete_node(const char *path);

    /*
        brief 获取分布式锁, 需保证zksever中lockpath存在否则异常
        @param lockpath: zk锁路径
    */
    void lock(std::string lockpath = "/lock");

    // 解锁
    void unlock(std::string lockpath = "/lock");



