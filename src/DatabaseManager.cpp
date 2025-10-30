#include "DatabaseManager.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

DatabaseManager::DatabaseManager(const string &filepath)
    : db_(nullptr), filepath_(filepath) {}

DatabaseManager::~DatabaseManager() {
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool DatabaseManager::connect() {
    if (sqlite3_open(filepath_.c_str(), &db_) != SQLITE_OK) {
        return false;
    }
    return true;
};

bool DatabaseManager::createTableTimestamps() {
    const char *createTableSQL = "CREATE TABLE IF NOT EXISTS timestamps ("
                                 "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                 "timezone TEXT NOT NULL,"
                                 "timestamp TEXT NOT NULL,"
                                 "type TEXT NOT NULL,"
                                 "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
                                 ");";

    if (sqlite3_exec(db_, createTableSQL, nullptr, nullptr, nullptr) !=
        SQLITE_OK) {
        return false;
    } else {
        return true;
    }
};

bool DatabaseManager::createTableDailyHours() {
    const char *createTableSQL = "CREATE TABLE IF NOT EXISTS dailyhours ("
                                 "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                 "date TEXT NOT NULL UNIQUE,"
                                 "hours TEXT NOT NULL"
                                 ");";

    if (sqlite3_exec(db_, createTableSQL, nullptr, nullptr, nullptr) !=
        SQLITE_OK) {
        return false;
    } else {
        return true;
    }
};

void DatabaseManager::printTableShell() {
    string command = "sqlite3 -cmd '.mode box' -cmd '.headers on' " +
                     filepath_ + " 'SELECT * FROM timestamps;'";

    system(command.c_str());
};

bool DatabaseManager::insertTimestamp(string timezone, string formatedTimestamp,
                                      string type) {
    const char *sql = "INSERT INTO timestamps (timezone, timestamp, type) "
                      "VALUES (?, ?, ?);";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, timezone.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, formatedTimestamp.c_str(), -1,
                          SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, type.c_str(), -1, SQLITE_TRANSIENT);

        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        if (success) {
            cout << type << " timestamp saved: " << formatedTimestamp << endl;
        }
        return success;
    } else {
        return false;
    }
}

string DatabaseManager::getLastError() const {
    if (db_ != nullptr) {
        return sqlite3_errmsg(db_);
    } else {
        return "Database not connected!";
    }
}

chrono::system_clock::time_point
DatabaseManager::parseTimestamp(const string &timestamp_str) const {
    istringstream ss(timestamp_str);
    chrono::system_clock::time_point timepoint;
    from_stream(ss, "%Y-%m-%d %H:%M:%S", timepoint);
    return timepoint;
}

WorkingHours DatabaseManager::calculateDailyHours(const string &date) {
    WorkingHours result{date, 0.0, 0, 0, 0, 0};
    result.workBreak = 0.5;
    result.minHoursForWorkBreak = 6;

    const char *sql = "SELECT timestamp, type FROM timestamps "
                      "WHERE DATE(timestamp) = ? "
                      "ORDER BY timestamp ASC;";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return result;
    }

    sqlite3_bind_text(stmt, 1, date.c_str(), -1, SQLITE_TRANSIENT);

    vector<chrono::system_clock::time_point> kommen_times;
    vector<chrono::system_clock::time_point> gehen_times;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *timestamp_str =
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        const char *type_str =
            reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));

        auto timepoint = parseTimestamp(timestamp_str);

        if (string(type_str) == "Kommen") {
            kommen_times.push_back(timepoint);
        } else {
            gehen_times.push_back(timepoint);
        }
    }

    sqlite3_finalize(stmt);

    result.kommen_count = kommen_times.size();
    result.gehen_count = gehen_times.size();

    // Calculate hours by pairing Kommen with Gehen
    size_t number_of_pairs = min(kommen_times.size(), gehen_times.size());
    for (size_t i = 0; i < number_of_pairs; ++i) {
        auto duration = gehen_times[i] - kommen_times[i];
        result.hours += duration_cast<chrono::minutes>(duration).count() / 60.0;
    }
    if (result.hours >= result.minHoursForWorkBreak) {
        result.hours = result.hours - result.workBreak;
    }

    return result;
}

bool DatabaseManager::populateDailyHours() {
    const char *sqlDailyHours =
        "INSERT INTO dailyhours (date, hours) VALUES (?, ?) "
        "ON CONFLICT(date) DO UPDATE SET hours = excluded.hours;";
    const char *sqlDates = "SELECT DISTINCT DATE(timestamp) AS date "
                           "FROM timestamps "
                           "ORDER BY date;";

    sqlite3_stmt *stmtDailyHours;
    sqlite3_stmt *stmtDates;

    vector<string> dates;

    if (sqlite3_prepare_v2(db_, sqlDates, -1, &stmtDates, nullptr) !=
        SQLITE_OK) {
        return false;
    }

    while (sqlite3_step(stmtDates) == SQLITE_ROW) {
        const char *date =
            reinterpret_cast<const char *>(sqlite3_column_text(stmtDates, 0));

        dates.push_back(date);
    }
    sqlite3_finalize(stmtDates);

    for (const auto &day : dates) {
        string workHour = format("{}", calculateDailyHours(day).hours);
        if (sqlite3_prepare_v2(db_, sqlDailyHours, -1, &stmtDailyHours,
                               nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmtDailyHours, 1, day.c_str(), -1,
                              SQLITE_TRANSIENT);
            sqlite3_bind_text(stmtDailyHours, 2, workHour.c_str(), -1,
                              SQLITE_TRANSIENT);

            sqlite3_step(stmtDailyHours);
            sqlite3_finalize(stmtDailyHours);
        } else {
            return false;
        }
    }
    return true;
}
