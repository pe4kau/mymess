#pragma once
#include "proto/messenger.grpc.pb.h"
#include "redis_client.h"
#include <grpcpp/grpcpp.h>

namespace messenger {
class EventServiceImpl final : public messenger::EventService::Service {
public:
    EventServiceImpl(RedisClient &redis);
    grpc::Status SubscribeEvents(grpc::ServerContext* context, const messenger::SubscribeRequest* request,
                                 grpc::ServerWriter<messenger::Event>* writer) override;
private:
    RedisClient &_redis;
};
}
