#pragma once

#include <map>
#include <string>
#include <sys/socket.h>

#define MAX_TRANSMISSION_UNIT 1500

namespace EasySocket{
    using std::map;
    using std::string;

    enum Domain{
        UNIX = AF_UNIX,
        IPV4 = AF_INET,
        IPV6 = AF_INET6,
        UNSPECIFIED = AF_UNSPEC
    };

    enum Protocol{
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM
    };

    enum PreHosts{
        LOCAL = 0,
        ANY = 1
    };

    class EasySocket{
    public:
    EasySocket(){}
    EasySocket(string _host, string _port) : host{_host}, port{port}{}
    virtual ~EasySocket(){}
    virtual void connect() = 0;
    virtual void connect(string _host, string _port) = 0;
    virtual void connect(PreHosts _host, string _port) = 0;
    void close();
    virtual void send(string _data) = 0;
    virtual string recv() = 0;

    void set_domain(Domain _domain);
    void set_protocol(Protocol _protocol);
    void set_send_buf(uint16_t _size);
    void set_recv_buf(uint16_t _size);
    void print_last_error();

    protected:
        int fd;
        bool connected = false;
        string last_err;

        string host = "127.0.0.1";
        string port = "0";

        Domain domain = Domain::IPV4;
        Protocol protocol = Protocol::TCP;
        uint16_t send_buf_size = MAX_TRANSMISSION_UNIT;
        uint16_t recv_buf_size = MAX_TRANSMISSION_UNIT;

        // options
        bool reuse = true;

        void set_last_error();
        void set_last_error(string error);
    };
    class EasySocket_Client : public EasySocket{
    public:
        void connect();
        void connect(string _host, string _port);
        void connect(PreHosts _host, string _port);
        void send(string _data);
        string recv();
    };
    class EasySocket_Server : public EasySocket{

    };
}