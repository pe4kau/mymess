#pragma once
#include <string>
#include <functional>

namespace messenger {
class RedisClient {
public:
    RedisClient(const std::string &uri);
    ~RedisClient();
    void publish(const std::string &channel, const std::string &message);
    // subscribe simplified: callback invoked for each message (blocking)
    void subscribe(const std::string &channel, std::function<void(const std::string&)> cb);
private:
    std::string _uri;
};
}
