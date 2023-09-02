
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mutex>
#include <map>
#include <thread>
#include <iostream>
#include <zookeeper/zookeeper.h>

// 声明ZookeeperClient类
class ZookeeperClient
{
private:
    // zookeeper的句柄
    zhandle_t *zh;
    // 存储每个线程获取的锁-可重入锁机制
    std::map<std::thread::id, std::string> lock_thread_id;
    // 处理连接事件
    static void connection_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx);

public:
    // 初始化zookeeper客户端
    ZookeeperClient(const char *host, int timout = 30000);

    // 关闭zookeeper客户端
    ~ZookeeperClient();

    // 创建节点
    bool create_node(const char *path, const char *data);

    // 读取节点
    std::string read_node(const char *path);

    // 更新节点
    bool update_node(const char *path, const char *data);

    // 删除节点
    bool delete_node(const char *path);

    // 获取分布式锁
    void lock(std::string lockpath = "/lock");

    // 解锁
    void unlock(std::string lockpath = "/lock");

private:
     // 处理锁事件
    static void lock_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx);
    
    // 获取加锁条件
    bool check_lock(std::thread::id tid, std::string lockpath = "/lock");
};
