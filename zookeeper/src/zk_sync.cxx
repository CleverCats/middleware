
#include <pthread.h>
#include <algorithm>
#include "zk_sync.h"

#define TIMEOUT 30000 // 会话超时时间（毫秒）
// 定义ZookeeperClient类的构造函数和成员函数

// 初始化zookeeper客户端
ZookeeperClient::ZookeeperClient(const char *host)
{
    // 连接到zookeeper服务器，使用connection_watcher作为连接事件的回调函数
    zh = zookeeper_init(host, connection_watcher, TIMEOUT, NULL, NULL, 0);
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

// 创建节点
void ZookeeperClient::create_node(const char *path, const char *data)
{
    // 创建一个名为path的持久节点，数据为data
    char buffer[128];                // 用于存储实际创建的节点路径
    int buffer_len = sizeof(buffer); // 用于存储节点路径的长度

    // 节点不存在则创建
    if (zoo_exists(zh, path, 0, nullptr) == ZNONODE)
    {
        int ret = zoo_create(zh, path, data, strlen(data), &ZOO_OPEN_ACL_UNSAFE, 0, buffer, buffer_len);
        if (ret == ZOK)
        {
            printf("Created node %s successfully.\n", buffer);
        }
        else
        {
            fprintf(stderr, "Failed to create node: %s\n", zerror(ret));
            exit(-1);
        }
    }
}

// 读取节点
void ZookeeperClient::read_node(const char *path)
{
    // 读取名为path的节点的数据
    char buffer[128];                // 用于存储节点的数据
    int buffer_len = sizeof(buffer); // 用于存储数据的长度
    struct Stat stat;                // 用于存储节点的元数据

    int ret = zoo_get(zh, path, 0, buffer, &buffer_len, &stat);
    if (ret == ZOK)
    {
        printf("Read node %s: %s\n", path, buffer);
    }
    else
    {
        fprintf(stderr, "Failed to read node: %s\n", zerror(ret));
        exit(-1);
    }
}

// 更新节点
void ZookeeperClient::update_node(const char *path, const char *data)
{
    // 更新名为path的节点的数据为data
    struct Stat stat; // 用于存储节点的元数据

    int ret = zoo_set(zh, path, data, strlen(data), -1);
    if (ret == ZOK)
    {
        printf("Updated node %s successfully.\n", path);
    }
    else
    {
        fprintf(stderr, "Failed to update node: %s\n", zerror(ret));
        exit(-1);
    }
}

// 删除节点
void ZookeeperClient::delete_node(const char *path)
{
    // 删除名为path的节点
    int ret = zoo_delete(zh, path, -1);
    if (ret == ZOK)
    {
        printf("Deleted node %s successfully.\n", path);
    }
    else
    {
        fprintf(stderr, "Failed to delete node: %s\n", zerror(ret));
        exit(-1);
    }
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

void ZookeeperClient::lock_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_DELETED_EVENT)
    {
        printf("The previous lock node %s is deleted.\n", path);
        // 通知等待锁的客户端，重新尝试获取锁
        pthread_cond_signal((pthread_cond_t *)watcherCtx);
    }
}

