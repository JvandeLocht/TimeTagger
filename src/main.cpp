#include "DatabaseManager.h"
#include "TimeManager.h"
#include <CLI/CLI.hpp>
#include <iostream>

int main(int argc, char **argv) {
  CLI::App app{"TimeTagger - Track your working hours"};

  const std::string TIMEZONE = "Europe/Berlin";
  bool create_timestamp = false;
  bool force_checkin = false;
  bool force_checkout = false;

  DatabaseManager dbManager("timestamps.db");
  TimeManager ts(TIMEZONE);

  if (dbManager.connect() != true) {
    std::cout << "Can't open database: " << dbManager.getLastError()
              << std::endl;
    return 1;
  }

  if (dbManager.createTableTimestamps() != true) {
    std::cout << "Can't create Table: " << dbManager.getLastError()
              << std::endl;
    return 1;
  }

  if (dbManager.createTableDailyHours() != true) {
    std::cout << "Can't create Table: " << dbManager.getLastError()
              << std::endl;
    return 1;
  }

  app.add_flag("-t,--timestamp,--create-timestamp", create_timestamp,
               "Create a new timestamp (auto-detects check-in/check-out "
               "based on time)");

  app.add_flag("-k,--checkin", force_checkin, "Forces the type Kommen");
  app.add_flag("-g,--checkout", force_checkout, "Forces the type Gehen");

  CLI11_PARSE(app, argc, argv);

  if (create_timestamp) {
    if (force_checkin) {
      ts.type = TimestampType::KOMMEN;
    } else if (force_checkout) {

      ts.type = TimestampType::GEHEN;
    }
    if (!ts.createTimestamp()) {
      std::cerr << "Error: Couldn't create timestamp!" << std::endl;
      return 1;
    }

    // Display the timestamp
    ts.print();

    // Save to database
    if (!dbManager.insertTimestamp(ts.getTimezone(), ts.getFormattedTime(),
                                   ts.getTypeString())) {
      std::cerr << "Error: Failed to save timestamp to database" << std::endl;
      return 1;
    }

    // Update daily hours
    if (!dbManager.populateDailyHours()) {
      std::cerr << "Warning: Failed to update daily hours" << std::endl;
    }
  }

  if (argc == 1) {
    std::cout << app.help() << std::endl;
    return 0;
  }

  return 0;
}
