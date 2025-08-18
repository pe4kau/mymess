#pragma once
#include <string>
#include <future>
#include <memory>
#include "INetworkClient.hpp"

namespace messenger {

class AuthManager {
public:
  explicit AuthManager(std::shared_ptr<INetworkClient> client)
    : client_(std::move(client)) {}

  std::future<LoginResult> Login(const std::string& username, const std::string& password) {
    return client_->LoginAsync(username, password);
  }

  std::future<std::string> Refresh(const std::string& refresh_token) {
    return client_->RefreshAsync(refresh_token);
  }

private:
  std::shared_ptr<INetworkClient> client_;
};

} // namespace messenger
