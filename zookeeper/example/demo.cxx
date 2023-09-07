#include <iostream>
#include <thread>
#include <unistd.h>
#include <pthread.h>

int a = 0;

class demo
{
public:
    demo() {}
    ~demo() {}
    void func2()
    {
        while (true)
        {
            sleep(2);
            a += 10;
            std::cout << "func2  " << a << std::endl;
        }
    }

    void func1()
    {
        while (true)
        {
            sleep(1);
            a += 1;
            std::cout << "func1  " << a << std::endl;
        }
    }
};

int main(int argc, char const *argv[])
{
    demo demoObj;
    std::thread t1(&demo::func1, &demoObj);
    std::thread t2(&demo::func2, &demoObj);

    t1.join(); // 等待 t1 完成
    t2.join(); // 等待 t2 完成
    return 0;
}