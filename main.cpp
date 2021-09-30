#include <iostream>
#include <vector>
#include <iterator>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sstream>

#include "spdlog/spdlog.h"

class HTTPServer
{
    private:
        void parse(std::string buffer)
        {
            spdlog::info("parse() = {0}", buffer);
        }

    public:
        int port = 0, threads = 0;

        void run()
        {
            spdlog::info("Starting server on port {0:d} and on {1:d} threads.\n", port, threads);

            int tcp_socket = 0;
            int current_connection = 0;

            struct sockaddr_in server_address = { 0 };
            std::string recv_buffer = "";
            recv_buffer.resize(1024);

            tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

            server_address.sin_family        = AF_INET;
            server_address.sin_addr.s_addr   = htonl(INADDR_ANY);
            server_address.sin_port          = htons(port);

            struct timeval tv;
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            setsockopt(tcp_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

            bind(tcp_socket, (struct sockaddr*)&server_address, sizeof(server_address));
            listen(tcp_socket, 0);

            while(true)
            {
                current_connection = accept(tcp_socket, (struct sockaddr*)NULL, NULL);
                if (current_connection == -1)
                    continue;

                while (true)
                {
                    // status возвращает либо кол-во прочитанных байтов, либо -1 (для будущего получения ошибки).
                    int status = recv(current_connection, &recv_buffer[0], recv_buffer.max_size(), 0);

                    // если status == 0 то не было ошибки, но и данных передано не было, только пустой tcp пакет.
                    if (status == -1 || status == 0)
                        break;

                    spdlog::info("Bytes read: {0}.", status);
                    //parse(recv_buffer);
                    
                    std::string html_page = "<!DOCTYPE html><html><body><h1>My First Heading</h1><p>My first paragraph.</p></body></html>";

                    std::ostringstream send_buffer;
                    send_buffer << "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " << html_page.length() << "\r\n\r\n" << html_page;
                    
                    write(current_connection, send_buffer.str().data(), send_buffer.str().length());
                    spdlog::info("Sent response!");
                    
                    recv_buffer.clear();
                    recv_buffer.resize(status + 1024);
                }

                spdlog::info("Closing connection.");
                close(current_connection);
            }

            // Unreachable code пока что.
            spdlog::info("Closing tcp_socket.");
            close(tcp_socket);
        }
};

int main()
{
    HTTPServer server;

    server.port = 9090;
    server.threads = 1;
    
    server.run();

    spdlog::info("Exiting.");
    return 0;
}