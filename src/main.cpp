#include "main.h"

int main(int argc, char** argv)
{
    CLI::App app{"Notification in CLI"};

    CLI11_PARSE(app, argc, argv);
    return 0;
}
