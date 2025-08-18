#include <gtest/gtest.h>
#include "messenger/CryptoEngine.hpp"
#include <vector>
#include <string>

using namespace messenger;

TEST(CryptoEngine, HandshakeEncryptDecrypt) {
  auto alice_id = CryptoEngine::X25519Generate();
  auto bob_id = CryptoEngine::X25519Generate();
  auto bob_spk = CryptoEngine::X25519Generate();
  auto bob_opk = CryptoEngine::X25519Generate();

  // Outbound (Alice) builds session from Bob's bundle
  auto alice = CryptoEngine::InitOutbound(alice_id.priv, bob_id.pub, bob_spk.pub, bob_opk.pub);
  // Inbound (Bob) builds session from Alice IK + Alice eph (we simulate eph via alice.our_dh.pub)
  auto bob = CryptoEngine::InitInbound(bob_id.priv, bob_spk, bob_opk, alice_id.pub, alice.our_dh.pub);

  // Alice sends
  CryptoEngine::Header h1{};
  auto ct1 = CryptoEngine::Encrypt(alice, std::vector<uint8_t>{'h','i'}, h1);
  // Bob decrypts
  auto pt1 = CryptoEngine::Decrypt(bob, h1, ct1);
  ASSERT_EQ(std::string(pt1.begin(), pt1.end()), "hi");

  // Bob replies (new ratchet header triggers on Alice side)
  CryptoEngine::Header h2{};
  auto ct2 = CryptoEngine::Encrypt(bob, std::vector<uint8_t>{'o','k'}, h2);
  auto pt2 = CryptoEngine::Decrypt(alice, h2, ct2);
  ASSERT_EQ(std::string(pt2.begin(), pt2.end()), "ok");
}
