#include <iostream>
#include <string>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>

#include <thread>

using namespace std;

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>

void connectCallback(const redisAsyncContext *c, int status);
void disconnectCallback(const redisAsyncContext *c, int status);
void getCallback(redisAsyncContext *c, void *r, void *privdata);

static void list_write(void)
{
    redisReply *reply;
    redisContext *rc_write = redisConnect("127.0.0.1", 6379);
    if (rc_write == NULL || rc_write->err)
    {
        if (rc_write)
        {
            cerr << "Error: " << rc_write->errstr << endl;
        }
        else
        {
            cerr << "Can't allocate redis context" << endl;
        }
        return;
    }
    /* 登录redis */
    reply = (redisReply *)redisCommand(rc_write, "AUTH password");
    if (reply->type == REDIS_REPLY_ERROR)
    {
        cout << "redis 登录时认证失败[" << reply->type << "]: " << reply->str << endl;
        freeReplyObject(reply);
        return;
    }
    freeReplyObject(reply);

    reply = (redisReply *)redisCommand(rc_write, "select 1");
    if (reply->type == REDIS_REPLY_ERROR)
    {
        cout << "redisCommand: [ select 1 ] error, " << reply->type << reply->str << endl;
        freeReplyObject(reply);
        return;
    }
    freeReplyObject(reply);

    stringstream redis_cmd;
    int i = 0;
    while (1)
    {
        sleep(5);
        i++;
        redis_cmd.clear();
        redis_cmd.str("");
        redis_cmd << "RPUSH list_test " << i; 
        reply = (redisReply *)redisCommand(rc_write, redis_cmd.str().data());
        if (reply->type == REDIS_REPLY_ERROR)
        {
            cout << "redisCommand: " << redis_cmd.str() << "error, " << reply->type << reply->str << endl;
            freeReplyObject(reply);
            continue;
        }
        // cout << "set reply: " << reply->str << endl;
        freeReplyObject(reply);
    }

    redisFree(rc_write);
}

static void list_read(void)
{
    redisReply *reply;
    redisContext *rc_read = redisConnect("127.0.0.1", 6379);
    if (rc_read == NULL || rc_read->err)
    {
        if (rc_read)
        {
            cerr << "Error: " << rc_read->errstr << endl;
        }
        else
        {
            cerr << "Can't allocate redis context" << endl;
        }
        return;
    }
    /* 登录redis */
    reply = (redisReply *)redisCommand(rc_read, "AUTH password");
    if (reply->type == REDIS_REPLY_ERROR)
    {
        cout << "redis 登录时认证失败[" << reply->type << "]: " << reply->str << endl;
        freeReplyObject(reply);
        return;
    }
    freeReplyObject(reply);

    reply = (redisReply *)redisCommand(rc_read, "select 1");
    if (reply->type == REDIS_REPLY_ERROR)
    {
        cout << "redisCommand: [ select 1 ] error, " << reply->type << reply->str << endl;
        freeReplyObject(reply);
        return;
    }
    freeReplyObject(reply);

    stringstream redis_cmd;

    uint64_t last_count = 0, cur_count = 0, new_items = 0;
    while (1)
    {
        sleep(1);
        redis_cmd.clear();
        redis_cmd.str("");
        redis_cmd << "LLEN list_test";
        reply = (redisReply *)redisCommand(rc_read, redis_cmd.str().data());
        if (reply->type == REDIS_REPLY_ERROR)
        {
            cout << "redisCommand: " << redis_cmd.str() << "error, " << reply->type << reply->str << endl;
            freeReplyObject(reply);
            continue;
        }
        cur_count = reply->integer;
        cout << "cur_count: " << cur_count << endl;
        freeReplyObject(reply);
        if (cur_count == 0 || cur_count == last_count)
        {
            cout << "no new items, cut_count=" << cur_count << ", last_count=" << last_count << endl;
            continue;
        }

        new_items = cur_count - last_count;
        last_count = cur_count;
        redis_cmd.clear();
        redis_cmd.str("");
        redis_cmd << "LRANGE list_test -" << new_items << " -1";
        reply = (redisReply *)redisCommand(rc_read, redis_cmd.str().data());
        if (reply->type == REDIS_REPLY_ERROR)
        {
            cout << "redisCommand: " << redis_cmd.str() << "error, " << reply->type << reply->str << endl;
            freeReplyObject(reply);
            continue;
        }
        for (int j = 0; j < reply->elements; j++)
        {
            cout << reply->element[j]->str << endl;
        }
        freeReplyObject(reply);
    }

    redisFree(rc_read);
}

