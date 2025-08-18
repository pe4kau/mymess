#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "db.h"
#include "redis_client.h"
#include "auth_service.h"
#include "chat_service.h"
#include "event_service.h"
#include "key_service.h"
#include "utils.h"

int main(int argc, char** argv) {
    std::string pg = messenger::getenv_or("DATABASE_URL", "postgresql://postgres:postgres@127.0.0.1:5432/messenger").value();
    std::string redis = messenger::getenv_or("REDIS_URL", "redis://127.0.0.1:6379").value();
    std::string addr = messenger::getenv_or("BIND_ADDR", "0.0.0.0:50051").value();

    try {
        messenger::Database db(pg);
        messenger::RedisClient redisc(redis);

        AuthServiceImpl auth(db);
        ChatServiceImpl chat(db, redisc);
        EventServiceImpl events(redisc);
        KeyServiceImpl keys(db);

        grpc::ServerBuilder builder;
        builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
        builder.RegisterService(&auth);
        builder.RegisterService(&chat);
        builder.RegisterService(&events);
        builder.RegisterService(&keys);

        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        std::cout << "Server listening on " << addr << std::endl;
        server->Wait();
    } catch (const std::exception &e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
