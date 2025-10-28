#include "Command.h"
#include "DatabaseManager.h"
#include "Timestamp.h"
#include <iostream>
#include <sqlite3.h>
#include <sstream>
#include <string>

using namespace std;

int main() {
  const string TIMEZONE = "Europe/Berlin";
  string input;

  DatabaseManager dbManager("timestamps.db");
  Timestamp ts(TIMEZONE);

  if (dbManager.connect() != true) {
    cout << "Can't open database: " << dbManager.getLastError() << endl;
    return 1;
  }

  if (dbManager.createTable() != true) {
    cout << "Can't create Table: " << dbManager.getLastError() << endl;
    return 1;
  }

  while (true) {
    cout << "> ";
    getline(cin, input);

    istringstream iss(input);
    string cmdStr;
    iss >> cmdStr;

    switch (parseCommand(cmdStr)) {
    case Command::HELP:
      cout << "Available commands: help, timestamp, quit\n";
      break;

    case Command::QUIT:
      return 0;

    case Command::TIMESTAMP:
      if (ts.createTimestamp() != true) {
        cout << "Couldnt create Timestamp!" << endl;
      }
      ts.print();
      break;

    case Command::PRINT:
      ts.print();
      break;

    case Command::UNKNOWN:
      cout << "Unknown command. Type 'help' for options.\n";
      break;
    }
  }

  return 0;
}
