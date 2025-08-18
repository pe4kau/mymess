#pragma once
#include <string>
#include <future>
#include <memory>
#include "INetworkClient.hpp"

namespace messenger {

class EventManager {
public:
  explicit EventManager(std::shared_ptr<INetworkClient> client)
    : client_(std::move(client)) {}

  void Subscribe(const std::string& user_id, INetworkClient::MessageHandler handler) {
    client_->StartReceiveLoop(user_id, std::move(handler));
  }

  void Unsubscribe() { client_->StopReceiveLoop(); }

private:
  std::shared_ptr<INetworkClient> client_;
};

} // namespace messenger
