#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <hiredis/hiredis.h>
using namespace std;

redisContext *c;
ofstream ofile_quote_csv;

static string CurrentLocalTime(void)
{
    time_t t;  //秒时间
    tm *local; //本地时间
    char buf[128] = {0};

    t = time(NULL); //获取目前秒时间

    local = localtime(&t); //转为本地时间
    strftime(buf, 64, "%Y-%m-%d %H:%M:%S", local);

    return buf;
}

static void split(std::string &s, std::string delim, std::vector<std::string> &strs)
{
    size_t last = 0;
    strs.clear();

    if (s.size() <= 0)
    {
        cout << "split len invalid" << endl;
        return;
    }
    size_t index = s.find_first_of(delim, last);
    while (index != std::string::npos)
    {
        strs.push_back(s.substr(last, index - last));
        last = index + 1;
        index = s.find_first_of(delim, last);
    }
    if (index - last > 0)
    {
        strs.push_back(s.substr(last, index - last));
    }
}

static void split_time(string &str_date_time, string &str_date, string &str_time)
{
    vector<string> strs;
    size_t index;

    if (str_date_time.size() <= 0)
    {
        cout << "split_time len invalid" << endl;
        return;
    }
    split(str_date_time, " ", strs);
    if (strs.size() == 2)
    {
        str_date = strs[0];
        str_time = strs[1];
    }
    /* 去掉日期中的无关字符 */
    index = str_date.find_first_of("-", 0);
    while (index != std::string::npos)
    {
        str_date = str_date.erase(index, 1);
        index = str_date.find_first_of("-", 0);
    }
    /* 去掉时间中的无关字符 */
    index = str_time.find_first_of(":", 0);
    while (index != std::string::npos)
    {
        str_time = str_time.erase(index, 1);
        index = str_time.find_first_of(":", 0);
    }

    index = str_time.find_first_of(".", 0);
    while (index != std::string::npos)
    {
        str_time = str_time.erase(index, 1);
        index = str_time.find_first_of(".", 0);
    }
}

