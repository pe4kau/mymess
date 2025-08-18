#pragma once
#include <QString>
#include <QDateTime>
#include <QMetaType>

struct Message {
    enum class Status { Pending, Sent, Delivered, Read };
    QString id;          // server id (if known)
    QString localId;     // local temp id
    QString chatId;
    QString author;      // display name or id
    QString text;
    QString imagePath;   // if non-empty, message is an image
    bool outgoing = false;
    QDateTime timestamp = QDateTime::currentDateTime();
    Status status = Status::Pending;
};
Q_DECLARE_METATYPE(Message)
