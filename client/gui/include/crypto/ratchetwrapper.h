#pragma once
#include "crypto/icryptosession.h"
#include <QCryptographicHash>

class RatchetWrapper : public ICryptoSession {
public:
    explicit RatchetWrapper(const QByteArray& key) : m_key(QCryptographicHash::hash(key, QCryptographicHash::Sha256)) {}
    QByteArray encrypt(const QByteArray& plaintext) override;
    QByteArray decrypt(const QByteArray& ciphertext) override;
private:
    QByteArray m_key;
};
