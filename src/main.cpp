#include "Command.h"
#include "DatabaseManager.h"
#include "TimeManager.h"
#include <iostream>
#include <sstream>

using namespace std;

int main() {
    const string TIMEZONE = "Europe/Berlin";
    string input;

    DatabaseManager dbManager("timestamps.db");
    TimeManager ts(TIMEZONE);

    if (dbManager.connect() != true) {
        cout << "Can't open database: " << dbManager.getLastError() << endl;
        return 1;
    }

    if (dbManager.createTableTimestamps() != true) {
        cout << "Can't create Table: " << dbManager.getLastError() << endl;
        return 1;
    }

    if (dbManager.createTableDailyHours() != true) {
        cout << "Can't create Table: " << dbManager.getLastError() << endl;
        return 1;
    }

    // cout << dbManager.calculateDailyHours("2025-10-28").hours << endl;
    dbManager.populateDailyHours();
    // while (true) {
    //   cout << "> ";
    //   getline(cin, input);

    //   istringstream iss(input);
    //   string cmdStr;
    //   iss >> cmdStr;

    //   switch (parseCommand(cmdStr)) {
    //   case Command::HELP:
    //     cout << "Available commands: help(h), timestamp(t), quit(q), "
    //             "writeDB(d), printDB(pd)\n";
    //     break;

    //   case Command::QUIT:
    //     return 0;

    //   case Command::TIMESTAMP:
    //     if (ts.createTimestamp() != true) {
    //       cout << "Couldnt create Timestamp!" << endl;
    //     }
    //     ts.print();
    //     break;

    //   case Command::PRINT:
    //     ts.print();
    //     break;

    //   case Command::PRINT_DB_SHELL:
    //     dbManager.printTableShell();
    //     break;

    //   case Command::WRITE_DB:
    //     dbManager.insertTimestamp(ts.getTimezone(), ts.getFormattedTime(),
    //                               ts.getTypeString());
    //     break;

    //   case Command::UNKNOWN:
    //     cout << "Unknown command. Type 'help' for options.\n";
    //     break;
    //   }
    // }

    return 0;
}
