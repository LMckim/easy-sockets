
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

map<EasySocket::PreHosts, string> prehosts = {
    {EasySocket::PreHosts::LOCAL, "127.0.0.1"},
    {EasySocket::PreHosts::ANY, "0.0.0.0"}  
};
/////////////////// BASE ///////////////////
void EasySocket::EasySocket::set_last_error(){
    this->last_err = strerror(errno);
    std::cout << this->last_err << std::endl;
    if(this->raise_exception) this->raise();
}
void EasySocket::EasySocket::set_last_error(string error){
    this->last_err = error;
    if(this->raise_exception) this->raise(error);
}
void EasySocket::EasySocket::raise(){
    throw this->last_err;
}
void EasySocket::EasySocket::raise(string _except){
    throw this->last_err + " " + _except;
}
void EasySocket::EasySocket::set_raise_exceptions(bool _raise){
    this->raise_exception = _raise;
}
void EasySocket::EasySocket::print_last_error(){
    std::cout << this->last_err << std::endl;
}
void EasySocket::EasySocket::close(){
    if(this->connected){
        ::close(this->fd);
        this->connected = false;
    }
}
void EasySocket::EasySocket::set_domain(Domain _domain){
    if(!this->connected) this->domain = _domain;
    else{
        // TODO: error
        this->set_last_error("Cannot set domain when already connected");
    }
}
void EasySocket::EasySocket::set_protocol(Protocol _protocol){
    if(!this->connected) this->protocol = _protocol;
    else{
        // TODO: error
        this->set_last_error("Cannot set protocol when already connected");
    }
}
void EasySocket::EasySocket::set_send_buf(uint16_t _size){
    if(!this->connected) this->send_buf_size = _size;
    else{
        // TODO: error
        this->set_last_error("Cannot change send buffer sizewhen already connected");
    }
}
void EasySocket::EasySocket::set_recv_buf(uint16_t _size){
    if(!this->connected) this->recv_buf_size = _size;
    else{
        // TODO: error
        this->set_last_error("Cannot change recv buffer sizewhen already connected");
    }
}
/////////////////// CLIENT /////////////////// 
void EasySocket::Client::connect(){
    this->connect(this->host, this->port);
}
void EasySocket::Client::connect(PreHosts _host, string _port){
    this->connect(prehosts[_host], _port);
}
void EasySocket::Client::connect(string _host, string _port){
    if(_port == "0"){
        // TODO: error
        this->set_last_error();
        return;
    }
    int res;
    int yes = 1;
    // TODO: add protocol stuff?
    this->fd = ::socket(this->domain, this->protocol, 0);
    if(this->fd == -1){
        // TODO: error
        this->set_last_error("bind?");
    }

    // options applications
    if(this->reuse){
        res = ::setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
        if(res == -1){
            this->set_last_error();
            return;
        }
    }
    // UDP is connectionless so we stop here
    if(this->protocol == Protocol::UDP) return;

    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = this->domain;
    // TODO: Allow port to be set via service?
    addr.sin_port = htons(std::stoi(_port));

    res = inet_pton(this->domain, _host.c_str(), &addr.sin_addr);
    if(res <= 0){
        // TODO: error
        this->set_last_error();
        return; // ??
    }
    // if we're using a TCP socket
    if(this->protocol == Protocol::TCP){
        res = ::connect(this->fd, (sockaddr *)&addr, sizeof(sockaddr));
        if(res == -1){
            // TODO: error
            this->set_last_error();
            return;
        }
    // UDP socket setup
    }else if(this->protocol == Protocol::UDP){
        // TODO:
        res = ::bind(this->fd, (sockaddr *)&addr, (socklen_t)sizeof(addr));
        if(res == -1){
            this->set_last_error();
            return;
        }

    }else{
        // TODO: error
    }
}
void EasySocket::Client::bind(){
    this->bind(this->port);
}
void EasySocket::Client::bind(string _port){
    this->port = _port;
    this->connect(this->host, _port);
}
void EasySocket::Client::set_sendto(string _host, string _port){
    this->host = _host;
    this->port = _port;
}
void EasySocket::Client::send(string _data){
    // TODO: handle for UDP
    int res, len = _data.size(), offset = 0;
    // TODO: hmmmmm probably shouldnt be allocating each send
    char send_buf[this->send_buf_size];
    memset(send_buf, '\0', sizeof(send_buf));
    strcpy(send_buf, _data.c_str());

    if(this->protocol == Protocol::TCP){
        // TODO: flags
        // TODO: full send
        res = ::send(this->fd, send_buf, sizeof(send_buf), 0);
        if(res == -1){
            this->set_last_error();
        }
    }else if(this->protocol == Protocol::UDP){
        std::cout << "udp\n";
        sockaddr_in addr;
        memset(&addr, 0, sizeof(sockaddr_in));
        addr.sin_family = this->domain;
        addr.sin_port = htons(std::stoi(this->port));
        addr.sin_addr.s_addr = INADDR_ANY;

        res = ::sendto(this->fd, send_buf, sizeof(send_buf), MSG_CONFIRM, (sockaddr *)&addr, sizeof(sockaddr));
        if(res == -1){
            this->set_last_error("send to");
        }
    }   
}
string EasySocket::Client::recv(){
    // TODO: handle for UDP
    int res;
    char recv_buf[this->recv_buf_size];
    memset(recv_buf, '\0', sizeof(recv_buf));
    // TODO: flags
    res = ::recv(this->fd, recv_buf, sizeof(recv_buf), 0);
    // connection closed
    if(res == 0){
        // TODO: handle
    // error
    }else if(res == -1){
        this->set_last_error();
    }
    return string(recv_buf);
}
/////////////////// SERVER ///////////////////
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
        std::cout << "FUCK1 " << res << std::endl;
        exit(0);
    }
    // cycle through connections for a good one
    for(p = servinfo; p != nullptr; p = p->ai_next){
        this->fd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(this->fd == -1){
            this->set_last_error();
            std::cout << "FUCK2" << std::endl;
        }
        // TODO: Error check
        res = ::setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
        res = ::bind(this->fd, p->ai_addr, p->ai_addrlen);
        if(res == -1){
            this->set_last_error();
            std::cout << "FUCK3 " << res << std::endl;
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
// void respond_with(std::function<void(string)>&_func){

// }
string EasySocket::Server::recv(){

}
void EasySocket::Server::recv_into(string &_str){

}
void EasySocket::Server::recv_into(char *_buf, const int _buf_len){

}
