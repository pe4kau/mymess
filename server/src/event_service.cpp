#include "event_service.h"
#include <chrono>
#include <thread>

namespace messenger {

EventServiceImpl::EventServiceImpl(RedisClient &redis): _redis(redis) {}

grpc::Status EventServiceImpl::SubscribeEvents(grpc::ServerContext* context, const messenger::SubscribeRequest* request,
                                               grpc::ServerWriter<messenger::Event>* writer) {
    std::string channel = "events:" + request->username();
    // subscribe and forward messages to writer
    _redis.subscribe(channel, [&](const std::string &msg){
        messenger::Event ev;
        ev.set_type("message");
        ev.set_payload(msg);
        ev.mutable_timestamp()->set_seconds(time(NULL));
        // attempt to write; if fails (client gone) we just stop
        writer->Write(ev);
    });
    // block until context is cancelled
    while(!context->IsCancelled()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    return grpc::Status::OK;
}

}
