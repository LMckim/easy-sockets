
#include <iostream>
#include "include/easysocket.hpp"


int main(){
    using EasySocket::Server;
    using EasySocket::PreHosts;

    Server s = Server();
    s.connect(PreHosts::LOCAL, "6555");
    s.respond_with("hello!");
    s.close();
    return 0;
}