// C++ Libraries
#include <vector>
#include <string>
#include <sstream>

// Logging library
#include "spdlog/spdlog.h"

struct connection_t
{
    int connection_socket;
    std::string recv_buffer;
};

struct httprequest_t
{
    std::string type = "";
    std::string argument = "";
    std::string version = "";
    std::vector<std::string> options;
};

struct httpresponse_t
{
    std::string version = "HTTP/1.1";
    int code;
    std::string description;
    //Так как это минимальный веб-сервер, то тут рано думать о keep-alive
    std::vector<std::string> options = {"Server: Fangtooth/1.0.0", "Connection: close"};
    std::string body;

    std::string to_str()
    {
        std::ostringstream response_stream;
        response_stream << version << " " << code << " " << description << "\r\n";

        for (int i = 0; i < options.size(); i++)
        {
            response_stream << options[i] << "\r\n";
        }

        response_stream << "\r\n" << body;
        return response_stream.str();
    }
};

template<typename T>
void pop_front(std::vector<T>& vec)
{
    assert(!vec.empty());
    vec.front() = std::move(vec.back());
    vec.pop_back();
}