
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "include/easysocket.hpp"

using std::map;
using std::string;
using std::thread;
using std::vector;

void EasySocket::Server::connect(){
    this->connect(this->host, this->port);
}
void EasySocket::Server::connect(PreHosts _host, string _port){
    this->connect(prehosts[_host], _port);
}
void EasySocket::Server::connect(string _host, string _port){
    addrinfo hints, *servinfo, *p;
    int yes = 1, res;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = this->protocol;
    hints.ai_flags = AI_PASSIVE;

    res = getaddrinfo(NULL, _port.c_str(), &hints, &servinfo);
    if(res !=0){
        this->set_last_error();
        exit(0);
    }
    // cycle through connections for a good one
    for(p = servinfo; p != nullptr; p = p->ai_next){
        this->fd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(this->fd == -1){
            this->set_last_error();
        }
        // TODO: Error check
        res = ::setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
        res = ::bind(this->fd, p->ai_addr, p->ai_addrlen);
        if(res == -1){
            this->set_last_error();
            ::close(this->fd);
            continue;
        }
        break;
    }
    // connect and listen to the socket
    res = ::listen(this->fd, this->backlog);
}
void EasySocket::Server::send(int _fd, string _data){
    int sent = 0;
    while(sent < _data.length()){
        sent = ::send(_fd, _data.c_str(), _data.length(), 0);
        if(sent == -1){
            this->set_last_error();
        }
    }
    ::close(_fd);

}
void EasySocket::Server::respond_with(string _data){
    vector<thread> prev_conns;
    char recv_buf[this->recv_buf_size];
    int last_bytes;
    memset(recv_buf, '\0', this->recv_buf_size);
    sockaddr_storage theirs;
    socklen_t t_size;
    while(true){
        // TODO: Flags, err
        int in_fd = ::accept(this->fd, (sockaddr*)&theirs, (socklen_t*)&t_size);
        last_bytes = ::recv(this->fd, recv_buf, this->recv_buf_size, 0);

        prev_conns.push_back(thread(
            &Server::send,
            this,
            in_fd,
            _data
        ));
        memset(recv_buf, '\0', this->recv_buf_size);
        std::cout << prev_conns.size() << std::endl;
        auto itr = prev_conns.begin();
        while(itr != prev_conns.end()){
            if((*itr).joinable()){
                (*itr).join();
                itr = prev_conns.erase(itr);
            }else itr++;
        }
    }
}
void EasySocket::Server::respond_with(string (*_func)(string)){
    vector<thread> prev_conns;
    char recv_buf[this->recv_buf_size];
    int last_bytes;
    memset(recv_buf, '\0', this->recv_buf_size);
    sockaddr_storage theirs;
    socklen_t t_size;
    while(true){
        // TODO: Flags, err
        int in_fd = ::accept(this->fd, (sockaddr*)&theirs, (socklen_t*)&t_size);
        last_bytes = ::recv(in_fd, recv_buf, this->recv_buf_size, 0);

        prev_conns.push_back(thread(
            &Server::send,
            this,
            in_fd,
            _func(string(recv_buf))
        ));
        memset(recv_buf, '\0', this->recv_buf_size);
        auto itr = prev_conns.begin();
        while(itr != prev_conns.end()){
            if((*itr).joinable()){
                (*itr).join();
                itr = prev_conns.erase(itr);
            }else itr++;
        }
    }
}
void respond_with(std::function<void(string)>&_func){
    
}
string EasySocket::Server::recv(){

}
void EasySocket::Server::recv_into(string &_str){

}
void EasySocket::Server::recv_into(char *_buf, const int _buf_len){

}