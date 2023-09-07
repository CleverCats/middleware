
#include <pthread.h>
#include <algorithm>
#include "zk_sync.h"

// 初始化zookeeper客户端
ZookeeperClient::ZookeeperClient(const char *host, int timout)
{
    // 连接到zookeeper服务器，使用connection_watcher作为连接事件的回调函数
    zh = zookeeper_init(host, connection_watcher, timout, NULL, NULL, 0);
    if (zh == NULL)
    {
        fprintf(stderr, "Failed to connect to zookeeper server.\n");
        exit(-1);
    }
}

// 关闭zookeeper客户端
ZookeeperClient::~ZookeeperClient()
{
    // 关闭与zookeeper服务器的连接
    zookeeper_close(zh);
}

// 处理连接事件
void ZookeeperClient::connection_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT)
    {
        if (state == ZOO_CONNECTED_STATE)
        {
            printf("Connected to zookeeper server.\n");
        }
        else if (state == ZOO_EXPIRED_SESSION_STATE)
        {
            printf("Zookeeper session expired.\n");
        }
    }
}

// 创建节点
bool ZookeeperClient::create_node(const char *path, const char *data, int statu)
{
    // 创建一个名为path的持久节点，数据为data
    char buffer[128];                // 用于存储实际创建的节点路径
    int buffer_len = sizeof(buffer); // 用于存储节点路径的长度

    // 节点不存在则创建
    if (zoo_exists(zh, path, 0, nullptr) == ZNONODE)
    {
        int ret = zoo_create(zh, path, data, strlen(data), &ZOO_OPEN_ACL_UNSAFE, statu, buffer, buffer_len);
        if (ret == ZOK)
        {
            return true;
            // printf("Created node %s successfully.\n", buffer);
        }
        else
        {
            fprintf(stderr, "Failed to create node: %s\n", zerror(ret));
            return false;
        }
    }
    return true;
}

// 读取节点
std::string ZookeeperClient::read_node(const char *path)
{
    // 读取名为path的节点的数据
    char buffer[128];                // 用于存储节点的数据
    int buffer_len = sizeof(buffer); // 用于存储数据的长度
    struct Stat stat;                // 用于存储节点的元数据

    int ret = zoo_get(zh, path, 0, buffer, &buffer_len, &stat);
    if (ret == ZOK)
    {
        return std::string(buffer, buffer + buffer_len);
        // printf("Read node %s: %s\n", path, buffer);
    }
    else
    {
        fprintf(stderr, "Failed to read node: %s\n", zerror(ret));
        exit(-1);
    }
}

// 更新节点
bool ZookeeperClient::update_node(const char *path, const char *data)
{
    // 更新名为path的节点的数据为data
    struct Stat stat; // 用于存储节点的元数据

    int ret = zoo_set(zh, path, data, strlen(data), -1);
    if (ret == ZOK)
    {
        return true;
        // printf("Updated node %s successfully.\n", path);
    }
    else
    {
        fprintf(stderr, "Failed to update node: %s\n", zerror(ret));
        return false;
    }
}

// 删除节点
bool ZookeeperClient::delete_node(const char *path)
{
    // 删除名为path的节点
    int ret = zoo_delete(zh, path, -1);
    if (ret == ZOK)
    {
        return true;
        // printf("Deleted node %s successfully.\n", path);
    }
    else
    {
        fprintf(stderr, "Failed to delete node: %s\n", zerror(ret));
        return false;
    }
}
