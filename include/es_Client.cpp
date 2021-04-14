
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