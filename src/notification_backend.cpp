#include "notification_backend.h"

#include <CLI/CLI.hpp>
#include <curl/curl.h>

#include <cerrno>
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#if defined(__linux__)
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace
{
// Ensures curl_global_init/cleanup are paired even on early returns.
class CurlGlobalGuard
{
public:
    CurlGlobalGuard()
    {
        status_ = curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    ~CurlGlobalGuard()
    {
        if (status_ == CURLE_OK)
        {
            curl_global_cleanup();
        }
    }

    [[nodiscard]] bool ok() const
    {
        return status_ == CURLE_OK;
    }

    [[nodiscard]] CURLcode status() const
    {
        return status_;
    }

private:
    CURLcode status_{CURLE_OK};
};

size_t collect_response(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    auto *buffer = static_cast<std::string *>(userdata);
    if (buffer != nullptr)
    {
        buffer->append(static_cast<const char *>(ptr), size * nmemb);
    }
    return size * nmemb;
}
} // namespace

NotificationBackend::NotificationBackend(const Mode mode) : mode_(mode) {}

//------------------------------------------------------------------------------
std::string NotificationBackend::name() const
{
    switch (mode_)
    {
    case Mode::Local:
        return "local";
    case Mode::Telegram:
        return "telegram";
    }
    return "local";
}

//------------------------------------------------------------------------------
std::string NotificationBackend::description() const
{
    switch (mode_)
    {
    case Mode::Local:
        return "Send notification using notify-send on Linux";
    case Mode::Telegram:
        return "Send notification via Telegram Bot API";
    }
    return "Send notification";
}

//------------------------------------------------------------------------------
void NotificationBackend::configure_cli(CLI::App &subcommand, Notification &notification) const
{
    subcommand.add_option("-t,--title", notification.payload.title, "Title")->required();
    subcommand.add_option("-b,--body", notification.payload.body, "Body")->required();
    subcommand.add_option("-p,--topic", notification.topic, "Topic");
    subcommand.add_option("-i,--image-url", notification.payload.image_url, "Image URL");

    if (mode_ == Mode::Telegram)
    {
        subcommand.add_option("-k,--bot-token", notification.bot_token, "Telegram bot token")->required();
        subcommand.add_option("-c,--chat-id", notification.chat_id, "Telegram chat ID")->required();
    }
}

//------------------------------------------------------------------------------
void NotificationBackend::send(const Notification &notification) const
{
    if (mode_ == Mode::Local)
    {
#if defined(__linux__)
        try
        {
            const std::vector<std::string> args = build_arguments(notification);
            execute_notify_send(args);
        }
        catch (const std::exception &ex)
        {
            std::cerr << "[notify-cli] Unexpected failure while sending local notification: "
                      << ex.what() << std::endl;
        }
#else
        (void)notification;
        std::cerr << "[notify-cli] notify-send backend is unavailable on this platform." << std::endl;
#endif
        return;
    }

    if (!send_via_telegram(notification))
    {
        std::cerr << "[notify-cli] Failed to deliver notification via Telegram." << std::endl;
    }
}

//------------------------------------------------------------------------------
bool NotificationBackend::send_via_telegram(const Notification &notification) const
{
    if (notification.bot_token.empty() || notification.chat_id.empty())
    {
        std::cerr << "[notify-cli] Missing Telegram credentials." << std::endl;
        return false;
    }

    CurlGlobalGuard curl_guard;
    if (!curl_guard.ok())
    {
        std::cerr << "[notify-cli] Failed to initialize libcurl: "
                  << curl_easy_strerror(curl_guard.status()) << std::endl;
        return false;
    }

    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), &curl_easy_cleanup);
    if (!curl)
    {
        std::cerr << "[notify-cli] Unable to create CURL easy handle." << std::endl;
        return false;
    }

    const std::string api_url = std::string(kTelegramBaseUrl) + notification.bot_token + "/sendMessage";
    const std::string message = build_message(notification);

    char *escaped_chat_id = curl_easy_escape(curl.get(), notification.chat_id.c_str(),
                                             static_cast<int>(notification.chat_id.size()));
    char *escaped_text = curl_easy_escape(curl.get(), message.c_str(), static_cast<int>(message.size()));

    if (escaped_chat_id == nullptr || escaped_text == nullptr)
    {
        if (escaped_chat_id != nullptr)
        {
            curl_free(escaped_chat_id);
        }
        if (escaped_text != nullptr)
        {
            curl_free(escaped_text);
        }
        std::cerr << "[notify-cli] Failed to encode Telegram payload." << std::endl;
        return false;
    }

    const std::string post_fields = std::string("chat_id=") + escaped_chat_id + "&text=" + escaped_text;
    curl_free(escaped_chat_id);
    curl_free(escaped_text);

    std::string response_body;

    curl_easy_setopt(curl.get(), CURLOPT_URL, api_url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_POST, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, post_fields.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDSIZE, static_cast<long>(post_fields.size()));
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "clinotify/1.0");
    curl_easy_setopt(curl.get(), CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, &collect_response);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response_body);

    const CURLcode result = curl_easy_perform(curl.get());
    if (result != CURLE_OK)
    {
        std::cerr << "[notify-cli] Failed to call Telegram API: " << curl_easy_strerror(result) << std::endl;
        return false;
    }

    long http_status = 0;
    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &http_status);
    if (http_status < 200 || http_status >= 300)
    {
        std::cerr << "[notify-cli] Telegram API returned status " << http_status;
        if (!response_body.empty())
        {
            std::cerr << ": " << response_body;
        }
        std::cerr << std::endl;
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
std::string NotificationBackend::build_message(const Notification &notification) const
{
    std::string message;

    if (!notification.payload.title.empty())
    {
        message = notification.payload.title;
    }

    if (!notification.payload.body.empty())
    {
        if (!message.empty())
        {
            message.append("\n");
        }
        message.append(notification.payload.body);
    }

    if (!notification.topic.empty())
    {
        if (!message.empty())
        {
            message.append("\n\n");
        }
        message.append("Topic: ");
        message.append(notification.topic);
    }

    if (!notification.payload.image_url.empty())
    {
        if (!message.empty())
        {
            message.append("\n\n");
        }
        message.append("Image: ");
        message.append(notification.payload.image_url);
    }

    if (message.empty())
    {
        message = "Notification";
    }

    return message;
}

//------------------------------------------------------------------------------
#if defined(__linux__)
std::vector<std::string> NotificationBackend::build_arguments(const Notification &notification) const
{
    std::vector<std::string> args;
    args.reserve(10);
    args.emplace_back(kNotifySendBinary);

    if (!notification.topic.empty())
    {
        args.emplace_back("--app-name");
        args.emplace_back(notification.topic);
        args.emplace_back("--category");
        args.emplace_back(notification.topic);
        args.emplace_back("--hint=string:topic:" + notification.topic);
    }

    if (!notification.id.empty())
    {
        args.emplace_back("--hint=string:notification-id:" + notification.id);
    }

    if (notification.created_at != 0)
    {
        args.emplace_back("--hint=int:created-at:" +
                          std::to_string(static_cast<long long>(notification.created_at)));
    }

    if (!notification.payload.image_url.empty())
    {
        args.emplace_back("--icon");
        args.emplace_back(notification.payload.image_url);
    }

    const std::string title = notification.payload.title.empty() ? "Notification" : notification.payload.title;
    args.emplace_back(title);

    if (!notification.payload.body.empty())
    {
        args.emplace_back(notification.payload.body);
    }

    return args;
}

//------------------------------------------------------------------------------
void NotificationBackend::execute_notify_send(const std::vector<std::string> &args) const
{
    std::vector<char *> argv;
    argv.reserve(args.size() + 1);

    for (const std::string &arg : args)
    {
        argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);

    const pid_t pid = ::fork();
    if (pid == -1)
    {
        std::cerr << "[notify-cli] Failed to fork for notify-send: " << std::strerror(errno) << std::endl;
        return;
    }

    if (pid == 0)
    {
        ::execvp(kNotifySendBinary, argv.data());
        std::cerr << "[notify-cli] execvp failed for notify-send: " << std::strerror(errno) << std::endl;
        _exit(EXIT_FAILURE);
    }

    int status = 0;
    if (::waitpid(pid, &status, 0) == -1)
    {
        std::cerr << "[notify-cli] Failed to wait for notify-send: " << std::strerror(errno) << std::endl;
        return;
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
    {
        std::cerr << "[notify-cli] notify-send exited abnormally";
        if (WIFEXITED(status))
        {
            std::cerr << " with status " << WEXITSTATUS(status);
        }
        else if (WIFSIGNALED(status))
        {
            std::cerr << " due to signal " << WTERMSIG(status);
        }
        std::cerr << std::endl;
    }
}
#endif
