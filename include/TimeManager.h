#include <optional>
#include <string>

#pragma once

enum class TimestampType {
    KOMMEN, // Arriving (before 9 AM)
    GEHEN   // Leaving (after 9 AM)
};

class TimeManager {
  private:
    std::string timezone_;
    std::string formatted_time_;

  public:
    TimeManager(const std::string &timezone);
    std::optional<TimestampType> type;

    std::string getTimezone() const;
    std::string getFormattedTime() const;
    std::string getTypeString() const;

    void print() const;
    bool createTimestamp();
    bool setFormattedTime(std::string &formatted_time);
};
