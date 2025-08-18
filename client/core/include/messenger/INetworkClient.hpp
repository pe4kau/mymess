#pragma once
#include <string>
#include <future>
#include <vector>
#include <optional>
#include <functional>
#include <memory>
#include <cstdint>

namespace messenger {

struct LoginResult {
  std::string access_token;
  std::string refresh_token;
  int64_t expires_at_unix{};
};

struct Ciphertext {
  std::vector<uint8_t> header;
  std::vector<uint8_t> body;
};

struct OutgoingMessage {
  std::string chat_id;
  std::string recipient_user_id;
  Ciphertext encrypted;
};

struct IncomingMessage {
  std::string chat_id;
  std::string sender_user_id;
  std::vector<uint8_t> header;
  std::vector<uint8_t> ciphertext;
  int64_t sent_at_unix{};
};

struct PreKey {
  uint32_t id{};
  std::vector<uint8_t> public_key;
};

struct SignedPreKey {
  uint32_t id{};
  std::vector<uint8_t> public_key;
  std::vector<uint8_t> signature;
};

struct PreKeyBundle {
  std::vector<uint8_t> identity_key;
  SignedPreKey signed_pre_key;
  PreKey one_time_pre_key;
};

// async network client interface
class INetworkClient {
public:
  virtual ~INetworkClient() = default;

  virtual std::future<LoginResult> LoginAsync(const std::string& username, const std::string& password) = 0;
  virtual std::future<std::string> RefreshAsync(const std::string& refresh_token) = 0;

  virtual std::future<std::string> SendMessageAsync(const OutgoingMessage& msg) = 0;

  // Long-lived subscription for receiving messages (threaded). Handler must be thread-safe.
  using MessageHandler = std::function<void(const IncomingMessage&)>;
  virtual void StartReceiveLoop(const std::string& chat_id, MessageHandler on_message) = 0;
  virtual void StopReceiveLoop() = 0;

  virtual std::future<bool> PublishPreKeysAsync(const std::vector<uint8_t>& identity_key,
                                               const SignedPreKey& spk,
                                               const std::vector<PreKey>& one_time) = 0;

  virtual std::future<PreKeyBundle> GetPreKeyBundleAsync(const std::string& user_id) = 0;
};

} // namespace messenger
