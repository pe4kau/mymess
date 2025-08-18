#pragma once
#include "proto/messenger.grpc.pb.h"
#include "db.h"
#include <grpcpp/grpcpp.h>

namespace messenger {
class KeyServiceImpl final : public messenger::KeyService::Service {
public:
    KeyServiceImpl(Database &db);
    grpc::Status UploadPreKey(grpc::ServerContext* context, const messenger::UploadPreKeyRequest* request,
                              messenger::UploadPreKeyResponse* response) override;
    grpc::Status GetPreKey(grpc::ServerContext* context, const messenger::GetPreKeyRequest* request,
                           messenger::GetPreKeyResponse* response) override;
private:
    Database &_db;
};
}
