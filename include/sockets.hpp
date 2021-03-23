#pragma once

#include <arpa/inet.h> // inet_atop
#include <sys/socket.h> // socket, connect
#include <netdb.h> // getaddrinfo
#include <string.h>
#include <unistd.h> // close

#include <iostream>
#include <functional>
#include <thread>
#include <mutex>
#include <string>

#define ES_BACKLOG 10
#define ES_BUF_DEFAULT 8192


namespace EasySocket{
    using std::function;
    using std::lock_guard;
    using std::mutex;
    using std::string;

    // private namespace
    namespace{
        int YES = 1;
        // int optlen;
        void *get_in_addr(sockaddr *sa){
            if (sa->sa_family == AF_INET) {
                return &(((struct sockaddr_in*)sa)->sin_addr);
            }
            return &(((struct sockaddr_in6*)sa)->sin6_addr);
        }
    }

    enum SOCK_DOMAIN{
        ANY = AF_UNSPEC,
        UNIX = AF_UNIX,
        IPV4 = AF_INET,
        IPV6 = AF_INET6,
        BLUETOOTH = AF_BLUETOOTH
    };
    enum SOCK_TYPE{
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM
    };
    enum SOCK_PROTO{
        LOCAL = PF_UNIX,
        INTERNET = PF_INET
    };

    class base_socket{
        public:
        base_socket(){}
        base_socket(string _host, string _port) : host{_host}, port{_port} {}
        virtual ~base_socket(){ if(this->connected) ::close(this->sockfd); }
        void set_host(string _host, string _port){
            this->host = _host; this->port = _port;
        }
        void close(){ 
            if(this->connected){
                ::close(this->sockfd); 
            }
            this->close_connection = true;
        }
        virtual bool connect() = 0;

        void set_domain(SOCK_DOMAIN domain){ this->domain = domain; }
        void set_type(SOCK_TYPE type){ this->type = type; }

        string last_send(){ return string(this->send_buf); }
        string last_recv(){ return string(this->recv_buf); }

        bool accepts_connections(){ 
            socklen_t len;
            int val = getsockopt(this->sockfd, SOL_SOCKET, SO_ACCEPTCONN, &YES, &len);
            return val == 1 ? true : false;
        }
        // TODO: implement set/get sockopts
        // string bound_to_device(){}
        // bool broadcast(){}
        // bool debug(){}
        // bool error(){}
        // bool no_route(){}
        // bool keep_alive(){}
        // bool linger(){}
        // bool protocol(){}
        // int recv_buffer(){}
        // int recv_buffer_force(){}
        // int timeouts(){}
        // bool reusable(){}
        // int send_buffer(){}
        // int send_buffer_force(){}
        // int timestamp(){}
        // int type(){}

        protected:
        bool connected = false;
        int sockfd;
        bool close_connection = false;
        int backlog = ES_BACKLOG;

        string host;
        string port;

        char recv_buf[ES_BUF_DEFAULT] = {'\0'};
        char send_buf[ES_BUF_DEFAULT] = {'\0'};

        int domain = SOCK_DOMAIN::ANY;
        int type = SOCK_TYPE::TCP;
    };

    class base_client : public base_socket{
        public:
        base_client(){}
        base_client(string _host, string _port) : base_socket(_host, _port){}
        bool connect(){
            int rv;
            addrinfo hint, *server_info, *ad_itr;

            memset(&hint, 0, sizeof(hint));
            hint.ai_family = this->domain;
            hint.ai_socktype = this->type;

            rv = getaddrinfo(
                this->host.c_str(), 
                this->port.c_str(), 
                &hint,
                &server_info
            );
            if(rv != 0){ std::cout << "RV ERROR" << std::endl; }
            // TODO: error check
            for(ad_itr=server_info; ad_itr != nullptr; ad_itr = ad_itr->ai_next){
                if((this->sockfd = socket(
                    ad_itr->ai_family,
                    ad_itr->ai_socktype,
                    ad_itr->ai_protocol
                )) == -1) continue;
                
                if(::connect(
                    this->sockfd,
                    ad_itr->ai_addr,
                    ad_itr->ai_addrlen
                ) == -1){
                    ::close(this->sockfd);
                    continue;
                    // TODO: error checking
                }
                break; // weve connected
            }
            // TODO: Error check
            if(ad_itr == nullptr){
                std::cout << "Could not connect" << std::endl;
                exit(1);
            }
            freeaddrinfo(server_info);
            return true;
        }

