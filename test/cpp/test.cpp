#include "include/easysocket.hpp"

int main(){
    using EasySocket::Domain;
    using EasySocket::Protocol;
    using EasySocket::PreHosts;
    using EasySocket::EasySocket_Client;

    EasySocket_Client client = EasySocket_Client();
    client.connect(PreHosts::LOCAL, "6555");
    client.print_last_error();
    for(int i=0; i < 10; i++){
        client.send("Hello World! "+std::to_string(i)+'\n');
    }
    client.print_last_error();
    client.close();
    return 0;
    
}