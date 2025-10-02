#include "configure.h"
#include "notification_backend.h"

#include <CLI/CLI.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <iostream>
#include <string>

namespace
{
// Sinh chuỗi mô tả phiên bản của ứng dụng.
[[nodiscard]] std::string build_version_string()
{
    return std::string(PROJECT_EXECUTABLE) + " version " + PROJECT_VERSION;
}
} // namespace

//------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    CLI::App app{"Notification in CLI"};
    app.set_version_flag("--version", build_version_string());
    app.require_subcommand(1);

    NotificationBackend local_backend(NotificationBackend::Mode::Local);
    Notification local_notification;

    NotificationBackend telegram_backend(NotificationBackend::Mode::Telegram);
    Notification telegram_notification;

    auto register_subcommand = [&app](NotificationBackend &backend, Notification &notification) {
        CLI::App *subcommand = app.add_subcommand(backend.name(), backend.description());
        backend.configure_cli(*subcommand, notification);

        subcommand->callback([&backend, &notification]() {
            const boost::uuids::uuid id = boost::uuids::random_generator()();
            notification.id = boost::uuids::to_string(id);

            const auto now = std::chrono::system_clock::now();
            notification.created_at = std::chrono::system_clock::to_time_t(now);

            print_notification_details(notification);
            backend.send(notification);
        });
    };

    register_subcommand(local_backend, local_notification);
    register_subcommand(telegram_backend, telegram_notification);

    CLI11_PARSE(app, argc, argv);
    return 0;
}

//------------------------------------------------------------------------------
void print_notification_details(const Notification &options)
{
    constexpr int kLineLength = 45;
    const std::string separator(kLineLength, '=');

    char time_buffer[100] = {0};
    const std::tm *tm_info = std::localtime(&options.created_at);
    if (tm_info != nullptr)
    {
        std::strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    }

    std::cout << '\n' << separator << '\n';
    std::cout << "         NOTIFICATION DETAILS\n";
    std::cout << separator << '\n';
    std::cout << std::left;
    std::cout << " Notification ID  : " << options.id << '\n';
    std::cout << " Topic            : " << options.topic << '\n';
    std::cout << " Bot Token        : "
              << (options.bot_token.empty() ? "<not set>" : "<provided>") << '\n';
    std::cout << " Chat ID          : " << options.chat_id << '\n';
    std::cout << " Created At       : " << time_buffer << '\n';
    std::cout << " Title            : " << options.payload.title << '\n';
    std::cout << " Body             : " << options.payload.body << '\n';
    std::cout << " Image URL        : " << options.payload.image_url << '\n';
    std::cout << separator << '\n' << std::endl;
}
