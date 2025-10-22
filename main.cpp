#include <chrono>
#include <format>
#include <iostream>
using namespace std;
using namespace chrono;

int main() {
  auto now = system_clock::now();

  // Get specific timezone
  auto ny_tz = locate_zone("America/New_York");
  auto berlin_tz = locate_zone("Europe/Berlin");

  // Convert to zoned_time
  auto ny_time = zoned_time{ny_tz, now};
  auto berlin_time = zoned_time{berlin_tz, now};

  cout << format("New York: {:%Y-%m-%d %H:%M:%S %Z}\n", ny_time);
  cout << format("Berlin: {:%Y-%m-%d %H:%M:%S %Z}\n", berlin_time);

  return 0;
}
