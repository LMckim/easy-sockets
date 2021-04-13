#include "easysocket.hpp"

#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <unistd.h>

using std::map;
using std::string;

map<EasySocket::PreHosts, string> prehosts = {
    {EasySocket::PreHosts::LOCAL, "127.0.0.1"},
    {EasySocket::PreHosts::ANY, "0.0.0.0"}  
};

void EasySocket::EasySocket::set_last_error(){
    this->last_err = strerror(errno);
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
void EasySocket::EasySocket_Client::connect(){
    this->connect(this->host, this->port);
}
void EasySocket::EasySocket_Client::connect(PreHosts _host, string _port){
    this->connect(prehosts[_host], _port);
}
void EasySocket::EasySocket_Client::connect(string _host, string _port){
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
void EasySocket::EasySocket_Client::bind(){
    this->bind(this->port);
}
void EasySocket::EasySocket_Client::bind(string _port){
    this->port = _port;
    this->connect(this->host, _port);
}
void EasySocket::EasySocket_Client::set_sendto(string _host, string _port){
    this->host = _host;
    this->port = _port;
}
void EasySocket::EasySocket_Client::send(string _data){
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
string EasySocket::EasySocket_Client::recv(){
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