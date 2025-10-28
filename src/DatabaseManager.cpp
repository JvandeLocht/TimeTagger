#include "DatabaseManager.h"
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

bool DatabaseManager::createTable() {
    const char *createTableSQL = "CREATE TABLE IF NOT EXISTS timestamps ("
                                 "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                 "timezone TEXT NOT NULL,"
                                 "timestamp TEXT NOT NULL,"
                                 "type TEXT NOT NULL,"
                                 "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
                                 ");";

    if (sqlite3_exec(db_, createTableSQL, nullptr, nullptr, nullptr) ==
        SQLITE_ABORT) {
        return false;
    } else {
        return true;
    }
};

bool DatabaseManager::insertTimestamp(string timezone, string formatedTimestamp,
                                      string type) {
    const char *sql = "INSERT INTO timestamps (timezone, timestamp, type) "
                      "VALUES (?, ?, ?);";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, timezone.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, formatedTimestamp.c_str(), -1,
                          SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, type.c_str(), -1, SQLITE_STATIC);

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
