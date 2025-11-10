This is just a playground to learn c++.

## Class Diagram
```mermaid
classDiagram
    direction TB
    
    %% Data Types
    class TimestampType {
        <<enumeration>>
        KOMMEN
        GEHEN
    }

    class WorkingHours {
        <<struct>>
        +string date
        +vector~double~ hours
        +double workBreak
        +double minHoursForWorkBreak
        +int kommen_count
        +int gehen_count
    }

    %% Core Components
    class TimeManager {
        -string timezone_
        -string formatted_time_
        +optional~TimestampType~ type
        +TimeManager(string timezone)
        +getTimezone() string
        +getFormattedTime() string
        +getTypeString() string
        +createTimestamp() bool
        +setFormattedTime(string& formatted_time) bool
        +print() void
    }

    class DatabaseManager {
        -SQLite::Database db_
        -string filepath_
        -parseTimestamp(string) optional~time_point~
        +DatabaseManager(string filepath)
        +createTableTimestamps() bool
        +createTableDailyHours() bool
        +populateDailyHours() bool
        +printTimestampsTable() void
        +insertTimestamp(string, string, string) bool
        +getLastError() string
        +calculateDailyHours(string date) WorkingHours
    }

    class Main {
        <<application>>
        +main(int argc, char** argv) int
        -string TIMEZONE
    }

    %% External Libraries
    class SQLiteCpp {
        <<library>>
    }

    class CLI11 {
        <<library>>
    }

    class Tabulate {
        <<library>>
    }

    class TomlPlusPlus {
        <<library>>
    }

    %% Relationships - organized to minimize crossings
    Main --> TimeManager
    Main --> DatabaseManager
    Main ..> CLI11
    Main ..> TomlPlusPlus
    
    TimeManager --> TimestampType
    
    DatabaseManager ..> WorkingHours
    DatabaseManager --> SQLiteCpp
    DatabaseManager ..> Tabulate
```
