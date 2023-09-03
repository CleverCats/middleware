#include <algorithm>
#include <iostream>
#include <string>
#include <string.h>
#include <list>
#include "zk_sync.h"
using namespace std;
#define HOST "60.204.204.220:2181" // 服务器地址和端口
ZookeeperClient zk(HOST);
class msgQueue
{
private:
    std::list<int> msgque;
    mutex msgmutx;
public:
    void msg_push()
    {
        cout << " ==== tid: " << std::this_thread::get_id() << " producer runing ====" << endl;
        for (int i = 0; i < 100; ++i)
        {
            zk.lock();
            msgque.push_back(i);
            cout << "msg push: " << i << endl;
            zk.unlock();
        }
    }
    void msg_get()
    {
        cout << " ==== tid: " << std::this_thread::get_id() << " consumer runing ====" << endl;
        while (true)
        {
            zk.lock();
            // cout << "==== tid: "<<std::this_thread::get_id()<<" get lock and sleep ====" << endl;
            if (!msgque.empty())
            {
                int msgrecv = msgque.front(); // 读取队列顶层消息
                msgque.pop_front();
                cout << "msg get: " << msgrecv << endl;
            }
            zk.unlock();
        }
    }
};
int main()
{
    msgQueue msgQue;
    std::thread producer(&msgQueue::msg_push, &msgQue);
    std::thread consumer(&msgQueue::msg_get, &msgQue);

    producer.join();
    consumer.join();
    return 0;
}