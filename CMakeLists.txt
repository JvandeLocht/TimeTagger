cmake_minimum_required(VERSION 3.20)
project(TimingTool)
add_executable(timing-tool main.cpp)
target_link_libraries(timing-tool sqlite3)
