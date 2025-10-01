#pragma once

#include "main.h"

#include <string>

namespace CLI
{
class App;
} // namespace CLI

// Giao diện chung cho mọi backend gửi thông báo.
// Mỗi backend chịu trách nhiệm cấu hình tham số CLI và thực thi việc gửi
// thông báo tương ứng với nền tảng mà nó hỗ trợ.
class NotificationBackend
{
public:
    virtual ~NotificationBackend() = default;

    // Trả về tên duy nhất của backend, dùng để khai báo subcommand.
    [[nodiscard]] virtual std::string name() const = 0;

    // Trả về mô tả ngắn gọn cho backend để hiển thị trong CLI.
    [[nodiscard]] virtual std::string description() const = 0;

    // Đăng ký toàn bộ tham số CLI cần thiết cho backend.
    virtual void configure_cli(CLI::App &subcommand, Notification &notification) const = 0;

    // Gửi thông báo với dữ liệu đã được thu thập từ CLI.
    virtual void send(const Notification &notification) const = 0;
};

