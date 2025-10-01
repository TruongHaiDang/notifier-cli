#pragma once

#include "main.h"

#include <string>
#include <vector>

namespace CLI
{
class App;
} // namespace CLI

// Backend gửi thông báo bằng notify-send trên Linux.
class NotificationBackend final
{
public:
    NotificationBackend() = default;
    ~NotificationBackend() = default;

    NotificationBackend(const NotificationBackend &) = delete;
    NotificationBackend &operator=(const NotificationBackend &) = delete;
    NotificationBackend(NotificationBackend &&) = delete;
    NotificationBackend &operator=(NotificationBackend &&) = delete;

    [[nodiscard]] std::string name() const;
    [[nodiscard]] std::string description() const;

    void configure_cli(CLI::App &subcommand, Notification &notification) const;
    void send(const Notification &notification) const;

    void set_telegram_bot_token(std::string bot_token);

private:
#if defined(__linux__)
    static constexpr const char *kNotifySendBinary = "notify-send";

    [[nodiscard]] std::vector<std::string> build_arguments(const Notification &notification) const;
    void execute_notify_send(const std::vector<std::string> &args) const;
#endif
};