        void ts_send(string data){
            lock_guard<mutex>(this->send_lock);
            memset(this->send_buf, '\0', sizeof(this->send_buf));
            strcpy(this->send_buf, data.c_str());
            ssize_t res = ::send(
                this->sockfd,
                this->send_buf,
                sizeof(this->send_buf),
                0
            );
            if(res == -1){
                std::cout << "ERROR\n";
            }
            recv(this->sockfd, this->recv_buf, sizeof(this->recv_buf), 0);
            // TODO: error check
        }
        protected:

        private:
        int last_bytes;
        sockaddr self_addr;
        mutex send_lock;
    };

    class base_server : public base_socket{
        public:
        base_server(){}
        base_server(string _host, string _port): base_socket(_host, _port){}

        bool connect(){
            addrinfo hint, *server_info, *ad_itr;
            int rv;

            memset(&hint, 0, sizeof(hint));
            hint.ai_family = this->domain;
            hint.ai_socktype = this->type;
            hint.ai_flags = AI_PASSIVE;

            rv = getaddrinfo(NULL, this->port.c_str(), &hint, &server_info);
            if(rv != 0){ std::cout << "RV ERROR" << std::endl; }
            // TODO: Error check
            for(ad_itr = server_info; ad_itr != nullptr; ad_itr = ad_itr->ai_next){
                if((this->sockfd = socket(
                    ad_itr->ai_family,
                    ad_itr->ai_socktype,
                    ad_itr->ai_protocol
                )) == -1) continue;

                // TODO: introduce options
                setsockopt(
                    this->sockfd,
                    SOL_SOCKET,
                    SO_REUSEADDR,
                    &YES,
                    sizeof(YES)
                );
                // TODO: Error checks
                
                if(::bind(
                    this->sockfd,
                    ad_itr->ai_addr,
                    ad_itr->ai_addrlen
                ) == -1){
                    std::cout << "Error during bind\n";
                    ::close(this->sockfd);
                    continue;
                    // TODO: error checking
                }
                break; // weve connected
            }
            if(ad_itr == nullptr){
                std::cout << "Could not connect" << std::endl;
                exit(1);
            }
            freeaddrinfo(server_info);

            ::listen(this->sockfd, this->backlog);
            return true;
        }
        void await_connections(string response){
            socklen_t sin_size;
            sockaddr_storage conn_addr;
            int incoming_fd;
            char incoming_addr_buf[INET6_ADDRSTRLEN];

            while(!this->close_connection){
                sin_size = sizeof(sin_size);
                std::cout << "Await\n";
                incoming_fd = accept(this->sockfd, (sockaddr *)&conn_addr, &sin_size);
                if(incoming_fd == -1){
                    // std::cout << "ERROR: " << incoming_fd << std::endl;
                    continue; // TODO: Error handle
                }
                inet_ntop(
                    conn_addr.ss_family,
                    get_in_addr((sockaddr *)&conn_addr),
                    incoming_addr_buf,
                    sizeof(incoming_addr_buf)
                );
                memset(this->recv_buf, '\0', sizeof(this->recv_buf));
                ::recv(incoming_fd, this->recv_buf, sizeof(this->recv_buf), 0);
                ::send(incoming_fd, response.c_str(), response.length(), 0);
                ::close(incoming_fd);
            }
        };
        void await_connections_callback(std::function<string(string)> &func){
            socklen_t sin_size;
            sockaddr_storage conn_addr;
            int incoming_fd;
            char incoming_addr_buf[INET6_ADDRSTRLEN];

            while(!this->close_connection){
                sin_size = sizeof(sin_size);
                // std::cout << "Await\n";
                incoming_fd = accept(this->sockfd, (sockaddr *)&conn_addr, &sin_size);
                if(incoming_fd == -1){
                    // std::cout << "ERROR: " << incoming_fd << std::endl;
                    continue; // TODO: Error handle
                }
                inet_ntop(
                    conn_addr.ss_family,
                    get_in_addr((sockaddr *)&conn_addr),
                    incoming_addr_buf,
                    sizeof(incoming_addr_buf)
                );
                ::recv(this->sockfd, &this->recv_buf, sizeof(char), 0);

                string response = func(string(this->recv_buf));
                ::send(incoming_fd, response.c_str(), response.length(), 0);
                ::close(incoming_fd);
            }
        };

    };
}





