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
                                 "date TEXT NOT NULL UNIQUE,"
                                 "hours REAL NOT NULL"
                                 ");";

    try {
        db_.exec(createTableSQL);
        return true;
    } catch (const std::exception &e) {
        std::cerr << "Table creation error: " << e.what() << std::endl;
        return false;
    }
}

void DatabaseManager::printTableShell() {
    std::string command = "sqlite3 -cmd '.mode box' -cmd '.headers on' " +
                          filepath_ + " 'SELECT * FROM timestamps;'";

    system(command.c_str());
}

void DatabaseManager::printTimestampsTable() {
    using namespace tabulate;

    const char *sql = "SELECT ts.timestamp, ts.type, d.hours "
                      "FROM timestamps ts "
                      "LEFT JOIN dailyhours d ON DATE(ts.timestamp) = d.date "
                      "ORDER BY timestamp DESC;";

    try {
        SQLite::Statement query(db_, sql);

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
                query.getColumn(2).getText(), // totol duration
            });

            // Alternate row colors for better readability
            if (row_count % 2 == 0) {
                table[row_count].format().font_color(Color::white);
            } else {
                table[row_count].format().font_color(Color::cyan);
            }
            row_count++;
        }

        // Print the table
        std::cout << table << std::endl;

        // Print summary
        std::cout << "Total records: " << (row_count - 1) << std::endl;

    } catch (const std::exception &e) {
        std::cerr << "Error printing table: " << e.what() << std::endl;
    }
}

bool DatabaseManager::insertTimestamp(std::string timezone,
                                      std::string formatedTimestamp,
                                      std::string type) {
    const char *sql = "INSERT INTO timestamps (timezone, timestamp, type) "
                      "VALUES (?, ?, ?);";

    try {
        SQLite::Statement query(db_, sql);
        query.bind(1, timezone);
        query.bind(2, formatedTimestamp);
        query.bind(3, type);

        query.exec();

        std::cout << type << " timestamp saved: " << formatedTimestamp
                  << std::endl;
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
    WorkingHours result{date, 0.0, 0, 0, 0, 0};
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
                std::cerr << "Error: Invalid timestamp in database: "
                          << timestamp_str << std::endl;
                result.hours = -1.0;
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
        size_t number_of_pairs =
            std::min(kommen_times.size(), gehen_times.size());
        for (size_t i = 0; i < number_of_pairs; ++i) {
            auto duration = gehen_times[i] - kommen_times[i];
            result.hours +=
                std::chrono::duration_cast<std::chrono::minutes>(duration)
                    .count() /
                60.0;
        }
        if (result.hours >= result.minHoursForWorkBreak) {
            result.hours = result.hours - result.workBreak;
        }

        return result;
    } catch (const std::exception &e) {
        std::cerr << "Query error: " << e.what() << std::endl;
        return result;
    }
}

bool DatabaseManager::populateDailyHours() {
    const char *sqlDailyHours =
        "INSERT INTO dailyhours (date, hours) VALUES (?, ?) "
        "ON CONFLICT(date) DO UPDATE SET hours = excluded.hours;";
    const char *sqlDates = "SELECT DISTINCT DATE(timestamp) AS date "
                           "FROM timestamps "
                           "ORDER BY date;";

    try {
        std::vector<std::string> dates;

        // Get all distinct dates
        {
            SQLite::Statement queryDates(db_, sqlDates);
            while (queryDates.executeStep()) {
                dates.push_back(queryDates.getColumn(0).getText());
            }
        }

        // Insert/update daily hours for each date
        for (const auto &day : dates) {
            double workHour = calculateDailyHours(day).hours;

            SQLite::Statement queryInsert(db_, sqlDailyHours);
            queryInsert.bind(1, day);
            queryInsert.bind(2, workHour);
            queryInsert.exec();
        }

        return true;
    } catch (const std::exception &e) {
        std::cerr << "Error populating daily hours: " << e.what() << std::endl;
        return false;
    }
}
