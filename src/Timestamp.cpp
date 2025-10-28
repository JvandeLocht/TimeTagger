#include "Timestamp.h"
#include <chrono>
#include <format>
#include <iostream>

using namespace std;
using namespace chrono;

Timestamp::Timestamp(const string &timezone) : timezone_(timezone) {}

string Timestamp::getTimezone() const { return timezone_; }
string Timestamp::getFormattedTime() const { return formatted_time_; }
TimestampType Timestamp::getType() const { return type_; }

string Timestamp::getTypeString() const {
    return (type_ == TimestampType::KOMMEN) ? "Kommen" : "Gehen";
}

bool Timestamp::createTimestamp() {
    auto zoned_time_ = zoned_time(locate_zone(timezone_), system_clock::now());

    // Determine type based on hour
    auto local_time = zoned_time_.get_local_time();
    auto date_part = floor<days>(local_time);
    auto time_of_day = hh_mm_ss{local_time - date_part};

    type_ = (time_of_day.hours().count() < 9) ? TimestampType::KOMMEN
                                              : TimestampType::GEHEN;

    // Format the timestamp
    formatted_time_ = format("{:%Y-%m-%d %H:%M:%S}", zoned_time_);

    return true;
}

void Timestamp::print() const {
    cout << "Timezone: " << timezone_ << endl;
    cout << "Time: " << formatted_time_ << endl;
    cout << "Type: " << getTypeString() << endl;
}
