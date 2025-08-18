#include "crypto/ratchetwrapper.h"

QByteArray RatchetWrapper::encrypt(const QByteArray& plaintext){
    QByteArray out = plaintext;
    for (int i=0;i<out.size();++i) out[i] = out[i] ^ m_key[i % m_key.size()];
    return out.toBase64();
}
QByteArray RatchetWrapper::decrypt(const QByteArray& ciphertext){
    QByteArray enc = QByteArray::fromBase64(ciphertext);
    for (int i=0;i<enc.size();++i) enc[i] = enc[i] ^ m_key[i % m_key.size()];
    return enc;
}
