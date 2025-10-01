#include "notification_backend.h"

#include <CLI/CLI.hpp>
#include <cerrno>
#include <cstring>
#include <exception>
#include <iostream>

#if defined(__linux__)
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

//------------------------------------------------------------------------------
std::string NotificationBackend::name() const
{
    return "local";
}

//------------------------------------------------------------------------------
std::string NotificationBackend::description() const
{
    return "Send notification using notify-send on Linux";
}

//------------------------------------------------------------------------------
void NotificationBackend::configure_cli(CLI::App &subcommand, Notification &notification) const
{
    subcommand.add_option("-t,--title", notification.payload.title, "Title")->required();
    subcommand.add_option("-b,--body", notification.payload.body, "Body")->required();
    subcommand.add_option("-p,--topic", notification.topic, "Topic");
    subcommand.add_option("-i,--image-url", notification.payload.image_url, "Image URL");
}

//------------------------------------------------------------------------------
void NotificationBackend::send(const Notification &notification) const
{
#if defined(__linux__)
    try
    {
        const std::vector<std::string> args = build_arguments(notification);
        execute_notify_send(args);
    }
    catch (const std::exception &ex)
    {
        std::cerr << "[notify-cli] Unexpected failure while sending notification: " << ex.what() << std::endl;
    }
#else
    (void)notification;
    std::cerr << "[notify-cli] notify-send backend is unavailable on this platform." << std::endl;
#endif
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
        args.emplace_back("--hint=int:created-at:" + std::to_string(static_cast<long long>(notification.created_at)));
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

void set_telegram_bot_token(std::string bot_token)
{
    
}

#endif
