// C++ libraries
#include <iostream>
#include <iterator>
#include <sstream>
#include <queue>
#include <mutex>
#include <thread>
#include <chrono>
#include <fstream>
#include <filesystem>

// C library
#include <unistd.h>
#include <signal.h>

// Linux-specific libraries
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>

// Logging library
#include "spdlog/spdlog.h"

// HTTP Status Codes
#include "HttpStatusCodes_C++11.h"

#include "helpers.h"

std::queue<int> thread_work;
std::mutex queue_lock;

const std::string SERVER_ROOT = "/var/www/html";
int tcp_socket = 0;

class ThreadWorker
{
    private:
        std::string replace_all(std::string data, const std::string match, const std::string replace) 
        {
            std::string final_string;

            while(true)
            {
                std::ostringstream tmp_string;
                size_t pos = data.find(match);

                if (pos == std::string::npos)
                    break;

                for (int i = 0; i < pos; i++)
                {
                    tmp_string << data[i];
                }

                tmp_string << replace;

                for (int i = pos + match.length(); i < data.length(); i++)
                {
                    tmp_string << data[i];
                }

                data = tmp_string.str();
                pos = data.find(match);
            }

            if (final_string.length() == 0)
                return data;
            else
                return final_string;
        }

        std::vector<std::string> split(std::string data, const std::string match)
        {
            std::vector<size_t> found_positions = {};
            std::vector<std::string> final_vector = {};

            size_t current_position = 0;
            while (true)
            {
                size_t result = data.find(match, current_position);

                if (result == std::string::npos)
                    break;

                found_positions.push_back(result);
                current_position = result + 1;
            }

            if (found_positions.size() != 0)
            {
                int last_index = 0;
                for (int i = 0; i < found_positions.size() + 1; i++)
                {
                    std::string match_string = "";
                    std::ostringstream string_stream;

                    int end_index = found_positions[i];
                    if (i == found_positions.size())
                        end_index = data.length();

                    for (int k = last_index; k < end_index; k++)
                    {
                        string_stream << data[k];
                    }

                    last_index = found_positions[i] + match.length();
                    match_string = string_stream.str();
                    final_vector.push_back(match_string);
                }
                return final_vector;
            }

            return {};
        }

        bool starts_with(const std::string data, const std::string match)
        {
            if (data.length() < match.length())
                return false;

            for (int i = 0; i < match.length(); i++)
            {
                if (data[i] != match[i])
                    return false;
            }

            return true;
        }

        httprequest_t parse(std::string buffer)
        {
            std::vector<std::string> lines = split(buffer, "\r\n");
            httprequest_t request;

            if (lines.size() == 0)
            {
                return request;
            }

            std::vector<std::string> command = split(lines[0], " ");
            std::ostringstream debug_info;
            debug_info << "[";

            for (int i = 1; i < lines.size(); i++)
            {
                debug_info << "\'" << lines[i] << "', ";
            }

            debug_info << "]";
            spdlog::info("Received request with type \"{0}\" and argument \"{1}\" and HTTP version is \"{2}\" , also options are: {3}", command[0], command[1], command[2], debug_info.str());

            pop_front(lines);

            request.type = command[0];
            request.argument = command[1];
            request.version = command[2];
            request.options = lines;

            return request;
        }

        std::string form_response(httprequest_t request)
        {
            spdlog::debug("argument = {0}", request.argument);
            if (request.argument.find('?') != std::string::npos)
                request.argument = split(request.argument, "?")[0];

            request.argument = decode_url(request.argument);

            httpresponse_t response;

            std::error_code fs_error; // Намеренно неиспользуемая переменная чтобы избежать поимки исключения.
            std::filesystem::path fs_file = std::filesystem::weakly_canonical(std::string(SERVER_ROOT + request.argument), fs_error);

            spdlog::info("Requested file \"{0}\"", fs_file.native());

            if (starts_with(fs_file, SERVER_ROOT))
            {
                if (std::filesystem::exists(fs_file))
                {
                    if (std::filesystem::is_directory(fs_file))
                        fs_file /= "index.html";

                    std::ifstream requested_file(fs_file);
                    
                    if (!requested_file.is_open())
                    {
                        switch (errno)
                        {
                            case ENOENT:
                                if (fs_file.filename() == "index.html")
                                    response.code = 403;
                                else
                                    response.code = 404;
                                break;

                            case EACCES:
                                response.code = 403;
                                break;
                            
                            default:
                                response.code = 418;
                                break;
                        }
                    }
                    else if (request.type == "GET")
                    {                
                        response.code = 200;

                        uintmax_t requested_file_size = std::filesystem::file_size(fs_file);
                        response.options.push_back("Content-Length: " + std::to_string(requested_file_size));

                        if (requested_file_size > 4 * 1024 * 1024) // Если размер файла превышает 4МБ, то отправлять частями.
                        {

                        }
                        else
                        {
                            requested_file.seekg(0, std::ios::end);
                            size_t size = requested_file.tellg();

                            std::string buffer;
                            buffer.resize(size);
                            
                            requested_file.seekg(0);
                            requested_file.read(&buffer[0], size); 

                            response.body = buffer;
                        }
                    }
                    else if (request.type == "HEAD")
                    {
                        response.code = 200;

                        uintmax_t requested_file_size = std::filesystem::file_size(fs_file);
                        response.options.push_back("Content-Length: " + std::to_string(requested_file_size));
                    }
                    else
                    {
                        response.code = 405;
                    }
                }
                else
                {
                    response.code = 404;
                }
            }
            else
            {
                response.code = 403;
            }

            response.description = HttpStatus::reasonPhrase(response.code);
            spdlog::info("response = {0}", replace_all(response.to_str(), "\r\n", "\\r\\n"));

            return response.to_str();
        }

