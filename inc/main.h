#include <iostream>
#include <CLI/CLI.hpp>
#include <string>
#include <string_view>
#include <chrono>
#include <ctime>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <vector>
#include <string>
#include <cerrno>
#include <cstring>
#include <cstdlib>

#if defined(__linux__)
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

using namespace std;

// Basic structure of a message
struct Notification {
    std::string id;
    std::string topic;
    std::time_t created_at;

    struct Payload {
        std::string title;
        std::string body;
        std::string image_url;
    } payload; // biến thành viên
};

// In chi tiết thông báo theo định dạng đẹp
void print_notification_details(const Notification& options);
void send_local_linux_notification(Notification notification);
