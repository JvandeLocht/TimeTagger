#include "TimeManager.h"
#include <chrono>
#include <format>
#include <iostream>

TimeManager::TimeManager(const std::string &timezone) : timezone_(timezone) {}

std::string TimeManager::getTimezone() const { return timezone_; }
std::string TimeManager::getFormattedTime() const { return formatted_time_; }
TimestampType TimeManager::getType() const { return type_; }

std::string TimeManager::getTypeString() const {
    return (type_ == TimestampType::KOMMEN) ? "Kommen" : "Gehen";
}

bool TimeManager::createTimestamp() {
    auto zoned_time_ = std::chrono::zoned_time(
        std::chrono::locate_zone(timezone_), std::chrono::system_clock::now());

    // Determine type based on hour
    auto local_time = zoned_time_.get_local_time();
    auto date_part = floor<std::chrono::days>(local_time);
    auto time_of_day = std::chrono::hh_mm_ss{local_time - date_part};

    type_ = (time_of_day.hours().count() < 9) ? TimestampType::KOMMEN
                                              : TimestampType::GEHEN;

    // Format the timestamp
    formatted_time_ = format("{:%Y-%m-%d %H:%M:%S}", zoned_time_);

    return true;
}

void TimeManager::print() const {
    std::cout << "Timezone: " << timezone_ << std::endl;
    std::cout << "Time: " << formatted_time_ << std::endl;
    std::cout << "Type: " << getTypeString() << std::endl;
}