        unsigned char from_hex (unsigned char ch) 
        {
            if (ch <= '9' && ch >= '0')
                ch -= '0';
            else if (ch <= 'f' && ch >= 'a')
                ch -= 'a' - 10;
            else if (ch <= 'F' && ch >= 'A')
                ch -= 'A' - 10;
            else 
                ch = 0;
            return ch;
        }

        std::string decode_url(std::string str) 
        {
            std::string result;
            size_t i;

            for (i = 0; i < str.length(); ++i)
            {
                if (str[i] == '+')
                {
                    result += ' ';
                }
                else if (str[i] == '%' && str.length() > i + 2)
                {
                    const unsigned char ch1 = from_hex(str[i+1]);
                    const unsigned char ch2 = from_hex(str[i+2]);
                    const unsigned char ch = (ch1 << 4) | ch2;
                    result += ch;
                    i += 2;
                }
                else
                {
                    result += str[i];
                }
            }
            return result;
        }

    public:
        void run()
        {
            std::string recv_buffer = "", send_buffer = "";
            recv_buffer.resize(4096);
            int current_connection = 0, status = 0;
            httprequest_t request;

            while (true)
            {
                queue_lock.lock(); // Блокирующая операция, не может провалиться, иначе дедлок в потоке.
                if (thread_work.size() == 0)
                {
                    queue_lock.unlock();
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }

                current_connection = thread_work.front();
                thread_work.pop();
                queue_lock.unlock();

                status = recv(current_connection, &recv_buffer[0], recv_buffer.size(), 0);

                // если status == 0 то не было ошибки, но и данных передано не было, только пустой tcp пакет.
                if (status == 0 || status == -1)
                {
                    if (status == -1)
                    {
                        spdlog::warn("recv() returned -1. Error code: {0} ({1})", errno, strerror(errno));
                    }

                    spdlog::info("Closing connection.");
                    shutdown(current_connection, SHUT_RDWR);
                    close(current_connection);
                    continue;
                }

                spdlog::info("Bytes read: {0}.", status);

                request = parse(recv_buffer);

                if (!request.type.empty())
                {
                    send_buffer = form_response(request);
                
                    send(current_connection, send_buffer.data(), send_buffer.length(), 0);
                    spdlog::info("Sent response!");
                }

                spdlog::info("Closing connection.");
                shutdown(current_connection, SHUT_RDWR);
                close(current_connection);

                recv_buffer.clear();
                recv_buffer.resize(4096);
            }
        }
};

void thread_worker()
{
    ThreadWorker worker;
    worker.run();
}

void start_server(int port, int threads)
{
    spdlog::info("Starting server on port {0:d} and on {1:d} threads.\n", port, threads);

    struct sockaddr_in server_address = { 0 };

    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family        = AF_INET;
    server_address.sin_addr.s_addr   = htonl(INADDR_ANY);
    server_address.sin_port          = htons(port);

    bind(tcp_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    listen(tcp_socket, 10);

    while (true)
    {
        int current_connection = accept(tcp_socket, (struct sockaddr*)NULL, NULL);
        
        if (current_connection == -1)
        {
            spdlog::error("accept() return {0}. Error code: \"{1}\"", current_connection, strerror(errno));
            continue;
        }

        queue_lock.lock();
        thread_work.push(current_connection);
        queue_lock.unlock();
    }
}

int main()
{
    spdlog::set_pattern("[%H:%M:%S] [%l] [thread %t] %v");

    int port = 9000;
    int threads = std::thread::hardware_concurrency() - 2;

    std::vector<std::thread> thread_workers;
    for (int i = 0; i < 2; i++)
    {
        thread_workers.push_back(std::thread(thread_worker));
        spdlog::info("Created thread {0}", i);
    }

    start_server(port, threads);

    return 0;
}
