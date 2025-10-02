#pragma once

#include "main.h"

#include <string>
#include <vector>

namespace CLI
{
class App;
} // namespace CLI

// Backend gửi thông báo theo từng chế độ (local hoặc telegram).
class NotificationBackend final
{
public:
    enum class Mode
    {
        Local,
        Telegram
    };

    explicit NotificationBackend(Mode mode);
    ~NotificationBackend() = default;

    NotificationBackend(const NotificationBackend &) = delete;
    NotificationBackend &operator=(const NotificationBackend &) = delete;
    NotificationBackend(NotificationBackend &&) = delete;
    NotificationBackend &operator=(NotificationBackend &&) = delete;

    [[nodiscard]] std::string name() const;
    [[nodiscard]] std::string description() const;

    void configure_cli(CLI::App &subcommand, Notification &notification) const;
    void send(const Notification &notification) const;

private:
    Mode mode_;

#if defined(__linux__)
    static constexpr const char *kNotifySendBinary = "notify-send";

    [[nodiscard]] std::vector<std::string> build_arguments(const Notification &notification) const;
    void execute_notify_send(const std::vector<std::string> &args) const;
#endif
    static constexpr const char *kTelegramBaseUrl = "https://api.telegram.org/bot";

    [[nodiscard]] bool send_via_telegram(const Notification &notification) const;
    [[nodiscard]] std::string build_message(const Notification &notification) const;
};
