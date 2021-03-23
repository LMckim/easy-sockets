#include "include/sockets.hpp"
#include <iostream>
#include <thread>
#include <time.h>

using std::thread;
using std::cout;

int main(){
    std::cout << "RUNNING TESTS...\n";
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
    std::string t_msg_01 = "TEST1";
    sleep(1);
    b_client.ts_send(t_msg_01);
    b_serv.close();
    if(b_serv.last_recv() == t_msg_01){
        std::cout << ".";
    }else std::cout << "F";
    b_client.ts_send("close");

    serv_t.join();
    std::cout << "\n";
    return 0;
}

