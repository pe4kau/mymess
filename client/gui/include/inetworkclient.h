#pragma once
#include <QObject>
#include <QString>
#include <QByteArray>
#include "utils/message.h"

class INetworkClient : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    virtual ~INetworkClient() = default;

    // Sends a text message. Returns a local temp id to correlate status updates.
    virtual QString sendMessage(const QString& chatId, const QString& text) = 0;
    // Sends a file (image or any binary).
    virtual QString sendFile(const QString& chatId, const QString& filePath) = 0;
    // Request history page from backend
    virtual void requestHistory(const QString& chatId, int limit, const QString& beforeMessageId = QString()) = 0;
    // Typing indicator
    virtual void setTyping(const QString& chatId, bool typing) = 0;

signals:
    void messageReceived(const QString& chatId, const Message& msg);
    void messageStatusChanged(const QString& chatId, const QString& localId, Message::Status status);
    void chatPresenceChanged(const QString& chatId, bool online, bool typing);
};
