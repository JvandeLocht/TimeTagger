#include <string>

enum class Command {
    HELP,
    QUIT,
    TIMESTAMP,
    PRINT,
    WRITE_DB,
    PRINT_DB_SHELL,
    UNKNOWN
};

Command parseCommand(const std::string &cmd);
