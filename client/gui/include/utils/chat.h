#pragma once
#include <QString>

struct Chat {
    QString id;
    QString title;
    QString lastMessage;
    int unread = 0;
    bool online = false;
    bool typing = false;
};
