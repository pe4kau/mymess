#pragma once
#include <pqxx/pqxx>
#include <string>

namespace messenger {
class Database {
public:
    Database(const std::string &conninfo);
    pqxx::connection &conn();
private:
    pqxx::connection _conn;
};
}
