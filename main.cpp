#include <chrono>
#include <format>
#include <iostream>
#include <sqlite3.h>
using namespace std;
using namespace chrono;

void insertTimestamp(sqlite3 *db, const string &timezone,
                     const string &readable);
string createTimestamp(const string &timezone);

int main() {
  sqlite3 *db;
  string const TIMEZONE = "Europe/Berlin";

  if (sqlite3_open("timestamps.db", &db) != SQLITE_OK) {
    cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
    return 1;
  }

  const char *createTableSQL = "CREATE TABLE IF NOT EXISTS timestamps ("
                               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                               "timezone TEXT NOT NULL,"
                               "timestamp TEXT NOT NULL,"
                               "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
                               ");";

  sqlite3_exec(db, createTableSQL, nullptr, nullptr, nullptr);

  insertTimestamp(db, TIMEZONE, createTimestamp(TIMEZONE));

  sqlite3_close(db);

  return 0;
}

string createTimestamp(const string &timezone) {
  string timestamp =
      format("{:%Y-%m-%d %H:%M}",
             zoned_time{locate_zone(timezone), system_clock::now()});
  return timestamp;
}

void insertTimestamp(sqlite3 *db, const string &timezone,
                     const string &readable) {
  const char *sql = "INSERT INTO timestamps (timezone, timestamp) "
                    "VALUES (?, ?);";

  sqlite3_stmt *stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_text(stmt, 1, timezone.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, readable.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
      cout << timezone << " timestamp saved: " << readable << endl;
    } else {
      cerr << "Insert failed: " << sqlite3_errmsg(db) << endl;
    }

    sqlite3_finalize(stmt);
  }
}
