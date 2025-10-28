#include <string>

enum class TimestampType {
    KOMMEN, // Arriving (before 9 AM)
    GEHEN   // Leaving (after 9 AM)
};

class Timestamp {
  private:
    std::string timezone_;
    TimestampType type_;
    std::string formatted_time_;

  public:
    Timestamp(const std::string &timezone);

    std::string getTimezone() const;
    std::string getFormattedTime() const;
    TimestampType getType() const;
    std::string getTypeString() const;

    void print() const;
    bool createTimestamp();
};
