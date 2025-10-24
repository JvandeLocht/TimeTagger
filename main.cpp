#include <chrono>
#include <format>
#include <iostream>
#include <sqlite3.h>
#include <string>
using namespace std;
using namespace chrono;

enum class TimestampType {
  KOMMEN, // Arriving (before 9 AM)
  GEHEN   // Leaving (after 9 AM)
};

class Timestamp {
private:
  string timezone_;
  zoned_time<system_clock::duration> zoned_time_;
  TimestampType type_;
  string formatted_time_;

public:
  // Constructor
  Timestamp(const string &timezone);

  // Getters
  string getTimezone() const;
  string getFormattedTime() const;
  TimestampType getType() const;

  string getTypeString() const;

  // Display method
  void print() const;
};

class DatabaseManager {
private:
  sqlite3 *db_;
  string filepath_;

public:
  DatabaseManager(const string &filepath);
  ~DatabaseManager();

  // Prevent copying (database connections shouldn't be copied)
  DatabaseManager(const DatabaseManager &) = delete;
  DatabaseManager &operator=(const DatabaseManager &) = delete;

  bool connect();
  bool createTable();
  bool insertTimestamp(string timezone, string formatedTimestamp, string type);
  string getLastError() const;
};

//================================================================================
//================================================================================

int main() {
  const string TIMEZONE = "Europe/Berlin";

  DatabaseManager dbManager("timestamps.db");

  if (dbManager.connect() != true) {
    cout << "Can't open database: " << dbManager.getLastError() << endl;
    return 1;
  }

  if (dbManager.createTable() != true) {
    cout << "Can't create Table: " << dbManager.getLastError() << endl;
    return 1;
  }

  // Create and save timestamp
  Timestamp ts(TIMEZONE);
  ts.print();

  if (dbManager.insertTimestamp(ts.getTimezone(), ts.getFormattedTime(),
                                ts.getTypeString()) != true) {
    cout << "Can't insert Timestamp: " << dbManager.getLastError() << endl;
    return 1;
  }

  return 0;
}

//================================================================================
//================================================================================

Timestamp::Timestamp(const string &timezone)
    : timezone_(timezone),
      zoned_time_(locate_zone(timezone), system_clock::now()) {

  // Determine type based on hour
  auto local_time = zoned_time_.get_local_time();
  auto date_part = floor<days>(local_time);
  auto time_of_day = hh_mm_ss{local_time - date_part};

  type_ = (time_of_day.hours().count() < 9) ? TimestampType::KOMMEN
                                            : TimestampType::GEHEN;

  // Format the timestamp
  formatted_time_ = format("{:%Y-%m-%d %H:%M:%S}", zoned_time_);
}

string Timestamp::getTimezone() const { return timezone_; }
string Timestamp::getFormattedTime() const { return formatted_time_; }
TimestampType Timestamp::getType() const { return type_; }

string Timestamp::getTypeString() const {
  return (type_ == TimestampType::KOMMEN) ? "Kommen" : "Gehen";
}

void Timestamp::print() const {
  cout << "Timezone: " << timezone_ << endl;
  cout << "Time: " << formatted_time_ << endl;
  cout << "Type: " << getTypeString() << endl;
}

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
    sqlite3_bind_text(stmt, 2, formatedTimestamp.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, type.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
      cout << type << " timestamp saved: " << formatedTimestamp << endl;
      return true;
    } else {
      return false;
    }

    sqlite3_finalize(stmt);
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
