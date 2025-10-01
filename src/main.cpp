#include "main.h"


int main(int argc, char **argv)
{
    CLI::App app{"Notification in CLI"};
    app.require_subcommand(1); // bắt buộc chọn 1 platform

    // ----- Local -----
    Notification local;
    auto sc_local = app.add_subcommand("local", "Send notification via local OS");
    sc_local->add_option("-t,--title", local.payload.title, "Title")->required();
    sc_local->add_option("-b,--body", local.payload.body, "Body")->required();
    sc_local->add_option("-p,--topic", local.topic, "Topic");
    sc_local->add_option("-i,--image-url", local.payload.image_url, "Image URL");
    sc_local->callback([&]()
                       {
        // Sinh ID
        boost::uuids::uuid id = boost::uuids::random_generator()();
        local.id = boost::uuids::to_string(id);
        // Lấy thời gian tạo
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        local.created_at = t;

        print_notification_details(local);
        send_local_linux_notification(local); 
    });

    CLI11_PARSE(app, argc, argv);
    return 0;
}

void print_notification_details(const Notification &options)
{
    constexpr int line_length = 45;
    const std::string separator(line_length, '=');

    char time_buffer[100];
    std::tm *tm_info = std::localtime(&options.created_at);
    std::strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);

    std::cout << "\n"
              << separator << "\n";
    std::cout << "         NOTIFICATION DETAILS\n";
    std::cout << separator << "\n";
    std::cout << std::left;
    std::cout << " Notification ID  : " << options.id << "\n";
    std::cout << " Topic            : " << options.topic << "\n";
    std::cout << " Created At       : " << time_buffer << "\n";
    std::cout << " Title            : " << options.payload.title << "\n";
    std::cout << " Body             : " << options.payload.body << "\n";
    std::cout << " Image URL        : " << options.payload.image_url << "\n";
    std::cout << separator << "\n"
              << std::endl;
}

void send_local_linux_notification(Notification notification)
{
#if defined(__linux__)
    const char *notify_send_binary = "notify-send";

    // Prepare arguments for notify-send while keeping ownership with std::string.
    std::vector<std::string> args;
    args.emplace_back(notify_send_binary);

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

    // Convert to the char* array expected by execvp.
    std::vector<char *> argv;
    argv.reserve(args.size() + 1);
    for (auto &arg : args)
    {
        argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);

    pid_t pid = ::fork();
    if (pid == -1)
    {
        std::cerr << "[notify-cli] Failed to fork for notify-send: " << std::strerror(errno) << std::endl;
        return;
    }

    if (pid == 0)
    {
        ::execvp(notify_send_binary, argv.data());
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
#else
    (void)notification;
    std::cerr << "[notify-cli] Local Linux notification backend is unavailable on this platform." << std::endl;
#endif
}
