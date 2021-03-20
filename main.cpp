#include "include/sockets.hpp"
#include <iostream>
#include <thread>

using std::thread;
using std::cout;

int main(){
    EasySocket::base_client b_client;
    EasySocket::base_server b_serv;

    b_serv.set_host("127.0.0.1", "5555");
    b_serv.connect();
    b_client.set_host("0.0.0.0", "5555");

    // auto l = [](std::string r){return r;};
    thread serv_t(
        &EasySocket::base_server::await_connections,
        &b_serv,
        "sup\n"
    );

    b_client.connect();
    b_client.ts_send("Yo yo yo1");
    b_client.ts_send("Yo yo yo2");
    b_client.ts_send("Yo yo yo3");

    serv_t.join();

    return 0;
}