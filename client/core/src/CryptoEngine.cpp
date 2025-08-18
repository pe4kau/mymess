#include "messenger/CryptoEngine.hpp"
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/rand.h>
#include <cstring>

namespace messenger {

static void ensure(bool ok, const char* msg) { if (!ok) throw std::runtime_error(msg); }

CryptoEngine::KeyPair CryptoEngine::X25519Generate() {
  EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_X25519, nullptr);
  ensure(pctx, "EVP_PKEY_CTX_new_id");
  EVP_PKEY* pkey = nullptr;
  ensure(EVP_PKEY_keygen_init(pctx) == 1, "keygen_init");
  ensure(EVP_PKEY_keygen(pctx, &pkey) == 1, "keygen");
  EVP_PKEY_CTX_free(pctx);
  // extract priv & pub
  std::vector<uint8_t> priv(32), pub(32);
  size_t len = 32;
  ensure(EVP_PKEY_get_raw_private_key(pkey, priv.data(), &len) == 1, "get priv");
  len = 32;
  ensure(EVP_PKEY_get_raw_public_key(pkey, pub.data(), &len) == 1, "get pub");
  EVP_PKEY_free(pkey);
  return {std::move(priv), std::move(pub)};
}

std::vector<uint8_t> CryptoEngine::X25519(const std::vector<uint8_t>& priv, const std::vector<uint8_t>& pub) {
  EVP_PKEY* p_priv = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, nullptr, priv.data(), priv.size());
  EVP_PKEY* p_pub  = EVP_PKEY_new_raw_public_key(EVP_PKEY_X25519, nullptr, pub.data(), pub.size());
  ensure(p_priv && p_pub, "raw keys");
  EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(p_priv, nullptr);
  ensure(ctx, "ctx");
  ensure(EVP_PKEY_derive_init(ctx) == 1, "derive_init");
  ensure(EVP_PKEY_derive_set_peer(ctx, p_pub) == 1, "set_peer");
  size_t secret_len = 0;
  ensure(EVP_PKEY_derive(ctx, nullptr, &secret_len) == 1, "derive size");
  std::vector<uint8_t> secret(secret_len);
  ensure(EVP_PKEY_derive(ctx, secret.data(), &secret_len) == 1, "derive");
  EVP_PKEY_CTX_free(ctx);
  EVP_PKEY_free(p_priv);
  EVP_PKEY_free(p_pub);
  secret.resize(secret_len);
  return secret;
}

std::vector<uint8_t> CryptoEngine::HKDF(const std::vector<uint8_t>& ikm, const std::vector<uint8_t>& salt, const std::vector<uint8_t>& info, size_t out_len) {
  std::vector<uint8_t> out(out_len);
  EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, nullptr);
  ensure(pctx, "hkdf ctx");
  ensure(EVP_PKEY_derive_init(pctx) == 1, "hkdf init");
  ensure(EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256()) == 1, "hkdf md");
  ensure(EVP_PKEY_CTX_set1_hkdf_salt(pctx, salt.data(), salt.size()) == 1, "hkdf salt");
  ensure(EVP_PKEY_CTX_set1_hkdf_key(pctx, ikm.data(), ikm.size()) == 1, "hkdf key");
  ensure(EVP_PKEY_CTX_add1_hkdf_info(pctx, info.data(), info.size()) == 1, "hkdf info");
  size_t len = out.size();
  ensure(EVP_PKEY_derive(pctx, out.data(), &len) == 1, "hkdf derive");
  EVP_PKEY_CTX_free(pctx);
  out.resize(len);
  return out;
}

std::vector<uint8_t> CryptoEngine::AEAD_Encrypt(const std::vector<uint8_t>& key,
                                                const std::vector<uint8_t>& nonce12,
                                                const std::vector<uint8_t>& aad,
                                                const std::vector<uint8_t>& plaintext) {
  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  ensure(ctx, "cipher ctx");
  ensure(EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1, "enc init");
  ensure(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, nonce12.size(), nullptr) == 1, "ivlen");
  ensure(EVP_EncryptInit_ex(ctx, nullptr, nullptr, key.data(), nonce12.data()) == 1, "set key/iv");
  int outl = 0;
  if (!aad.empty()) ensure(EVP_EncryptUpdate(ctx, nullptr, &outl, aad.data(), aad.size()) == 1, "aad");
  std::vector<uint8_t> out(plaintext.size()+16);
  int len = 0, total = 0;
  ensure(EVP_EncryptUpdate(ctx, out.data(), &len, plaintext.data(), plaintext.size()) == 1, "enc update");
  total += len;
  ensure(EVP_EncryptFinal_ex(ctx, out.data()+total, &len) == 1, "final");
  total += len;
  std::vector<uint8_t> tag(16);
  ensure(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag.data()) == 1, "get tag");
  EVP_CIPHER_CTX_free(ctx);
  out.resize(total);
  out.insert(out.end(), tag.begin(), tag.end());
  return out;
}

