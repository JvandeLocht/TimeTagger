cmake_minimum_required(VERSION 3.20)
project(TimingTool)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find CLI11 (header-only library)
find_package(CLI11 CONFIG REQUIRED)
find_package(SQLiteCpp CONFIG REQUIRED)

add_executable(timetagger src/main.cpp src/TimeManager.cpp src/DatabaseManager.cpp)
target_include_directories(timetagger PRIVATE include)
target_link_libraries(timetagger PRIVATE sqlite3 SQLiteCpp CLI11::CLI11)
