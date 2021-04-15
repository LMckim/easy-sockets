
#include <iostream>
#include <string>

#include "include/easysocket.hpp"

using std::string;

string test_respond(string _data){
    std::cout << _data << std::endl;

    return 
        "HTTP/2 200 OK\n"
        "content-type: text/html\n\n"
        "<!DOCTYPE html>\n"
        "<head></head>\n"
        "<body>\n"
        "<h1>HELLO</h1>\n"
        "<p>Woohoo it works!</p>\n"
        "</body>\n";
}

int main(){
    using EasySocket::Server;
    using EasySocket::PreHosts;

    Server s = Server();
    s.connect("0.0.0.0", "6555");
    // s.respond_with("hello!\n");
    s.respond_with(&test_respond);
    s.close();
    return 0;
}