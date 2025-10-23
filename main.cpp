#include <chrono>
#include <format>
#include <iostream>
#include <sqlite3.h>
using namespace std;
using namespace chrono;

void insertTimestamp(sqlite3 *db, const string &timezone, int64_t unix_ts,
                     const string &readable) {
  const char *sql =
      "INSERT INTO timestamps (timezone, timestamp_unix, timestamp_readable) "
      "VALUES (?, ?, ?);";

  sqlite3_stmt *stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, timezone.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, unix_ts);
    sqlite3_bind_text(stmt, 3, readable.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
      cout << timezone << " timestamp saved: " << readable << endl;
    } else {
      cerr << "Insert failed: " << sqlite3_errmsg(db) << endl;
    }

    sqlite3_finalize(stmt);
  }
}

int main() {
  sqlite3 *db;

  if (sqlite3_open("timestamps.db", &db) != SQLITE_OK) {
    cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
    return 1;
  }

  const char *createTableSQL = "CREATE TABLE IF NOT EXISTS timestamps ("
                               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                               "timezone TEXT NOT NULL,"
                               "timestamp_unix INTEGER NOT NULL,"
                               "timestamp_readable TEXT NOT NULL,"
                               "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
                               ");";

  sqlite3_exec(db, createTableSQL, nullptr, nullptr, nullptr);

  auto now = system_clock::now();
  auto unix_timestamp = duration_cast<seconds>(now.time_since_epoch()).count();

  // Get specific timezone
  auto berlin_tz = locate_zone("Europe/Berlin");

  // Convert to zoned_time
  auto berlin_time = zoned_time{berlin_tz, now};

  cout << format("Berlin: {:%Y-%m-%d %H:%M}\n", berlin_time);

  string berlin_readable = format("{:%Y-%m-%d %H:%M}", berlin_time);

  insertTimestamp(db, "Europe/Berlin", unix_timestamp, berlin_readable);

  sqlite3_close(db);

  return 0;
}
