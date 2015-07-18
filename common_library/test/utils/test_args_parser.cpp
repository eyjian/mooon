#include <mooon/utils/args_parser.h>

STRING_ARG_DEFINE(ip1, "127.0.0.1", "ip1");
INTEGER_ARG_DEFINE(uint16_t, port1, 8888, 1000, 5000, "port1");

int main(int argc, char* argv[])
{
    std::string errmsg;
    mooon::utils::g_help_string = "help";
    mooon::utils::g_version_string = "version";

    if (!mooon::utils::parse_arguments(argc, argv, &errmsg))
    {
        fprintf(stderr, "%s\n", errmsg.c_str());
        exit(1);
    }

    printf("ip1: %s\n", mooon::argument::ip1->c_value());
    printf("port1: %d\n", mooon::argument::port1->value());
    return 0;
}
