#include "DatabaseManager.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <tabulate/table.hpp>
#include <vector>

DatabaseManager::DatabaseManager(const std::string &filepath)
    : db_(filepath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE),
      filepath_(filepath) {}

bool DatabaseManager::createTableTimestamps() {
  const char *createTableSQL = "CREATE TABLE IF NOT EXISTS timestamps ("
                               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                               "timezone TEXT NOT NULL,"
                               "timestamp TEXT NOT NULL,"
                               "type TEXT NOT NULL,"
                               "session_id integer NOT NULL,"
                               "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
                               ");";

  try {
    db_.exec(createTableSQL);
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Table creation error: " << e.what() << std::endl;
    return false;
  }
}

bool DatabaseManager::createTableDailyHours() {
  const char *createTableSQL = "CREATE TABLE IF NOT EXISTS dailyhours ("
                               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                               "date TEXT NOT NULL,"
                               "hours REAL NOT NULL,"
                               "session_id integer NOT NULL,"
                               "UNIQUE(date, hours)"
                               ");";

  try {
    db_.exec(createTableSQL);
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Table creation error: " << e.what() << std::endl;
    return false;
  }
}

void DatabaseManager::printTimestampsTable() {
  using namespace tabulate;

  const char *sql = "SELECT ts.timestamp, ts.type, d.hours "
                    "FROM timestamps ts "
                    "LEFT JOIN dailyhours d ON ts.session_id = d.session_id "
                    "ORDER BY timestamp ASC;";
  const char *sqlSum = "SELECT SUM(hours) from dailyhours";

  try {
    SQLite::Statement query(db_, sql);
    SQLite::Statement querySum(db_, sqlSum);
    querySum.executeStep();

    // Create table with headers
    Table table;
    table.add_row({"Timestamp", "Type", "Hours"});

    // Format header row
    table[0]
        .format()
        .font_style({FontStyle::bold})
        .font_color(Color::yellow)
        .font_align(FontAlign::center);

    // Add data rows
    int row_count = 1;
    while (query.executeStep()) {
      table.add_row({
          query.getColumn(0).getText(), // timestamp
          query.getColumn(1).getText(), // type
          row_count % 2 == 0 ? query.getColumn(2).getText()
                             : "", // totol duration
      });

      if (row_count % 2 == 0) {
        table[row_count].format().font_color(Color::white).hide_border_top();
      }
      row_count++;
    }

    // Print the table
    std::cout << table << std::endl;

    // Print summary
    std::cout << "Total records: " << (row_count - 1) << std::endl;
    std::cout << "Total hours: " << querySum.getColumn(0).getDouble()
              << std::endl;

  } catch (const std::exception &e) {
    std::cerr << "Error printing table: " << e.what() << std::endl;
  }
}

bool DatabaseManager::insertTimestamp(std::string timezone,
                                      std::string formatedTimestamp,
                                      std::string type) {
  const char *sql =
      "INSERT INTO timestamps (timezone, timestamp, type, session_id) "
      "VALUES (?, ?, ?, ?);";

  try {
    int session_id{-1};

    SQLite::Statement query(db_, sql);
    query.bind(1, timezone);
    query.bind(2, formatedTimestamp);
    query.bind(3, type);

    if (type == "Kommen") {
      const char *getMaxSql =
          "SELECT COALESCE(MAX(session_id), 0) + 1 FROM timestamps;";
      SQLite::Statement maxQuery(db_, getMaxSql);
      if (maxQuery.executeStep()) {
        session_id = maxQuery.getColumn(0).getInt();
      }
    } else if (type == "Gehen") {
      const char *getCurrentSql = "SELECT MAX(session_id) FROM timestamps;";
      SQLite::Statement currentQuery(db_, getCurrentSql);
      if (currentQuery.executeStep() && !currentQuery.getColumn(0).isNull()) {
        session_id = currentQuery.getColumn(0).getInt();
      } else {
        std::cerr << "Error: No active session to check out from!" << std::endl;
        return false;
      }
    } else {
      std::cerr << "Error: Invalid type '" << type << "'" << std::endl;
      return false;
    }

    query.bind(4, session_id);
    query.exec();

    std::cout << type << " timestamp saved: " << formatedTimestamp << std::endl;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Insert error: " << e.what() << std::endl;
    return false;
  }
}