std::vector<uint8_t> CryptoEngine::AEAD_Decrypt(const std::vector<uint8_t>& key,
                                                const std::vector<uint8_t>& nonce12,
                                                const std::vector<uint8_t>& aad,
                                                const std::vector<uint8_t>& ciphertext) {
  if (ciphertext.size() < 16) throw std::runtime_error("ct too short");
  std::vector<uint8_t> ct(ciphertext.begin(), ciphertext.end()-16);
  std::vector<uint8_t> tag(ciphertext.end()-16, ciphertext.end());
  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  ensure(ctx, "ctx");
  ensure(EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1, "dec init");
  ensure(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, nonce12.size(), nullptr) == 1, "ivlen");
  ensure(EVP_DecryptInit_ex(ctx, nullptr, nullptr, key.data(), nonce12.data()) == 1, "set key/iv");
  int outl = 0;
  if (!aad.empty()) ensure(EVP_DecryptUpdate(ctx, nullptr, &outl, aad.data(), aad.size()) == 1, "aad");
  std::vector<uint8_t> out(ct.size());
  int len = 0, total = 0;
  ensure(EVP_DecryptUpdate(ctx, out.data(), &len, ct.data(), ct.size()) == 1, "dec update");
  total += len;
  ensure(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tag.size(), tag.data()) == 1, "set tag");
  int ok = EVP_DecryptFinal_ex(ctx, out.data()+total, &len);
  EVP_CIPHER_CTX_free(ctx);
  if (ok != 1) throw std::runtime_error("GCM auth fail");
  total += len;
  out.resize(total);
  return out;
}

static std::vector<uint8_t> concat(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
  std::vector<uint8_t> out; out.reserve(a.size()+b.size()); out.insert(out.end(), a.begin(), a.end()); out.insert(out.end(), b.begin(), b.end()); return out;
}

CryptoEngine::SessionState CryptoEngine::InitOutbound(const std::vector<uint8_t>& our_identity_priv,
                                   const std::vector<uint8_t>& their_identity_pub,
                                   const std::vector<uint8_t>& their_signed_pre_key_pub,
                                   const std::optional<std::vector<uint8_t>>& their_one_time_pre_key_pub) {
  auto eph = X25519Generate();
  // DH1: IKa x SPKb, DH2: EKa x IKb, DH3: EKa x SPKb, DH4: (optional) EKa x OPKb
  auto dh1 = X25519(our_identity_priv, their_signed_pre_key_pub);
  auto dh2 = X25519(eph.priv, their_identity_pub);
  auto dh3 = X25519(eph.priv, their_signed_pre_key_pub);
  std::vector<uint8_t> dhk = concat(concat(dh1, dh2), dh3);
  if (their_one_time_pre_key_pub) {
    auto dh4 = X25519(eph.priv, *their_one_time_pre_key_pub);
    dhk = concat(dhk, dh4);
  }
  auto rk_ck = HKDF(dhk, {}, {'X','3','D','H'}, 64);
  SessionState st{};
  st.root_key.assign(rk_ck.begin(), rk_ck.begin()+32);
  st.chain_key_send.assign(rk_ck.begin()+32, rk_ck.begin()+64);
  st.chain_key_recv = st.chain_key_send; // until first ratchet from peer
  st.our_dh = std::move(eph);
  st.their_dh_pub = their_signed_pre_key_pub;
  st.pn = 0;
  st.n_send = 0;
  st.n_recv = 0;
  return st;
}

