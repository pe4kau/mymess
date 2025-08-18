#pragma once
#include "proto/messenger.grpc.pb.h"
#include "db.h"
#include <grpcpp/grpcpp.h>

namespace messenger {
class AuthServiceImpl final : public messenger::AuthService::Service {
public:
    AuthServiceImpl(Database &db);
    grpc::Status Register(grpc::ServerContext* context, const messenger::RegisterRequest* request,
                          messenger::AuthResponse* response) override;
    grpc::Status Login(grpc::ServerContext* context, const messenger::LoginRequest* request,
                       messenger::AuthResponse* response) override;
    grpc::Status Refresh(grpc::ServerContext* context, const messenger::RefreshRequest* request,
                         messenger::AuthResponse* response) override;
private:
    Database &_db;
    std::string hash_password(const std::string &password);
    bool verify_password(const std::string &password, const std::string &hash);
};