bool ZookeeperClient::check_lock(std::thread::id tid, std::string lockpath)
{
    std::string lock_node_path = lock_thread_id.find(tid)->second;
    // 获取LOCK_PATH下所有子节点，并按照编号排序
    struct String_vector children; // 用于存储子节点名称的数组

    int ret = zoo_get_children(zh, lockpath.c_str(), 0, &children);

    if (ret != ZOK)
    {
        fprintf(stderr, "Failed to get children of lock node: %s\n", zerror(ret));
        exit(-1);
    }
    std::sort(children.data, children.data + children.count, [](const char *a, const char *b)
              { return strcmp(a, b) < 0; }); // 对子节点名称进行排序

    printf("Sorted children of lock node:\n");
    for (int i = 0; i < children.count; i++)
    {
        printf("%s\n", children.data[i]);
    }

    // 判断自己创建的子节点是否是编号最小的那个，如果是，则获得锁
    char *my_node;
    my_node = (char *)lock_node_path.c_str() + lockpath.length() + 1; // 去掉父节点路径和斜杠，得到自己创建的子节点名称

    if (strcmp(my_node, children.data[0]) == 0)
    {
        std::cout << "Got the lock: " << lock_node_path << std::endl;
        return true;
    }

    // 如果不是，则对排在自己前面的那个子节点进行监听，等待其被删除后重新判断是否获得锁
    char *prev_node = NULL; // 用于存储排在自己前面的那个子节点名称
    for (int i = 1; i < children.count; i++)
    {
        if (strcmp(my_node, children.data[i]) == 0)
        {
            prev_node = children.data[i - 1];
            break;
        }
    }

    if (prev_node == NULL)
    {
        fprintf(stderr, "Failed to find the previous node.\n");
        exit(-1);
    }

    printf("The previous lock node is %s, waiting for it to be deleted.\n", prev_node);

    // 创建一个互斥锁和一个条件变量，用于等待锁事件的通知
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    // 对排在自己前面的那个子节点进行监听，使用lock_watcher作为锁事件的回调函数，传入条件变量作为上下文参数
    char prev_path[128]; // 用于存储排在自己前面的那个子节点的完整路径
    sprintf(prev_path, "%s/%s", lockpath.c_str(), prev_node);
    std::cout << "prev_path: " << prev_path << std::endl;
    ret = zoo_wexists(zh, prev_path, lock_watcher, &cond, NULL);

    if (ret != ZOK)
    {
        if(ret == ZNONODE)
        {
            // 销毁互斥锁和条件变量
            pthread_mutex_destroy(&mutex);
            pthread_cond_destroy(&cond);
            return true;
        }
        fprintf(stderr, "Failed to watch the previous node: %s\n", zerror(ret));
        exit(-1);
    }

    // 加锁互斥锁，等待条件变量的信号
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);

    // 销毁互斥锁和条件变量
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return false;
}

void ZookeeperClient::lock(std::string lockpath)
{

    // 获取当前线程的ID
    std::thread::id tid = std::this_thread::get_id();

    // 判断当前线程是否已经获取了锁
    if (lock_thread_id.find(tid) != lock_thread_id.end())
    {
        std::string lock_node_path = lock_thread_id.find(tid)->second;
        std::cout << "Reentered the lock: " << lock_node_path << std::endl;
        return;
    }

    // 创建一个名为LOCK_PATH/lock-的临时顺序节点，数据为空
    char buffer[128];                // 用于存储实际创建的节点路径
    int buffer_len = sizeof(buffer); // 用于存储节点路径的长度

    // std::cout<<"lock_node_path empty"<<std::endl;
    int ret = zoo_create(zh, (lockpath + "/lock-").c_str(), "", 0, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE, buffer, buffer_len);

    if (ret != ZOK)
    {
        fprintf(stderr, "Failed to create lock node: %s\n", zerror(ret));
        exit(-1);
    }

    std::string lock_node_path(buffer);
    lock_thread_id.insert(std::pair<std::thread::id, std::string>(tid, lock_node_path));
    printf("Created lock node %s successfully.\n", buffer);

    std::cout << "try lock" << std::endl;
    // 重新尝试获取锁
    bool is_getlock = check_lock(tid);
    while (!is_getlock)
    {
        is_getlock = check_lock(tid);
    };
    return;
}

// 释放分布式锁
void ZookeeperClient::unlock(std::string lockpath)
{

    std::thread::id tid = std::this_thread::get_id();
    auto zknode = lock_thread_id.find(tid);
    if (zknode != lock_thread_id.end())
    {
        std::string zk_delete_path = zknode->second;
        std::cout << "Trying to delete lock node: " << zk_delete_path << std::endl;
        lock_thread_id.erase(zknode);
        int ret = zoo_delete(zh, zk_delete_path.c_str(), -1);

        if (ret == ZOK)
        {
            printf("Released the lock.\n");
            return;
        }
    }
}