static int redis_init(void)
{
    redisReply *reply;

    /* 连接到redis */
    c = redisConnect("127.0.0.1", 6379);
    if (c == NULL || c->err)
    {
        if (c)
        {
            cout << "redisConnect Error: " << c->errstr << endl;
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

    cout << "redis connected and auth succ." << endl;

    return 0;
}

static int open_quote_data_file(void)
{
    string ldate_time, ldate, ltime;
    stringstream file_name;

    ldate_time = CurrentLocalTime();
    split_time(ldate_time, ldate, ltime);

    system("mkdir -p quote_data/gta");
    string dir_name = "quote_data/gta/";

    file_name.clear();
    file_name.str("");
    file_name << dir_name << ldate << ".csv";
    cout << file_name.str() << endl;
    ofile_quote_csv.open(file_name.str());
    if (!ofile_quote_csv)
    {
        cout << "open file[" << file_name.str() << "] failed: " << strerror(errno) << endl;
        return -1;
    }

    return 0;
}

int main(void)
{
    int ret;
    string ldatetime, ldate, ltime;
    stringstream redis_cmd;
    redisReply *reply, *reply1;

    ret = open_quote_data_file();
    if (ret)
    {
        cout << "open_quote_data_file error: " << ret << endl;
    }

    ret = redis_init();
    if (ret)
    {
        cout << "open_quote_data_file error: " << ret << endl;
        return -1;
    }
#if 0
    /* 按日期获取key */
    ldatetime = CurrentLocalTime();
    split_time(ldatetime, ldate, ltime);

    string sec = ltime.substr(ltime.size() - 2);
    if (sec == "50")
    {
        
    }
    string hm = ltime.substr(0, ltime.size() - 2);
   

    redis_cmd.clear();
    redis_cmd.str("");
    /* 使用 KEYS *:*:ldate* 命令获取指定日期的所有key */
    redis_cmd << "KEYS *:*:" << ldate << "*";
    cout << redis_cmd.str() << endl;
    reply = (redisReply *)redisCommand(c, redis_cmd.str().data());
    if (!reply || c->err)
    {
        cout << "redisCommand error: " << c->errstr << "recreate a new connection..." << endl;
        freeReplyObject(reply);
        redisFree(c);
        redis_init();
    }
    if (reply->type == REDIS_REPLY_ERROR)
    {
        cout << "redis 命令执行失败[" << reply->type << "]: " << reply->str << endl;
    }
    cout << "reply->type: " << reply->type << ", reply->elements: " << reply->elements << endl;
    freeReplyObject(reply);
    redisFree(c);

    return 0;
#endif

    redis_cmd.clear();
    redis_cmd.str("");
    /* 使用 KEYS *:*:ldate* 命令获取指定日期的所有key */
    redis_cmd << "KEYS *:*:" << ldate << "*";
    // cout << redis_cmd.str() << endl;
    reply = (redisReply *)redisCommand(c, redis_cmd.str().data());
    if (!reply || c->err)
    {
        cout << "redisCommand error: " << c->errstr << "recreate a new connection..." << endl;
        freeReplyObject(reply);
        redisFree(c);
        redis_init();
    }
    if (reply->type == REDIS_REPLY_ERROR)
    {
        cout << "redis 命令执行失败[" << reply->type << "]: " << reply->str << endl;
    }

    //返回执行结果为整型的命令,只有状态为REDIS_REPLY_INTEGER,并且INTEGER是大于0时,才表示这种类型的命令执行成功
    if (!(reply->type == REDIS_REPLY_ARRAY && reply->elements > 0))
    {
        cout << "redisCommand error: " << c->errstr << "recreate a new connection..." << endl;
        freeReplyObject(reply);
        redisFree(c);
        redis_init();
    }
    cout << "replytype: " << reply->type << ", replyelements: " << reply->elements << endl;

    for (int i = 0; i < reply->elements; i++)
    {
        redis_cmd.clear();
        redis_cmd.str("");
        redis_cmd << "SADD hashes " << reply->element[i]->str;
        // cout << redis_cmd.str() << endl;
        reply1 = (redisReply *)redisCommand(c, redis_cmd.str().data());
        freeReplyObject(reply1);
    }
    freeReplyObject(reply);

    redis_cmd.clear();
    redis_cmd.str("");
    redis_cmd << "SORT hashes BY *->[3]time GET # STORE sortedhashes";
    cout << redis_cmd.str() << endl;
    reply1 = (redisReply *)redisCommand(c, redis_cmd.str().data());
    if (reply1->type == REDIS_REPLY_ERROR)
    {
        cout << "redis 命令执行失败[" << reply1->type << "]: " << reply1->str << endl;
    }
    freeReplyObject(reply1);

    redis_cmd.clear();
    redis_cmd.str("");
    redis_cmd << "lrange sortedhashes 0 -1";
    reply = (redisReply *)redisCommand(c, redis_cmd.str().data());
    if (reply->type == REDIS_REPLY_ERROR)
    {
        cout << "redis 命令执行失败[" << reply->type << "]: " << reply->str << endl;
        return -1;
    }
    for (int i = 0; i < reply->elements; i++)
    {
        redis_cmd.clear();
        redis_cmd.str("");
        redis_cmd << "HVALS " << reply->element[i]->str;
        // cout << redis_cmd.str() << endl;
        reply1 = (redisReply *)redisCommand(c, redis_cmd.str().data());
        if (!(reply1->type == REDIS_REPLY_ARRAY && reply1->elements > 0))
        {
            cout << "redisCommand error: " << c->errstr << "recreate a new connection..." << endl;
            freeReplyObject(reply);
            redisFree(c);
            redis_init();
        }
        // cout << "replytype: " << reply1->type << ", replyelements: " << reply1->elements << endl;
        for (int j = 0; j < reply1->elements; j++)
        {
            // cout << reply1->element[j]->str << endl;
            ofile_quote_csv << reply1->element[j]->str;
            if (j != reply1->elements - 1)
            {
                ofile_quote_csv << "|";
            }
        }
        ofile_quote_csv << endl;
        freeReplyObject(reply1);
        // sleep(1);
    }
    freeReplyObject(reply);

    redisFree(c);

    return 0;

#if 0
        redis_cmd.clear();
        redis_cmd.str("");
        redis_cmd << "HVALS " << reply->element[i]->str;
        // cout << redis_cmd.str() << endl;
        reply1 = (redisReply *)redisCommand(c, redis_cmd.str().data());
        if (!(reply1->type == REDIS_REPLY_ARRAY && reply1->elements > 0))
        {
            cout << "redisCommand error: " << c->errstr << "recreate a new connection..." << endl;
            freeReplyObject(reply);
            redisFree(c);
            redis_init();
        }
        // cout << "replytype: " << reply1->type << ", replyelements: " << reply1->elements << endl;
        for (int j = 0; j < reply1->elements; j++)
        {
            // cout << reply1->element[j]->str << endl;
            ofile_quote_csv << reply1->element[j]->str << "|";
        }
        ofile_quote_csv << endl;
        // sleep(1);
    }

    freeReplyObject(reply);
    freeReplyObject(reply1);
    redisFree(c);

    return 0;
#endif
}
