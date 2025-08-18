#include "viewmodels/messagelistmodel.h"

MessageListModel::MessageListModel(QObject* parent) : QAbstractListModel(parent) {}

int MessageListModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return m_messages.size();
}

QVariant MessageListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row()<0 || index.row()>=m_messages.size()) return {};
    const Message& m = m_messages.at(index.row());
    switch(role){
        case IdRole: return m.id;
        case LocalIdRole: return m.localId;
        case ChatIdRole: return m.chatId;
        case AuthorRole: return m.author;
        case TextRole: return m.text;
        case ImagePathRole: return m.imagePath;
        case OutgoingRole: return m.outgoing;
        case TimestampRole: return m.timestamp;
        case StatusRole: return static_cast<int>(m.status);
        default: return {};
    }
}

QHash<int,QByteArray> MessageListModel::roleNames() const {
    return {
        {IdRole, "id"},
        {LocalIdRole, "localId"},
        {ChatIdRole, "chatId"},
        {AuthorRole, "author"},
        {TextRole, "text"},
        {ImagePathRole, "imagePath"},
        {OutgoingRole, "outgoing"},
        {TimestampRole, "timestamp"},
        {StatusRole, "status"}
    };
}

void MessageListModel::addMessage(const Message& m) {
    beginInsertRows(QModelIndex(), m_messages.size(), m_messages.size());
    m_messages.push_back(m);
    endInsertRows();
}

void MessageListModel::updateStatusByLocalId(const QString& localId, Message::Status st){
    for (int i=0;i<m_messages.size();++i){
        if (m_messages[i].localId == localId){
            m_messages[i].status = st;
            emit dataChanged(index(i), index(i), {StatusRole});
            break;
        }
    }
}
