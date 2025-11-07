#include <SQLiteCpp/SQLiteCpp.h>
#include <chrono>
#include <optional>
#include <string>

#pragma once

struct WorkingHours {
    std::string date;
    std::vector<double> hours;
    double workBreak;
    double minHoursForWorkBreak;
    int kommen_count;
    int gehen_count;
};

class DatabaseManager {
  private:
    SQLite::Database db_;
    std::string filepath_;

    std::optional<std::chrono::system_clock::time_point>
    parseTimestamp(const std::string &timestamp_str) const;

  public:
    DatabaseManager(const std::string &filepath);

    bool createTableTimestamps();
    bool createTableDailyHours();
    bool populateDailyHours();
    void printTimestampsTable();
    bool insertTimestamp(std::string timezone, std::string formatedTimestamp,
                         std::string type);
    std::string getLastError() const;

    WorkingHours calculateDailyHours(const std::string &date);
};
