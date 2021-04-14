
#include <iostream>
#include <string>

#include "include/easysocket.hpp"

using std::string;

string test_respond(string _data){
    if(_data.find("yo") != std::string::npos){
        return "heyo!\n";
    }else{
        return "heyno...\n";
    }
}

int main(){
    using EasySocket::Server;
    using EasySocket::PreHosts;

    Server s = Server();
    s.connect(PreHosts::LOCAL, "6555");
    // s.respond_with("hello!\n");
    s.respond_with(&test_respond);
    s.close();
    return 0;
}