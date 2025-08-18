#pragma once
#include <QByteArray>
#include <QString>

// NOTE: This is a thin demo interface. Replace with a real Double Ratchet implementation in production.
class ICryptoSession {
public:
    virtual ~ICryptoSession() = default;
    virtual QByteArray encrypt(const QByteArray& plaintext) = 0;
    virtual QByteArray decrypt(const QByteArray& ciphertext) = 0;
};
