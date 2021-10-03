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
#include <iomanip>

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

#include "str_helpers.h"
#include "helpers.h"

std::queue<int> thread_work;
std::mutex queue_lock;

int PORT = 0, MAX_THREADS = 0;
std::string SERVER_ROOT = "";

class ThreadWorker
{
    private:
        httprequest_t parse(std::string buffer)
        {
            std::vector<std::string> lines = str_helpers::split(buffer, "\r\n");
            httprequest_t request;

            /*if (lines.size() == 0)
            {
                return request;
            }*/

            std::vector<std::string> command = str_helpers::split(lines[0], " ");
            std::ostringstream debug_info;
            debug_info << "[";

            for (int i = 1; i < lines.size(); i++)
            {
                debug_info << "\'" << lines[i] << "', ";
            }

            debug_info << "]";
            spdlog::debug("Received request with type \"{0}\" and argument \"{1}\" and HTTP version is \"{2}\" , also options are: {3}", command[0], command[1], command[2], debug_info.str());

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
                request.argument = str_helpers::split(request.argument, "?")[0];

            request.argument = decode_url(request.argument);

            httpresponse_t response;

            std::error_code fs_error; // Намеренно неиспользуемая переменная чтобы избежать поимки исключения.
            std::filesystem::path fs_file = std::filesystem::weakly_canonical(std::string(SERVER_ROOT + request.argument), fs_error);

            spdlog::debug("Requested file \"{0}\"", fs_file.native());

            if (str_helpers::starts_with(fs_file, SERVER_ROOT))
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
                        response.options.push_back("Content-Type: " + get_content_type(fs_file.extension()));

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
                        response.options.push_back("Content-Type: " + get_content_type(fs_file.extension()));
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

            response.options.push_back("Date: " + get_http_date());
            response.description = HttpStatus::reasonPhrase(response.code);
            spdlog::debug("response = {0}", str_helpers::replace_all(response.to_str(), "\r\n", "\\r\\n"));

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

        std::string get_content_type(std::string file_extension)
        {
            if (file_extension == ".html")
                return "text/html";
            else if (file_extension == ".jpg" || file_extension == ".jpeg")
                return "image/jpeg";
            else if (file_extension == ".png")
                return "image/png";
            else if (file_extension == ".js")
                return "application/javascript";
            else if (file_extension == ".css")
                return "text/css";
            else if (file_extension == ".gif")
                return "image/gif";
            else if (file_extension == ".swf")
                return "application/x-shockwave-flash";
            
            return "application/octet-stream";
        }

        std::string get_http_date()
        {
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);

            std::stringstream ss;
            ss << std::put_time(std::gmtime(&in_time_t), "%a, %d %b %Y %X GMT"); // Mon, 30 Sep 2021 00:00:00 GMT
            return ss.str();
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
                        spdlog::debug("recv() returned -1. Error code: {0} ({1})", errno, strerror(errno));
                    }

                    spdlog::debug("Closing connection.");
                    shutdown(current_connection, SHUT_RDWR);
                    close(current_connection);
                    continue;
                }

                spdlog::debug("Bytes read: {0}.", status);

                request = parse(recv_buffer);

                if (!request.type.empty())
                {
                    send_buffer = form_response(request);
                
                    send(current_connection, send_buffer.data(), send_buffer.length(), 0);
                    spdlog::debug("Sent response!");
                }

                spdlog::debug("Closing connection.");
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

void start_server()
{
    struct sockaddr_in server_address = { 0 };

    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family        = AF_INET;
    server_address.sin_addr.s_addr   = htonl(INADDR_ANY);
    server_address.sin_port          = htons(PORT);

    bind(tcp_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    listen(tcp_socket, 10);

    while (true)
    {
        int current_connection = accept(tcp_socket, (struct sockaddr*)NULL, NULL);
        
        if (current_connection == -1)
        {
            spdlog::debug("accept() return {0}. Error code: \"{1}\"", current_connection, strerror(errno));
            continue;
        }

        queue_lock.lock();
        thread_work.push(current_connection);
        queue_lock.unlock();
    }
}

bool read_config()
{
    std::filesystem::path config_path = "/etc/httpd.conf";

    if (!std::filesystem::exists(config_path))
    {
        spdlog::critical("\"{0}\" does not exist.", config_path.native());
        return false;
    }

    if (std::filesystem::is_directory(config_path))
    {
        spdlog::critical("\"{0}\" is a directory and not file.", config_path.native());
        return false;
    }

    std::ifstream config_file(config_path);

    if (!config_file.is_open())
    {
        spdlog::critical("Failed to open \"{0}\"", config_path.native());
        return false;
    }

    config_file.seekg(0, std::ios::end);
    size_t size = config_file.tellg();

    std::string buffer;
    buffer.resize(size);
    
    config_file.seekg(0);
    config_file.read(&buffer[0], size);

    if (!config_file)
    {
        spdlog::critical("Error reading \"{0}\"", config_path.native());
        return false;
    }

    if (buffer.length() == 0)
    {
        spdlog::critical("\"{0}\" is empty.", config_path.native());
        return false;
    }

    std::vector<std::string> vec_lines = str_helpers::split(buffer, "\n");
    vec_lines.erase(std::find(vec_lines.begin(), vec_lines.end(), "")); // Удалить все пустые ячейки.

    for (int i = 0; i < vec_lines.size(); i++)
    {
        if (str_helpers::starts_with(vec_lines[i], "thread_limit"))
        {
            MAX_THREADS = std::stoul(str_helpers::replace_all(vec_lines[i], "thread_limit ", ""));
        }
        else if (str_helpers::starts_with(vec_lines[i], "document_root"))
        {
            SERVER_ROOT = str_helpers::replace_all(vec_lines[i], "document_root ", "");
        }
        else if (str_helpers::starts_with(vec_lines[i], "port"))
        {
            PORT = std::stoul(str_helpers::replace_all(vec_lines[i], "port ", ""));
        }
    }

    if (MAX_THREADS <= 0)
    {
        spdlog::critical("thread_limit not found in \"{0}\"", config_path.native());
        return false;
    }
    
    if (SERVER_ROOT == "")
    {
        spdlog::critical("document_root not found in \"{0}\"", config_path.native());
        return false;
    }
    
    if (PORT <= 0)
    {
        spdlog::critical("port not found in \"{0}\"", config_path.native());
        return false;
    }

    return true;
}

int main()
{
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%H:%M:%S] [%l] [thread %t] %v");

    if (!read_config())
    {
        spdlog::critical("Error reading config. Exiting.");
        return -1;
    }

    std::vector<std::thread> thread_workers;
    for (int i = 0; i < MAX_THREADS; i++)
    {
        thread_workers.push_back(std::thread(thread_worker));
        spdlog::info("Created thread {0}", i);
    }

    spdlog::info("Starting server on port {0:d} and on {1:d} threads.\n", PORT, MAX_THREADS);
    start_server();

    for (int i = 0; i < MAX_THREADS; i++)
    {
        thread_workers[i].join();
        spdlog::info("Joined thread {0}", i);
    }

    spdlog::info("Exiting.");
    return 0;
}
