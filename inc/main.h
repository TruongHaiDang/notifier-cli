#pragma once

#include <chrono>
#include <ctime>
#include <string>

// Đại diện cho nội dung một thông báo.
struct Notification
{
    std::string id;
    std::string topic;
    std::string bot_token;
    std::string chat_id;
    std::time_t created_at{0};

    struct Payload
    {
        std::string title;
        std::string body;
        std::string image_url;
    } payload;
};

// In chi tiết thông báo theo định dạng dễ đọc.
void print_notification_details(const Notification &options);
