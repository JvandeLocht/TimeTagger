#include <chrono>
#include <sqlite3.h>
#include <string>

struct WorkingHours {
    std::string date;
    double hours;
    int kommen_count;
    int gehen_count;
};

class DatabaseManager {
  private:
    sqlite3 *db_;
    std::string filepath_;

    std::chrono::system_clock::time_point
    parseTimestamp(const std::string &timestamp_str) const;

  public:
    DatabaseManager(const std::string &filepath);
    ~DatabaseManager();

    // Prevent copying (database connections shouldn't be copied)
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    bool connect();
    bool createTableTimestamps();
    bool createTableDailyHours();
    bool populateDailyHours();
    void printTableShell();
    bool insertTimestamp(std::string timezone, std::string formatedTimestamp,
                         std::string type);
    std::string getLastError() const;

    WorkingHours calculateDailyHours(const std::string &date);
};
