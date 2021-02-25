#include <iostream>
#include "redis_mgr.h"
#include "../cross_platform_utils/cross_platform_utils.h"

using namespace std;

bool RedisMgr::Init(const string& ip, const short port, const string& pwd)
{
    // 连接redis
    redis_cli_ = std::make_shared<cpp_redis::client>();
    bool isSucc = false;
    redis_cli_->connect(ip, port, [&isSucc](const std::string &host, std::size_t port, cpp_redis::client::connect_state status) {
        if (status == cpp_redis::client::connect_state::ok) {
            cout << "redis连接成功，ip: " << host << ", port: " << port << endl;;
            isSucc = true;
        }
    });
    redis_cli_->sync_commit(std::chrono::seconds(kRedisCommitTimeOuts));
    if (!isSucc) {
        cout << "redis连接断开，ip: " << ip << ", port: " << port << endl;;
        return false;
    }

    // auth
    redis_cli_->auth(pwd, [&isSucc](const cpp_redis::reply &reply) {
        if (reply.is_error()) {
            isSucc = false;
            cout << "redis认证失败: " << reply.as_string() << endl;
        }
    });
    redis_cli_->sync_commit(std::chrono::seconds(kRedisCommitTimeOuts));
    
    return isSucc;
}

bool RedisMgr::Set(const string& key, const string& val)
{
    bool result = true;

    auto curr_ts = CrossPlatformUtils::GetLocalTimeStamp();
    int64_t finish_ts = 0;
    redis_cli_->set(key, val, [&result, &finish_ts](cpp_redis::reply &reply) {
        if (reply.is_error()) {
            cout << "set err: " << reply << ": " << reply.error() << endl;
            result = false;
            return;
        }
        finish_ts = CrossPlatformUtils::GetLocalTimeStamp();
    });

    redis_cli_->sync_commit();
    cout << FILE_FUN << "timeusage|" << finish_ts - curr_ts << " us" << endl;

    return result;
}

bool RedisMgr::GetOrderId(string& id)
{
    bool result = true;
    redis_cli_->select(kOrderIdDbNo, [&result](cpp_redis::reply &reply) {
        if (reply.is_error()) {
            cout << "redis SELECT DB err: " << reply << ": " << reply.error() << endl;
            result = false;
            return;
        }
    });

    redis_cli_->incr(kOrderIdKey, [&](cpp_redis::reply &reply) {
        if (reply.is_error()) {
            cout << "redis incr err: " << reply << ": " << reply.error() << endl;
            result = false;
            return;
        }
        id = to_string(reply.as_integer());
    });
    redis_cli_->sync_commit(); // 实时接口，以同步方式提交

    return result;
}