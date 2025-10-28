#include <string>

enum class Command { HELP, QUIT, TIMESTAMP, PRINT, UNKNOWN };

Command parseCommand(const std::string &cmd);