std::string DatabaseManager::getLastError() const { return db_.getErrorMsg(); }

std::optional<std::chrono::system_clock::time_point>
DatabaseManager::parseTimestamp(const std::string &timestamp_str) const {
  std::istringstream ss(timestamp_str);
  std::chrono::system_clock::time_point timepoint;
  std::chrono::from_stream(ss, "%Y-%m-%d %H:%M:%S", timepoint);

  if (ss.fail()) {
    return std::nullopt;
  }

  return timepoint;
}

WorkingHours DatabaseManager::calculateDailyHours(const std::string &date) {
  WorkingHours result{date, {}, 0, 0, 0, 0};
  result.workBreak = 0.5;
  result.minHoursForWorkBreak = 6;

  const char *sql = "SELECT timestamp, type FROM timestamps "
                    "WHERE DATE(timestamp) = ? "
                    "ORDER BY timestamp ASC;";

  try {
    SQLite::Statement query(db_, sql);
    query.bind(1, date);

    std::vector<std::chrono::system_clock::time_point> kommen_times;
    std::vector<std::chrono::system_clock::time_point> gehen_times;

    while (query.executeStep()) {
      std::string timestamp_str = query.getColumn(0).getText();
      std::string type_str = query.getColumn(1).getText();

      auto timepoint_opt = parseTimestamp(timestamp_str);

      if (!timepoint_opt) {
        std::cerr << "Error: Invalid timestamp in database: " << timestamp_str
                  << std::endl;
        result.hours.push_back(-1.0);
        return result;
      }

      auto timepoint = *timepoint_opt;

      if (type_str == "Kommen") {
        kommen_times.push_back(timepoint);
      } else {
        gehen_times.push_back(timepoint);
      }
    }

    result.kommen_count = kommen_times.size();
    result.gehen_count = gehen_times.size();

    // Calculate hours by pairing Kommen with Gehen
    size_t number_of_pairs = std::min(kommen_times.size(), gehen_times.size());
    for (size_t i = 0; i < number_of_pairs; ++i) {
      auto duration = gehen_times[i] - kommen_times[i];
      result.hours.push_back(
          std::chrono::duration_cast<std::chrono::minutes>(duration).count() /
          60.0);
    }

    return result;
  } catch (const std::exception &e) {
    std::cerr << "Query error: " << e.what() << std::endl;
    return result;
  }
}

bool DatabaseManager::populateDailyHours() {
  const char *sqlDailyHours = "INSERT OR IGNORE INTO dailyhours (date, hours, "
                              "session_id) VALUES (?, ?, ?); ";
  const char *sqlDates = "SELECT DISTINCT DATE(timestamp) AS date, session_id "
                         "FROM timestamps "
                         "ORDER BY date;";
  struct Row {
    int session_id;
    std::string day;
  };

  try {
    std::vector<Row> dates;

    // Get all distinct dates
    {
      SQLite::Statement queryDates(db_, sqlDates);
      while (queryDates.executeStep()) {
        Row row;
        row.day = queryDates.getColumn(0).getText();
        row.session_id = queryDates.getColumn(1).getInt();
        dates.push_back(row);
      }
    }

    // Insert/update daily hours for each date
    for (const auto &row : dates) {
      std::vector<double> workHours = calculateDailyHours(row.day).hours;
      for (const auto &hour : workHours) {
        SQLite::Statement queryInsert(db_, sqlDailyHours);
        queryInsert.bind(1, row.day);
        queryInsert.bind(2, hour);
        queryInsert.bind(3, row.session_id);
        queryInsert.exec();
      }
    }

    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error populating daily hours: " << e.what() << std::endl;
    return false;
  }
}
