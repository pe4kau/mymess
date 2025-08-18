#pragma once
#include <QAbstractListModel>
#include "utils/message.h"

class MessageListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole+1,
        LocalIdRole,
        ChatIdRole,
        AuthorRole,
        TextRole,
        ImagePathRole,
        OutgoingRole,
        TimestampRole,
        StatusRole
    };
    Q_ENUM(Roles)

    explicit MessageListModel(QObject* parent=nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addMessage(const Message& m);
    void updateStatusByLocalId(const QString& localId, Message::Status st);

private:
    QList<Message> m_messages;
};
