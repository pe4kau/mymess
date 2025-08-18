#pragma once
#include <string>
#include <future>
#include <memory>
#include <vector>
#include "INetworkClient.hpp"

namespace messenger {

class ChatManager {
public:
  explicit ChatManager(std::shared_ptr<INetworkClient> client)
    : client_(std::move(client)) {}

  std::future<std::string> SendEncrypted(const OutgoingMessage& msg) {
    return client_->SendMessageAsync(msg);
  }

  void Subscribe(const std::string& chat_id, INetworkClient::MessageHandler handler) {
    client_->StartReceiveLoop(chat_id, std::move(handler));
  }

  void Unsubscribe() {
    client_->StopReceiveLoop();
  }

private:
  std::shared_ptr<INetworkClient> client_;
};

} // namespace messenger