CryptoEngine::SessionState CryptoEngine::InitInbound(const std::vector<uint8_t>& our_identity_priv,
                                  const KeyPair& our_signed_pre_key,
                                  const std::optional<KeyPair>& our_one_time_pre_key,
                                  const std::vector<uint8_t>& their_identity_pub,
                                  const std::vector<uint8_t>& their_ephemeral_pub) {
  // Mirror of outbound
  auto dh1 = X25519(our_signed_pre_key.priv, their_identity_pub);
  auto dh2 = X25519(our_identity_priv, their_ephemeral_pub);
  auto dh3 = X25519(our_signed_pre_key.priv, their_ephemeral_pub);
  std::vector<uint8_t> dhk = concat(concat(dh1, dh2), dh3);
  if (our_one_time_pre_key) {
    auto dh4 = X25519(our_one_time_pre_key->priv, their_ephemeral_pub);
    dhk = concat(dhk, dh4);
  }
  auto rk_ck = HKDF(dhk, {}, {'X','3','D','H'}, 64);
  SessionState st{};
  st.root_key.assign(rk_ck.begin(), rk_ck.begin()+32);
  st.chain_key_recv.assign(rk_ck.begin()+32, rk_ck.begin()+64);
  st.chain_key_send = st.chain_key_recv;
  st.our_dh = our_signed_pre_key;
  st.their_dh_pub = their_ephemeral_pub;
  st.pn = 0;
  st.n_send = 0;
  st.n_recv = 0;
  return st;
}

void CryptoEngine::RatchetStep(SessionState& state, const std::vector<uint8_t>& new_their_dh) {
  // RK,CK = KDF(RK, DH(our_dh, new_their_dh))
  auto dh = X25519(state.our_dh.priv, new_their_dh);
  auto rk_ck = HKDF(dh, state.root_key, {'D','R'}, 64);
  state.root_key.assign(rk_ck.begin(), rk_ck.begin()+32);
  state.chain_key_recv.assign(rk_ck.begin()+32, rk_ck.begin()+64);
  // also advance sending chain with new DH
  auto new_pair = X25519Generate();
  auto dh2 = X25519(new_pair.priv, new_their_dh);
  auto rk_ck2 = HKDF(dh2, state.root_key, {'D','R','2'}, 64);
  state.root_key.assign(rk_ck2.begin(), rk_ck2.begin()+32);
  state.chain_key_send.assign(rk_ck2.begin()+32, rk_ck2.begin()+64);
  state.our_dh = std::move(new_pair);
  state.their_dh_pub = new_their_dh;
  state.pn = state.n_recv;
  state.n_send = 0;
  state.n_recv = 0;
}

std::vector<uint8_t> CryptoEngine::DeriveNonce(uint32_t n) {
  std::vector<uint8_t> nonce(12, 0);
  // last 4 bytes = n (big endian)
  nonce[8] = (n >> 24) & 0xFF;
  nonce[9] = (n >> 16) & 0xFF;
  nonce[10] = (n >> 8) & 0xFF;
  nonce[11] = (n) & 0xFF;
  return nonce;
}

static std::vector<uint8_t> KDF_CK(const std::vector<uint8_t>& ck, const char* label) {
  return CryptoEngine::HKDF(ck, {}, std::vector<uint8_t>((const uint8_t*)label, (const uint8_t*)label + strlen(label)), 32);
}

std::vector<uint8_t> CryptoEngine::Encrypt(SessionState& state, const std::vector<uint8_t>& plaintext, Header& out_header) {
  auto mk = KDF_CK(state.chain_key_send, "MK");
  state.chain_key_send = KDF_CK(state.chain_key_send, "CK");
  auto nonce = DeriveNonce(state.n_send);
  std::vector<uint8_t> aad = state.our_dh.pub; // bind to DH
  auto ct = AEAD_Encrypt(mk, nonce, aad, plaintext);
  out_header.dh_pub = state.our_dh.pub;
  out_header.pn = state.pn;
  out_header.n = state.n_send;
  state.n_send++;
  return ct;
}

std::vector<uint8_t> CryptoEngine::Decrypt(SessionState& state, const Header& header, const std::vector<uint8_t>& ciphertext) {
  if (header.dh_pub != state.their_dh_pub) {
    // new ratchet
    RatchetStep(state, header.dh_pub);
  }
  auto mk = KDF_CK(state.chain_key_recv, "MK");
  state.chain_key_recv = KDF_CK(state.chain_key_recv, "CK");
  auto nonce = DeriveNonce(state.n_recv);
  std::vector<uint8_t> aad = header.dh_pub;
  auto pt = AEAD_Decrypt(mk, nonce, aad, ciphertext);
  state.n_recv++;
  return pt;
}

} // namespace messenger
