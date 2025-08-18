#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <optional>
#include <stdexcept>

namespace messenger {

// Simplified Double Ratchet interface backed by OpenSSL primitives.
class CryptoEngine {
public:
  struct KeyPair { std::vector<uint8_t> priv; std::vector<uint8_t> pub; };
  struct Header { std::vector<uint8_t> dh_pub; uint32_t pn{}; uint32_t n{}; };

  struct SessionState {
    std::vector<uint8_t> root_key;
    std::vector<uint8_t> chain_key_send;
    std::vector<uint8_t> chain_key_recv;
    KeyPair our_dh;
    std::vector<uint8_t> their_dh_pub;
    uint32_t pn{};
    uint32_t n_send{};
    uint32_t n_recv{};
  };

  static KeyPair X25519Generate();
  static std::vector<uint8_t> X25519(const std::vector<uint8_t>& priv, const std::vector<uint8_t>& pub);

  static std::vector<uint8_t> HKDF(const std::vector<uint8_t>& ikm, const std::vector<uint8_t>& salt, const std::vector<uint8_t>& info, size_t out_len);

  static std::vector<uint8_t> AEAD_Encrypt(const std::vector<uint8_t>& key,
                                           const std::vector<uint8_t>& nonce12,
                                           const std::vector<uint8_t>& aad,
                                           const std::vector<uint8_t>& plaintext);
  static std::vector<uint8_t> AEAD_Decrypt(const std::vector<uint8_t>& key,
                                           const std::vector<uint8_t>& nonce12,
                                           const std::vector<uint8_t>& aad,
                                           const std::vector<uint8_t>& ciphertext);

  // Initialize session from a pre-key bundle (X3DH simplified as DH + HKDF)
  static SessionState InitOutbound(const std::vector<uint8_t>& our_identity_priv,
                                   const std::vector<uint8_t>& their_identity_pub,
                                   const std::vector<uint8_t>& their_signed_pre_key_pub,
                                   const std::optional<std::vector<uint8_t>>& their_one_time_pre_key_pub);

  static SessionState InitInbound(const std::vector<uint8_t>& our_identity_priv,
                                  const KeyPair& our_signed_pre_key,
                                  const std::optional<KeyPair>& our_one_time_pre_key,
                                  const std::vector<uint8_t>& their_identity_pub,
                                  const std::vector<uint8_t>& their_ephemeral_pub);

  // Perform DH ratchet when header contains new DH
  static void RatchetStep(SessionState& state, const std::vector<uint8_t>& new_their_dh);

  static std::vector<uint8_t> DeriveNonce(uint32_t n);

  static std::vector<uint8_t> Encrypt(SessionState& state, const std::vector<uint8_t>& plaintext, Header& out_header);

  static std::vector<uint8_t> Decrypt(SessionState& state, const Header& header, const std::vector<uint8_t>& ciphertext);
};

} // namespace messenger
