#include <iostream>
#include <cpp_redis/cpp_redis>

using namespace std;

int main(void)
{
    //! Enable logging
    cpp_redis::active_logger = std::unique_ptr<cpp_redis::logger>(new cpp_redis::logger);

    cpp_redis::client client;

    client.connect("127.0.0.1", 6379, [](const string &host, size_t port, cpp_redis::client::connect_state status) {
        if (status == cpp_redis::client::connect_state::dropped)
        {
            cout << "client disconnected from " << host << ":" << port << endl;
        }
    });

    //auth
    // client.send({ "AUTH", "password" });
    client.auth("password");

    // same as client.send({ "SET", "hello", "42" }, ...)
    client.set("hello", "42", [](cpp_redis::reply &reply) {
        cout << "set hello 42: " << reply << endl;
        // if (reply.is_string())
        //   do_something_with_string(reply.as_string())
    });

    client.decrby("hello", 12, [](cpp_redis::reply &reply) {
        cout << "decrby hello 12: " << reply << endl;
        // if (reply.is_integer())
        //   do_something_with_integer(reply.as_integer())
    });

    client.get("hello", [](cpp_redis::reply &reply) {
        cout << "get hello: " << reply << endl;
    });

    // commands are pipelined and only sent when client.commit() is called
    // client.commit();

    // synchronous commit, no timeout
    client.sync_commit();

    // synchronous commit, timeout
    // client.sync_commit(std::chrono::milliseconds(100));
}
