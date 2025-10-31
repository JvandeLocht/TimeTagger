#include "DatabaseManager.h"
#include "TimeManager.h"
#include <CLI/CLI.hpp>
#include <iostream>

using namespace std;

int main(int argc, char **argv) {
  CLI::App app{"TimeTagger - Track your working hours"};

  const string TIMEZONE = "Europe/Berlin";
  bool create_timestamp = false;

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

  if (!dbManager.populateDailyHours()) {
    cout << "Warning: Failed to populate daily hours table" << endl;
  }

  app.add_flag("-t,--timestamp,--create-timestamp", create_timestamp,
               "Create a new timestamp (auto-detects check-in/check-out "
               "based on time)");

  CLI11_PARSE(app, argc, argv);

  if (create_timestamp) {
    if (!ts.createTimestamp()) {
      cerr << "Error: Couldn't create timestamp!" << endl;
      return 1;
    }

    // Display the timestamp
    ts.print();

    // Save to database
    if (!dbManager.insertTimestamp(ts.getTimezone(), ts.getFormattedTime(),
                                   ts.getTypeString())) {
      cerr << "Error: Failed to save timestamp to database" << endl;
      return 1;
    }

    // Update daily hours
    if (!dbManager.populateDailyHours()) {
      cerr << "Warning: Failed to update daily hours" << endl;
    }
  }

  if (argc == 1) {
    cout << app.help() << endl;
    return 0;
  }

  return 0;
}
