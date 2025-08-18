#include "auth_service.h"
#include "utils.h"
#include <argon2.h>
#include <pqxx/pqxx>
#include <random>

using grpc::Status;
using grpc::StatusCode;

namespace messenger {

static std::string random_token() {
    static std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<unsigned long long> dist;
    std::ostringstream ss;
    ss << std::hex << dist(rng) << dist(rng);
    return ss.str();
}

AuthServiceImpl::AuthServiceImpl(Database &db): _db(db) {}

std::string AuthServiceImpl::hash_password(const std::string &password) {
    // Using Argon2id via argon2 library. For brevity use simple parameters.
    std::string salt = "static_salt_change_in_prod"; // in prod generate per-user random salt
    std::string out(64, '\\0');
    argon2id_hash_raw(2, 1<<16, 1, password.data(), password.size(),
                      salt.data(), salt.size(), (void*)out.data(), out.size());
    return out;
}

bool AuthServiceImpl::verify_password(const std::string &password, const std::string &hash) {
    // naive: recompute and compare
    return hash_password(password) == hash;
}

Status AuthServiceImpl::Register(ServerContext* context, const RegisterRequest* request, AuthResponse* response) {
    try {
        auto &conn = _db.conn();
        pqxx::work w(conn);
        // store user record: username, password_hash, identity_key, device_id, pre_key
        std::string h = hash_password(request->password());
        w.exec_params("INSERT INTO users (username, password_hash, identity_key) VALUES ($1, $2, $3)",
                      request->username(), pqxx::binarystring(h.data(), h.size()), pqxx::binarystring(request->identity_key()));
        // store prekey as device_prekeys
        w.exec_params("INSERT INTO device_prekeys (username, device_id, pre_key) VALUES ($1, $2, $3)",
                      request->username(), request->device_id(), pqxx::binarystring(request->pre_key()));
        w.commit();
        response->set_success(true);
        response->set_access_token(random_token());
        response->set_refresh_token(random_token());
        response->set_message("Registered");
        return Status::OK;
    } catch (const std::exception &e) {
        return Status(StatusCode::INTERNAL, e.what());
    }
}

Status AuthServiceImpl::Login(ServerContext* context, const LoginRequest* request, AuthResponse* response) {
    try {
        auto &conn = _db.conn();
        pqxx::work w(conn);
        pqxx::result r = w.exec_params("SELECT password_hash FROM users WHERE username=$1", request->username());
        if (r.empty()) return Status(StatusCode::NOT_FOUND, "user not found");
        std::string hash = r[0][0].as<std::string>();
        if (!verify_password(request->password(), hash)) return Status(StatusCode::UNAUTHENTICATED, "bad password");
        response->set_success(true);
        response->set_access_token(random_token());
        response->set_refresh_token(random_token());
        response->set_message("OK");
        return Status::OK;
    } catch (const std::exception &e) {
        return Status(StatusCode::INTERNAL, e.what());
    }
}

Status AuthServiceImpl::Refresh(ServerContext* context, const RefreshRequest* request, AuthResponse* response) {
    // For minimal implementation just return a new access token
    response->set_success(true);
    response->set_access_token(random_token());
    response->set_refresh_token(random_token());
    response->set_message("Refreshed");
    return Status::OK;
}

}
