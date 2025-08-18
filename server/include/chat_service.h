#pragma once
#include "proto/messenger.grpc.pb.h"
#include "db.h"
#include "redis_client.h"
#include <grpcpp/grpcpp.h>

namespace messenger {
class ChatServiceImpl final : public messenger::ChatService::Service {
public:
    ChatServiceImpl(Database &db, RedisClient &redis);
    grpc::Status CreateChat(grpc::ServerContext* context, const messenger::CreateChatRequest* request,
                            messenger::CreateChatResponse* response) override;
    grpc::Status SendMessage(grpc::ServerContext* context, const messenger::SendMessageRequest* request,
                             messenger::SendMessageResponse* response) override;
    grpc::Status GetHistory(grpc::ServerContext* context, const messenger::GetHistoryRequest* request,
                            messenger::GetHistoryResponse* response) override;
private:
    Database &_db;
    RedisClient &_redis;
};
}
