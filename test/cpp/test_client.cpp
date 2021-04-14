#include <iostream>

#include "include/easysocket.hpp"

int main(){
    using EasySocket::Domain;
    using EasySocket::Protocol;
    using EasySocket::PreHosts;
    using EasySocket::Client;

    try{
        Client client = Client();
        client.connect(PreHosts::LOCAL, "6555");
        for(int i=0; i < 10; i++){
            client.send("Hello World! "+std::to_string(i)+'\n');
        }
        client.close();

        Client client_u = Client();
        client_u.set_protocol(Protocol::UDP);
        client_u.bind("6556");
        for(int i=0; i < 10; i++){
            client_u.send("Hello World! UDP "+std::to_string(i)+'\n');
        }
        client_u.close();
    }catch(std::string e){
        std::cout << e << std::endl;
    }
    return 0;
    
}