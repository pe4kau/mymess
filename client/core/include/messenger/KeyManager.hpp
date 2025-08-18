#pragma once
#include <string>
#include <future>
#include <vector>
#include <memory>
#include "INetworkClient.hpp"

namespace messenger {

struct SessionId { std::string chat_id; };

class KeyManager {
public:
  explicit KeyManager(std::shared_ptr<INetworkClient> client)
    : client_(std::move(client)) {}

  // Publish a fresh batch of pre-keys
  std::future<bool> PublishPreKeys(const std::vector<uint8_t>& identity_key,
                                   const SignedPreKey& spk,
                                   const std::vector<PreKey>& one_time) {
    return client_->PublishPreKeysAsync(identity_key, spk, one_time);
  }

  // Fetch bundle for peer and return it
  std::future<PreKeyBundle> GetBundle(const std::string& user_id) {
    return client_->GetPreKeyBundleAsync(user_id);
  }

private:
  std::shared_ptr<INetworkClient> client_;
};

} // namespace messenger
