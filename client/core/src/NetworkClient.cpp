#include "messenger/INetworkClient.hpp"
#include "messenger/CryptoEngine.hpp"
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include "messenger.pb.h"
#include "messenger.grpc.pb.h"
#include <atomic>
#include <thread>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

namespace messenger {

class NetworkClient : public INetworkClient {
public:
  NetworkClient(const std::string& server_addr, const std::string& root_pem, const std::string& auth_header_prefix = "Bearer ")
  : auth_header_prefix_(auth_header_prefix) {
    grpc::SslCredentialsOptions ssl_opts;
    ssl_opts.pem_root_certs = root_pem;
    auto creds = grpc::SslCredentials(ssl_opts);
    channel_ = grpc::CreateChannel(server_addr, creds);
    auth_ = messenger::AuthService::NewStub(channel_);
    chat_ = messenger::ChatService::NewStub(channel_);
    keys_ = messenger::KeyService::NewStub(channel_);
  }

  std::future<LoginResult> LoginAsync(const std::string& username, const std::string& password) override {
    return std::async(std::launch::async, [this, username, password]{
      ClientContext ctx;
      messenger::LoginRequest req;
      req.set_username(username);
      req.set_password(password);
      messenger::LoginResponse resp;
      auto s = auth_->Login(&ctx, req, &resp);
      if (!s.ok()) throw std::runtime_error(s.error_message());
      token_ = resp.access_token();
      return LoginResult{resp.access_token(), resp.refresh_token(), resp.expires_at_unix()};
    });
  }

  std::future<std::string> RefreshAsync(const std::string& refresh_token) override {
    return std::async(std::launch::async, [this, refresh_token]{
      ClientContext ctx;
      messenger::RefreshRequest req;
      req.set_refresh_token(refresh_token);
      messenger::RefreshResponse resp;
      auto s = auth_->Refresh(&ctx, req, &resp);
      if (!s.ok()) throw std::runtime_error(s.error_message());
      token_ = resp.access_token();
      return token_;
    });
  }

  std::future<std::string> SendMessageAsync(const OutgoingMessage& msg) override {
    return std::async(std::launch::async, [this, msg]{
      ClientContext ctx;
      if (!token_.empty()) ctx.AddMetadata("authorization", auth_header_prefix_ + token_);
      messenger::SendMessageRequest req;
      auto* env = req.mutable_message();
      env->set_chat_id(msg.chat_id);
      env->set_recipient_user_id(msg.recipient_user_id);
      env->set_ciphertext(std::string(reinterpret_cast<const char*>(msg.encrypted.body.data()), msg.encrypted.body.size()));
      env->set_header(std::string(reinterpret_cast<const char*>(msg.encrypted.header.data()), msg.encrypted.header.size()));
      messenger::SendMessageResponse resp;
      auto s = chat_->SendMessage(&ctx, req, &resp);
      if (!s.ok()) throw std::runtime_error(s.error_message());
      return resp.message_id();
    });
  }

  void StartReceiveLoop(const std::string& chat_id, MessageHandler on_message) override {
    StopReceiveLoop();
    stop_.store(false);
    recv_thread_ = std::thread([this, chat_id, on_message]{
      ClientContext ctx;
      if (!token_.empty()) ctx.AddMetadata("authorization", auth_header_prefix_ + token_);
      auto stream = chat_->Receive(&ctx);
      messenger::ReceiveRequest req;
      req.set_chat_id(chat_id);
      stream->Write(req);
      messenger::ReceiveEvent ev;
      while (!stop_.load() && stream->Read(&ev)) {
        if (ev.has_message()) {
          const auto& m = ev.message();
          IncomingMessage im;
          im.chat_id = m.chat_id();
          im.sender_user_id = m.sender_user_id();
          auto h = m.header();
          im.header.assign(h.begin(), h.end());
          auto c = m.ciphertext();
          im.ciphertext.assign(c.begin(), c.end());
          im.sent_at_unix = m.sent_at_unix();
          on_message(im);
        }
      }
      stream->WritesDone();
      Status s = stream->Finish();
    });
  }

  void StopReceiveLoop() override {
    if (recv_thread_.joinable()) {
      stop_.store(true);
      recv_thread_.join();
    }
  }

  std::future<bool> PublishPreKeysAsync(const std::vector<uint8_t>& identity_key,
                                        const SignedPreKey& spk,
                                        const std::vector<PreKey>& one_time) override {
    return std::async(std::launch::async, [this, identity_key, spk, one_time]{
      ClientContext ctx;
      if (!token_.empty()) ctx.AddMetadata("authorization", auth_header_prefix_ + token_);
      messenger::PublishPreKeysRequest req;
      req.mutable_identity_key()->set_public_key(std::string(reinterpret_cast<const char*>(identity_key.data()), identity_key.size()));
      auto* out_spk = req.mutable_signed_pre_key();
      out_spk->set_id(spk.id);
      out_spk->set_public_key(std::string(reinterpret_cast<const char*>(spk.public_key.data()), spk.public_key.size()));
      out_spk->set_signature(std::string(reinterpret_cast<const char*>(spk.signature.data()), spk.signature.size()));
      for (const auto& pk : one_time) {
        auto* out_pk = req.add_one_time_pre_keys();
        out_pk->set_id(pk.id);
        out_pk->set_public_key(std::string(reinterpret_cast<const char*>(pk.public_key.data()), pk.public_key.size()));
      }
      messenger::PublishPreKeysResponse resp;
      auto s = keys_->PublishPreKeys(&ctx, req, &resp);
      if (!s.ok()) throw std::runtime_error(s.error_message());
      return resp.ok();
    });
  }

  std::future<PreKeyBundle> GetPreKeyBundleAsync(const std::string& user_id) override {
    return std::async(std::launch::async, [this, user_id]{
      ClientContext ctx;
      if (!token_.empty()) ctx.AddMetadata("authorization", auth_header_prefix_ + token_);
      messenger::GetPreKeyBundleRequest req;
      req.set_user_id(user_id);
      messenger::PreKeyBundle resp;
      auto s = keys_->GetPreKeyBundle(&ctx, req, &resp);
      if (!s.ok()) throw std::runtime_error(s.error_message());
      PreKeyBundle out;
      auto idk = resp.identity_key().public_key();
      out.identity_key.assign(idk.begin(), idk.end());
      out.signed_pre_key.id = resp.signed_pre_key().id();
      auto spkpub = resp.signed_pre_key().public_key();
      out.signed_pre_key.public_key.assign(spkpub.begin(), spkpub.end());
      auto sig = resp.signed_pre_key().signature();
      out.signed_pre_key.signature.assign(sig.begin(), sig.end());
      out.one_time_pre_key.id = resp.one_time_pre_key().id();
      auto pkpub = resp.one_time_pre_key().public_key();
      out.one_time_pre_key.public_key.assign(pkpub.begin(), pkpub.end());
      return out;
    });
  }

private:
  std::shared_ptr<Channel> channel_;
  std::unique_ptr<messenger::AuthService::Stub> auth_;
  std::unique_ptr<messenger::ChatService::Stub> chat_;
  std::unique_ptr<messenger::KeyService::Stub> keys_;
  std::atomic<bool> stop_{true};
  std::thread recv_thread_;
  std::string token_;
  std::string auth_header_prefix_;
};

} // namespace messenger
