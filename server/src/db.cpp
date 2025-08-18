#include "db.h"
#include <stdexcept>

namespace messenger {
Database::Database(const std::string &conninfo) : _conn(conninfo) {
    if (!_conn.is_open()) throw std::runtime_error("Failed to open Postgres connection");
}
pqxx::connection &Database::conn() { return _conn; }
}
