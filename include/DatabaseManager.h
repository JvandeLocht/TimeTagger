#include <sqlite3.h>
#include <string>

class DatabaseManager {
  private:
    sqlite3 *db_;
    std::string filepath_;

  public:
    DatabaseManager(const std::string &filepath);
    ~DatabaseManager();

    // Prevent copying (database connections shouldn't be copied)
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    bool connect();
    bool createTable();
    void printTableShell();
    bool insertTimestamp(std::string timezone, std::string formatedTimestamp,
                         std::string type);
    std::string getLastError() const;
};
