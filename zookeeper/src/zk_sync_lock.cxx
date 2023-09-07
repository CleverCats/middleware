#include <pthread.h>
#include <algorithm>
#include "zk_sync.h"
thread_local contextinfo sync_lock;
// 监视回调
void ZookeeperClient::lock_watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_DELETED_EVENT)
    {
        std::cout << "notify_one " << std::endl;
        contextinfo *context = (contextinfo *)watcherCtx;
        context->notified = true;
        context->node_check.notify_one();
    }
    else
    {
        std::cout << "lock_watcher undefine statu" << std::endl;
    }
}

bool ZookeeperClient::try_lock(std::thread::id tid, std::string node, std::string lockpath)
{
    if (lock_thread_id.find(tid) == lock_thread_id.end())
    {
        flash_threads_queue.lock();
        if (lock_thread_id.find(tid) == lock_thread_id.end())
        {
            std::thread::id tid = std::this_thread::get_id();
            lock_thread_id.insert(std::pair<std::thread::id, std::string>(tid, node));
        }
        flash_threads_queue.unlock();
    }

    std::string lock_node_path = lock_thread_id.find(tid)->second;
    // 获取LOCK_PATH下所有子节点，并按照编号排序

    struct String_vector children; // 用于存储子节点名称的数组
    // sleep(10);
    int ret = zoo_get_children(zh, lockpath.c_str(), 0, &children);

    if (ret != ZOK)
    {
        fprintf(stderr, "Failed to get children of lock node: %s\n", zerror(ret));
        return false;
        // exit(-1);
    }

    // 若自己是唯一节点
    if (children.count == 1)
        return true;

    std::sort(children.data, children.data + children.count, [](const char *a, const char *b)
              { return strcmp(a, b) < 0; }); // 对子节点名称进行排序

    // printf("Sorted children of lock node:\n");
    // for (int i = 0; i < children.count; i++)
    // {
    //     printf("%s\n", children.data[i]);
    // }

    // 本线程创建的节点不是序号最小
    char *my_node = (char *)lock_node_path.c_str() + lockpath.length() + 1; // 去掉父节点路径和斜杠，得到自己创建的子节点名称
    // std::cout << "minist node" << children.data[0] << std::endl;
    // std::cout << "my node" << my_node << std::endl;

    if (strcmp(my_node, children.data[0]) == 0)
    {
        // std::cout << "get the lock: " << lock_node_path << std::endl;
        return true;
    }

    // 对排在自己前面的那个子节点进行监听，等待其被删除后重新判断是否获得锁
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

    sync_lock.notified = false;
    // 对排在自己前面的那个子节点进行监听，使用lock_watcher作为锁事件的回调函数，传入条件变量作为上下文参数
    char prev_path[128]; // 用于存储排在自己前面的那个子节点的完整路径
    sprintf(prev_path, "%s/%s", lockpath.c_str(), prev_node);
    // std::cout << "prev_path: " << prev_path << std::endl;
    ret = zoo_wexists(zh, prev_path, lock_watcher, &sync_lock, NULL);

    if (ret != ZOK)
    {
        if (ret == ZNONODE)
        {
            // 监视节点不存在(已释放)
            return true;
        }
        fprintf(stderr, "Failed to watch the previous node: %s\n", zerror(ret));
        return false;
        // exit(-1);
    }

    // 加锁互斥锁，等待条件变量的信号
    std::unique_lock<std::mutex> lck(sync_lock.node_lock);

    sync_lock.node_check.wait(lck, [&]() -> bool
                              {
        if(sync_lock.notified == true)
        {
            std::cout<< "node: "<<prev_path<<" be deleted"<<std::endl;    
            // sleep(10);
            // std::cout<< "sleeping 10s end"<<std::endl;
        }
        else
        {   
            // std::cout<< "waiting node: "<<prev_path<<" be deleted"<<std::endl;
            // sleep(10);
            // std::cout<< "sleeping 10s end"<<std::endl;
        }
        return sync_lock.notified; });

    // 监视节点已释放
    return true;
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
        // Zookeeper 客户端与服务器端失去连接[重试]
        if (ret == ZCONNECTIONLOSS || ret == ZUNIMPLEMENTED)
        {
            static short retrytime = 0;
            while (ret != ZOK && retrytime < 3)
            {
                sleep(3);
                fprintf(stderr, "retrying %d/3 create node \n", retrytime + 1);
                ret = zoo_create(zh, (lockpath + "/lock-").c_str(), "", 0, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE, buffer, buffer_len);
                ++retrytime;
            }
            retrytime = 0;
        }
        if (ret != ZOK)
        {
            fprintf(stderr, "Failed to create lock node: %s\n", zerror(ret));
            exit(-1);
        }
    }

    std::string lock_node_path(buffer);
    // std::cout << "creat node :" << lock_node_path << std::endl;

    // 尝试获取锁
    bool is_getlock = try_lock(tid, lock_node_path);
    while (!is_getlock)
    {
        is_getlock = try_lock(tid, lock_node_path);
    }

    return;
}

// 释放分布式锁
void ZookeeperClient::unlock(std::string lockpath)
{

    std::thread::id tid = std::this_thread::get_id();
    std::string zk_delete_path = "";

    flash_threads_queue.lock();
    auto zknode = lock_thread_id.find(tid);
    if (zknode != lock_thread_id.end())
    {
        zk_delete_path = zknode->second;
        // std::cout << "Trying to delete lock node: " << zk_delete_path << std::endl;

        lock_thread_id.erase(zknode);
        // std::cout<<std::endl;
        // std::cout << "delete thread: " <<std::this_thread::get_id() << std::endl;
    }
    flash_threads_queue.unlock();

    if (zk_delete_path != "")
    {
        int ret = zoo_delete(zh, zk_delete_path.c_str(), -1);

        if (ret == ZOK)
        {
            // std::cout<<"release node: "<<zk_delete_path<<std::endl;
            return;
        }
    }
}
