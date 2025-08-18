#include "redis_client.h"
#include <hiredis/hiredis.h>
#include <stdexcept>
#include <thread>
#include <iostream>

namespace messenger {
RedisClient::RedisClient(const std::string &uri) : _uri(uri) {
    // uri not parsed in this minimal client; will connect to localhost default
}

RedisClient::~RedisClient() {
}

void RedisClient::publish(const std::string &channel, const std::string &message) {
    redisContext *c = redisConnect("127.0.0.1", 6379);
    if (!c || c->err) {
        if(c) { /* ignore */ }
        return;
    }
    redisCommand(c, "PUBLISH %s %s", channel.c_str(), message.c_str());
    redisFree(c);
}

void RedisClient::subscribe(const std::string &channel, std::function<void(const std::string&)> cb) {
    // Blocking subscribe in separate thread
    std::thread([channel, cb](){
        redisContext *c = redisConnect("127.0.0.1", 6379);
        if (!c || c->err) {
            if(c) redisFree(c);
            return;
        }
        redisReply *reply = (redisReply*)redisCommand(c, "SUBSCRIBE %s", channel.c_str());
        freeReplyObject(reply);
        while (redisGetReply(c, (void**)&reply) == REDIS_OK) {
            if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 3) {
                // message is element 2
                std::string msg(reply->element[2]->str);
                cb(msg);
            }
            freeReplyObject(reply);
        }
        redisFree(c);
    }).detach();
}
}
