#pragma once
#include <QAbstractListModel>
#include "utils/chat.h"

class ChatListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles { IdRole = Qt::UserRole+1, TitleRole, LastMessageRole, UnreadRole, OnlineRole, TypingRole };
    Q_ENUM(Roles)

    explicit ChatListModel(QObject* parent=nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addOrUpdateChat(const Chat& c);
    void setPresence(const QString& chatId, bool online, bool typing);
private:
    QList<Chat> m_chats;
};
