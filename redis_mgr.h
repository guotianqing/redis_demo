#ifndef REDIS_MGR_H_
#define REDIS_MGR_H_

#include <string>
#include <cpp_redis/cpp_redis>

using namespace std;
using namespace cpp_redis;

class RedisMgr
{
 public:
  bool Init(const string& ip, const short port, const string& pwd);
  bool Set(const string& key, const string& val);
  bool GetOrderId(string& id);

 private:
  std::shared_ptr<cpp_redis::client> redis_cli_;

  const int kRedisCommitTimeOuts = 3;
  const int kOrderIdDbNo = 1;
  const string kOrderIdKey = "stk_order_id";
  

};

#endif