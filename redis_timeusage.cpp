#include <iostream>
#include <string>
#include <chrono>
#include "redis_mgr.h"
#include "cross_platform_utils.h"

using namespace std;

int main(void)
{
    using namespace std::chrono;

    string ip = "192.168.1.72";
    unsigned int port = 6379;
    string pwd = "devredis";

    RedisMgr redis_mgr;
    if (!redis_mgr.Init(ip, port, pwd)) {
        cout << FILE_FUN << "redis_mgr init failed" << endl;
        return -1;
    }

    string key = "a";
    string val = "b";
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    if (!redis_mgr.Set(key, val)) {
        cout << FILE_FUN << "redis_mgr set failed" << endl;
        return -1;
    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    duration<double, std::milli> time_span = t2 - t1; // 根据需要的精度调整参数
    std::cout << FILE_FUN << "set tooks|" << time_span.count() << " ms\n";

    string stkorderid;
    t1 = high_resolution_clock::now();
    if (!redis_mgr.GetOrderId(stkorderid)) {
        cout << FILE_FUN << "redis_mgr set failed" << endl;
        return -1;
    }
    t2 = high_resolution_clock::now();
    time_span = t2 - t1; // 根据需要的精度调整参数
    std::cout << FILE_FUN << "GetOrderId tooks|" << time_span.count() << " ms\n";
    cout << FILE_FUN << "stk orderid|" << stkorderid << endl;
    
    cout << FILE_FUN_STR << endl;
    return 0;
}
