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
        void *get_in_addr(sockaddr *sa){
            if (sa->sa_family == AF_INET) {
                return &(((struct sockaddr_in*)sa)->sin_addr);
            }
            return &(((struct sockaddr_in6*)sa)->sin6_addr);
        }
    }
    
    class base_client{
        public:
        base_client(){}
        base_client(string _host, string _port):host{_host}, port{_port} {}
        ~base_client(){ close(this->sockfd); }
        void set_host(string _host, string _port){
            this->host = _host;
            this->port = _port;
        }
        bool connect(){
            int rv;
            addrinfo hint, *server_info, *ad_itr;

            memset(&hint, 0, sizeof(hint));
            hint.ai_family = AF_UNSPEC;
            hint.ai_socktype = SOCK_STREAM;

            rv = getaddrinfo(
                this->host.c_str(), 
                this->port.c_str(), 
                &hint,
                &server_info
            );
            if(rv == -1){ std::cout << "ERROR\n"; return false; }
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
                    close(this->sockfd);
                    continue;
                    // TODO: error checking
                }
                break; // weve connected
            }
            // TODO: Error check
            freeaddrinfo(server_info);
            return true;
        }

        void ts_send(string data){
            lock_guard<mutex>(this->send_lock);
            strcpy(this->send_buf, data.c_str());
            std::cout << "Sending: " << this->send_buf << std::endl;
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
            std::cout << "Client response: " << this->recv_buf << std::endl;
            memset(this->send_buf, '\0', sizeof(this->send_buf));
            // TODO: error check
        }
        // void send(string data){
            
        // }
        protected:

        private:
        bool connected = false;

        string host;
        string port;

        int sockfd;
        int last_bytes;
        char recv_buf[ES_BUF_DEFAULT] = {'\0'};
        char send_buf[ES_BUF_DEFAULT] = {'\0'};
        sockaddr self_addr;

        mutex send_lock;
    };

    class base_server{
        public:
        base_server(){}
        base_server(string _host, string _port): host{_host}, port{_port} {}
        void set_host(string _host, string _port){
            this->host = _host;
            this->port = _port;
        }
        void connect(){
            std::cout << "Connecting...\n";
            addrinfo hint, *server_info, *ad_itr;
            int rv;

            memset(&hint, 0, sizeof(hint));
            hint.ai_family = AF_UNSPEC;
            hint.ai_socktype = SOCK_STREAM;
            hint.ai_flags = AI_PASSIVE;

            rv = getaddrinfo(NULL, this->port.c_str(), &hint, &server_info);
            if(rv != 0){
                std::cout << "RV ERROR\n";
            }
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
                    close(this->sockfd);
                    continue;
                    // TODO: error checking
                }else std::cout << "Successful Bind\n";
                break; // weve connected
            }
            if(ad_itr == nullptr){
                std::cout << "Could not connect\n";
                exit(1);
            }else{
                // TODO: remove
                std::cout << "Successful connection\n";
            }
            freeaddrinfo(server_info);

            ::listen(this->sockfd, this->backlog);
        }
        /**
         * Just prints out any incoming connections
         **/ 
        void await_connections(string response){
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
                std::cout << "Connected from: " << incoming_addr_buf << std::endl;
                recv(incoming_fd, this->recv_buf, sizeof(this->recv_buf), 0);
                std::cout << "Recieved: " << this->recv_buf << std::endl;
                send(incoming_fd, response.c_str(), response.length(), 0);
                std::cout << "Response is: " << response << std::endl;
                close(incoming_fd);
                memset(this->recv_buf, '\0', sizeof(this->recv_buf));
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
                std::cout << "Connected from: " << incoming_addr_buf << std::endl;
                recv(this->sockfd, &this->recv_buf, sizeof(char), 0);

                string response = func(string(this->recv_buf));
                send(incoming_fd, response.c_str(), response.length(), 0);
                std::cout << "Response is: " << response << std::endl;
                close(incoming_fd);
            }
        };
        protected:

        private:
        bool close_connection = false;

        int sockfd;
        int backlog = ES_BACKLOG;

        string host;
        string port;
        
        char recv_buf[ES_BUF_DEFAULT] = {'\0'};
        char send_buf[ES_BUF_DEFAULT] = {'\0'};

    };
}





