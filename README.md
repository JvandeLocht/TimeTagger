This is just a playground to learn c++.

## Class Diagram
```mermaid
classDiagram
    direction TB
    
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

    class TimeManager {
        -string timezone_
        -string formatted_time_
        +optional~TimestampType~ type
        +TimeManager(timezone)
        +getTimezone() string
        +getFormattedTime() string
        +getTypeString() string
        +createTimestamp() bool
        +setFormattedTime() bool
        +print() void
    }

    class DatabaseManager {
        -Database db_
        -string filepath_
        +DatabaseManager(filepath)
        +createTableTimestamps() bool
        +createTableDailyHours() bool
        +populateDailyHours() bool
        +printTimestampsTable() void
        +insertTimestamp() bool
        +calculateDailyHours() WorkingHours
    }

    class Main {
        <<application>>
        -string TIMEZONE
        +main() int
    }

    Main --> TimeManager : creates
    Main --> DatabaseManager : creates
    TimeManager --> TimestampType : uses
    DatabaseManager ..> WorkingHours : returns
```
