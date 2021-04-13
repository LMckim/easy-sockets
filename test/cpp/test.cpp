#include <iostream>

#include "include/easysocket.hpp"

int main(){
    using EasySocket::Domain;
    using EasySocket::Protocol;
    using EasySocket::PreHosts;
    using EasySocket::EasySocket_Client;

    try{
        EasySocket_Client client = EasySocket_Client();
        client.connect(PreHosts::LOCAL, "6555");
        for(int i=0; i < 10; i++){
            client.send("Hello World! "+std::to_string(i)+'\n');
        }
        client.close();

        EasySocket_Client client_u = EasySocket_Client();
        client_u.set_protocol(Protocol::UDP);
        client_u.bind("6556");
        client_u.send("Hello from UDP!");
    }catch(std::string e){
        std::cout << e << std::endl;
    }
    return 0;
    
}