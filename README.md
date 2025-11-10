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

## Entity-Relationship Diagram

```mermaid
erDiagram
    timestamps {
        INTEGER id "PK, AUTOINCREMENT"
        TEXT timezone "NOT NULL"
        TEXT timestamp "NOT NULL"
        TEXT type "NOT NULL (Kommen/Gehen)"
        INTEGER session_id "NOT NULL, FK"
        DATETIME created_at "DEFAULT CURRENT_TIMESTAMP"
    }

    dailyhours {
        INTEGER id "PK, AUTOINCREMENT"
        TEXT date "NOT NULL"
        REAL hours "NOT NULL"
        INTEGER session_id "NOT NULL, FK"
        string constraint "UNIQUE(date, hours)"
    }

    timestamps ||--|| dailyhours : "linked by session_id"
```
