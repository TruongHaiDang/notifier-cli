#include "configure.h"
#include "notify_send_backend.h"
#include "notification_backend.h"

#include <CLI/CLI.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

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

    struct BackendContext
    {
        std::shared_ptr<NotificationBackend> backend;
        Notification notification;
    };

    std::vector<std::shared_ptr<BackendContext>> contexts;
    contexts.reserve(1);

    // Hàm tiện ích để đăng ký backend vào CLI. Điều này giúp việc bổ sung
    // backend mới trong tương lai chỉ cần gọi hàm này với hiện thực tương ứng.
    auto register_backend = [&](std::shared_ptr<NotificationBackend> backend) {
        auto context = std::make_shared<BackendContext>();
        context->backend = std::move(backend);

        CLI::App *subcommand = app.add_subcommand(context->backend->name(), context->backend->description());
        context->backend->configure_cli(*subcommand, context->notification);

        subcommand->callback([ctx = context->backend, notification_context = context]() {
            const boost::uuids::uuid id = boost::uuids::random_generator()();
            notification_context->notification.id = boost::uuids::to_string(id);

            const auto now = std::chrono::system_clock::now();
            notification_context->notification.created_at = std::chrono::system_clock::to_time_t(now);

            print_notification_details(notification_context->notification);
            ctx->send(notification_context->notification);
        });

        contexts.emplace_back(std::move(context));
    };

    register_backend(std::make_shared<NotifySendBackend>());

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
    std::cout << " Created At       : " << time_buffer << '\n';
    std::cout << " Title            : " << options.payload.title << '\n';
    std::cout << " Body             : " << options.payload.body << '\n';
    std::cout << " Image URL        : " << options.payload.image_url << '\n';
    std::cout << separator << '\n' << std::endl;
}
