#pragma once
#include <string>
#include <vector>
#include <optional>
#include <sqlite3.h>

namespace messenger {

class SessionStore {
public:
  explicit SessionStore(const std::string& db_path, const std::string& key = "") {
    if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK) {
      throw std::runtime_error("Failed to open database");
    }
    // SQLCipher keying (ignored if not compiled with SQLCipher)
    if (!key.empty()) {
      sqlite3_exec(db_, ("PRAGMA key = '" + key + "';").c_str(), nullptr, nullptr, nullptr);
    }
    const char* schema = R"SQL(
    CREATE TABLE IF NOT EXISTS sessions(
      chat_id TEXT PRIMARY KEY,
      state BLOB NOT NULL
    );
    CREATE TABLE IF NOT EXISTS prekeys(
      id INTEGER PRIMARY KEY,
      pub BLOB NOT NULL,
      priv BLOB NOT NULL
    );
    )SQL";
    char* err = nullptr;
    if (sqlite3_exec(db_, schema, nullptr, nullptr, &err) != SQLITE_OK) {
      std::string e = err ? err : "";
      sqlite3_free(err);
      throw std::runtime_error("Failed to init schema: " + e);
    }
  }

  ~SessionStore() { if (db_) sqlite3_close(db_); }

  void SaveSession(const std::string& chat_id, const std::vector<uint8_t>& blob) {
    sqlite3_stmt* st = nullptr;
    sqlite3_prepare_v2(db_, "REPLACE INTO sessions(chat_id,state) VALUES(?,?);", -1, &st, nullptr);
    sqlite3_bind_text(st, 1, chat_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_blob(st, 2, blob.data(), (int)blob.size(), SQLITE_TRANSIENT);
    sqlite3_step(st);
    sqlite3_finalize(st);
  }

  std::optional<std::vector<uint8_t>> LoadSession(const std::string& chat_id) {
    sqlite3_stmt* st = nullptr;
    sqlite3_prepare_v2(db_, "SELECT state FROM sessions WHERE chat_id=?;", -1, &st, nullptr);
    sqlite3_bind_text(st, 1, chat_id.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(st) == SQLITE_ROW) {
      const void* data = sqlite3_column_blob(st, 0);
      int size = sqlite3_column_bytes(st, 0);
      std::vector<uint8_t> out((const uint8_t*)data, (const uint8_t*)data + size);
      sqlite3_finalize(st);
      return out;
    }
    sqlite3_finalize(st);
    return std::nullopt;
  }

private:
  sqlite3* db_{};
};

} // namespace messenger
