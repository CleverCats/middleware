#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unordered_map>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <mutex>
#include <iostream>
#include <zookeeper/zookeeper.h>

struct contextinfo
{
    std::mutex node_lock;
    std::atomic<bool> notified{false};
    std::condition_variable node_check;
    /* data */
};

// 声明ZookeeperClient类
class ZookeeperClient
{
private:
    // zookeeper的句柄
    zhandle_t *zh;
    // lock_thread_id lock
    std::mutex flash_threads_queue;
    // 线程局部环境
    // 存储每个线程获取的锁-可重入锁机制
    std::unordered_map<std::thread::id, std::string> lock_thread_id;
    // 处理连接事件
    static void connection_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx);

public:
    // 初始化zookeeper客户端
    ZookeeperClient(const char *host, int timout = 30000);

    // 关闭zookeeper客户端
    ~ZookeeperClient();

    // 创建节点 statu 0 持久化节点 1 临时节点
    bool create_node(const char *path, const char *data, int statu = 0);

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
    bool try_lock(std::thread::id tid, std::string node, std::string lockpath = "/lock");
};
