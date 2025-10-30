#include <string>

enum class TimestampType {
    KOMMEN, // Arriving (before 9 AM)
    GEHEN   // Leaving (after 9 AM)
};

struct WorkingHours {
    std::string date;
    double hours;
    double workBreak;
    double minHoursForWorkBreak;
    int kommen_count;
    int gehen_count;
};

class TimeManager {
  private:
    std::string timezone_;
    TimestampType type_;
    std::string formatted_time_;

  public:
    TimeManager(const std::string &timezone);

    std::string getTimezone() const;
    std::string getFormattedTime() const;
    TimestampType getType() const;
    std::string getTypeString() const;

    void print() const;
    bool createTimestamp();
};