int main(void)
{
    redisReply *reply;

    signal(SIGPIPE, SIG_IGN); //捕捉程序收到数据包时候的信号

#if 1
    /* 同步接口 */
    cout << "begin sync api test" << endl;
    redisContext *c = redisConnect("127.0.0.1", 6379);
    if (c == NULL || c->err)
    {
        if (c)
        {
            cerr << "Error: " << c->errstr << endl;
        }
        else
        {
            cerr << "Can't allocate redis context" << endl;
        }
        return -1;
    }
    /* 登录redis */
    reply = (redisReply *)redisCommand(c, "AUTH password");
    if (reply->type == REDIS_REPLY_ERROR)
    {
        cout << "redis 登录时认证失败[" << reply->type << "]: " << reply->str << endl;
        freeReplyObject(reply);
        return -2;
    }
    freeReplyObject(reply);

    reply = (redisReply *)redisCommand(c, "select 1");
    cout << "set reply: " << reply->str << endl;
    freeReplyObject(reply);

    reply = (redisReply *)redisCommand(c, "SET atest b");
    cout << "set reply: " << reply->str << endl;
    freeReplyObject(reply);

    reply = (redisReply *)redisCommand(c, "get atest");
    cout << "get reply: " << reply->str << endl;
    freeReplyObject(reply);

    thread task_producer(list_write);
    task_producer.detach();
    thread task_consumer(list_read);
    task_consumer.detach();

    while (1)
    {
        sleep(1);
    }

    redisFree(c);
    cout << "sync api test done!" << endl;
#endif

#if 0
    /* 异步接口 */
    cout << "begin async api test" << endl;

    struct event_base *base = event_base_new();//新建一个libevent事件处理
    redisAsyncContext  *c;
    c = redisAsyncConnect("127.0.0.1", 6379);//新建异步连接
    if (c->err) 
    {
        cerr << "Error: " << c->errstr << endl;
        return -1;
    }

    redisLibeventAttach(c, base);//将连接添加到libevent事件处理
    redisAsyncSetConnectCallback(c, connectCallback);//设置连接回调
    redisAsyncSetDisconnectCallback(c, disconnectCallback);//设置断开连接回调

    redisAsyncCommand(c, NULL, NULL, "SET asyckey asycvalue");//发送set命令
    redisAsyncCommand(c, getCallback, NULL, "GET asyckey");//发送get命令
    event_base_dispatch(base);//开始libevent循环。注意在这一步之前redis是不会进行连接的，前边调用的命令发送函数也没有真正发送命令

    cout << "async api test done!" << endl;
#endif

    return 0;
}

//设置命令执行后的回调函数
void getCallback(redisAsyncContext *c, void *r, void *privdata)
{
    redisReply *reply = (redisReply *)r;
    if (reply == NULL)
        return;
    cout << reply->str << endl;

    /* Disconnect after receiving the reply to GET */
    redisAsyncDisconnect(c);
}

//设置连接回调函数
void connectCallback(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        cout << "connect to redis error: " << c->errstr << endl;
        c = redisAsyncConnect("127.0.0.1", 6379);
    }
    else
    {
        printf("Connected to redis\n");
    }
}

//设置断开连接回调函数
void disconnectCallback(const redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        cout << "disconnect error: " << c->errstr << endl;
        return;
    }
    printf("Disconnected...\n");
}
