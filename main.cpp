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

  // Save to database
  bool saveToDatabase(sqlite3 *db) const;
};

int main() {
  sqlite3 *db;
  const string TIMEZONE = "Europe/Berlin";

  if (sqlite3_open("timestamps.db", &db) != SQLITE_OK) {
    cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
    return 1;
  }

  // Create table with type column
  const char *createTableSQL = "CREATE TABLE IF NOT EXISTS timestamps ("
                               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                               "timezone TEXT NOT NULL,"
                               "timestamp TEXT NOT NULL,"
                               "type TEXT NOT NULL,"
                               "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
                               ");";

  sqlite3_exec(db, createTableSQL, nullptr, nullptr, nullptr);

  // Create and save timestamp
  Timestamp ts(TIMEZONE);
  ts.print();
  ts.saveToDatabase(db);

  sqlite3_close(db);
  return 0;
}

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

bool Timestamp::saveToDatabase(sqlite3 *db) const {
  const char *sql = "INSERT INTO timestamps (timezone, timestamp, type) "
                    "VALUES (?, ?, ?);";

  sqlite3_stmt *stmt;
  bool success = false;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, timezone_.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, formatted_time_.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, getTypeString().c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
      cout << getTypeString() << " timestamp saved: " << formatted_time_
           << endl;
      success = true;
    } else {
      cerr << "Insert failed: " << sqlite3_errmsg(db) << endl;
    }

    sqlite3_finalize(stmt);
  }

  return success;
}
