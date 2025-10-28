#include "Command.h"
#include <string>
#include <unordered_map>

Command parseCommand(const std::string &cmd) {
    static std::unordered_map<std::string, Command> cmdMap = {
        {"help", Command::HELP},
        {"h", Command::HELP},
        {"quit", Command::QUIT},
        {"q", Command::QUIT},
        {"p", Command::PRINT},
        {"print", Command::PRINT},
        {"timestamp", Command::TIMESTAMP},
        {"t", Command::TIMESTAMP},
        {"d", Command::WRITE_DB},
        {"writeDB", Command::WRITE_DB},
        {"pd", Command::PRINT_DB_SHELL},
    };

    auto it = cmdMap.find(cmd);
    return (it != cmdMap.end()) ? it->second : Command::UNKNOWN;
}
