#include <gtest/gtest.h>
#include "messenger/KeyManager.hpp"
#include "messenger/INetworkClient.hpp"
#include <memory>
#include <queue>

using namespace messenger;

// Simple fake network client for unit test
class FakeClient : public INetworkClient {
public:
  std::future<LoginResult> LoginAsync(const std::string&, const std::string&) override {
    return std::async(std::launch::deferred, []{ return LoginResult{"a","r",0}; });
  }
  std::future<std::string> RefreshAsync(const std::string& r) override {
    return std::async(std::launch::deferred, [r]{ return r; });
  }
  std::future<std::string> SendMessageAsync(const OutgoingMessage&) override {
    return std::async(std::launch::deferred, []{ return std::string("msgid"); });
  }
  void StartReceiveLoop(const std::string&, MessageHandler) override {}
  void StopReceiveLoop() override {}
  std::future<bool> PublishPreKeysAsync(const std::vector<uint8_t>& identity_key,
                                        const SignedPreKey& spk,
                                        const std::vector<PreKey>& one_time) override {
    stored_idk = identity_key; stored_spk = spk; stored_otk = one_time;
    return std::async(std::launch::deferred, []{ return true; });
  }
  std::future<PreKeyBundle> GetPreKeyBundleAsync(const std::string& user_id) override {
    return std::async(std::launch::deferred, [this]{
      PreKeyBundle b; b.identity_key = stored_idk; b.signed_pre_key = stored_spk; b.one_time_pre_key = stored_otk.front(); return b;
    });
  }
  std::vector<uint8_t> stored_idk;
  SignedPreKey stored_spk;
  std::vector<PreKey> stored_otk;
};

TEST(KeyManager, PublishAndFetchBundle) {
  auto client = std::make_shared<FakeClient>();
  KeyManager km(client);

  std::vector<uint8_t> idk = {1,2,3};
  SignedPreKey spk{42, {4,5,6}, {7,8}};
  std::vector<PreKey> otk = { PreKey{7, {9,10,11}} };

  auto ok = km.PublishPreKeys(idk, spk, otk).get();
  ASSERT_TRUE(ok);

  auto bundle = km.GetBundle("userB").get();
  ASSERT_EQ(bundle.signed_pre_key.id, 42);
  ASSERT_EQ(bundle.one_time_pre_key.id, 7);
  ASSERT_EQ(bundle.identity_key.size(), 3);
}
