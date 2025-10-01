#pragma once

#include "notification_backend.h"

#include <string>
#include <vector>

namespace CLI
{
class App;
} // namespace CLI

#if defined(__linux__)
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

// Backend thông báo dành riêng cho notify-send trên Linux.
// Đây là một trong nhiều backend dự kiến sẽ xuất hiện (Telegram, Firebase
// FCM,...), tất cả đều tuân theo giao diện NotificationBackend.
class NotifySendBackend final : public NotificationBackend
{
public:
    NotifySendBackend() = default;
    ~NotifySendBackend() = default;

    NotifySendBackend(const NotifySendBackend &) = delete;
    NotifySendBackend &operator=(const NotifySendBackend &) = delete;
    NotifySendBackend(NotifySendBackend &&) = delete;
    NotifySendBackend &operator=(NotifySendBackend &&) = delete;

    // Tên backend dùng cho subcommand CLI.
    [[nodiscard]] std::string name() const override;

    // Mô tả hiển thị trong danh sách lệnh.
    [[nodiscard]] std::string description() const override;

    // Thiết lập các tham số CLI dành riêng cho backend notify-send.
    void configure_cli(CLI::App &subcommand, Notification &notification) const override;

    // Gửi thông báo đã cho bằng cách sử dụng notify-send.
    void send(const Notification &notification) const override;

private:
#if defined(__linux__)
    static constexpr const char *kNotifySendBinary = "notify-send";

    // Chuẩn bị danh sách tham số truyền cho notify-send.
    [[nodiscard]] std::vector<std::string> build_arguments(const Notification &notification) const;
    // Thực thi notify-send với tập tham số đã chuẩn bị.
    void execute_notify_send(const std::vector<std::string> &args) const;
#endif
};
