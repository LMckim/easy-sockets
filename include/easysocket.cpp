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


