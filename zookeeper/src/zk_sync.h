
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <zookeeper/zookeeper.h>

// 声明ZookeeperClient类
class ZookeeperClient
{
private:
    // zookeeper的句柄
    zhandle_t *zh;
    // 锁节点
    std::string lock_node_path;
    // 处理连接事件
    static void connection_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx);

public:
    // 初始化zookeeper客户端
    ZookeeperClient(const char *host);

    // 关闭zookeeper客户端
    ~ZookeeperClient();

    // 创建节点
    void create_node(const char *path, const char *data);

    // 读取节点
    void read_node(const char *path);

    // 更新节点
    void update_node(const char *path, const char *data);

    // 删除节点
    void delete_node(const char *path);

    // 处理锁事件
    static void lock_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx);

    // 获取分布式锁
    void lock(std::string lockpath = "/lock");

    // 解锁
    void unlock(std::string lockpath = "/lock");
};
