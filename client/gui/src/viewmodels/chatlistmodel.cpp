#include "viewmodels/chatlistmodel.h"

ChatListModel::ChatListModel(QObject* parent) : QAbstractListModel(parent) {}

int ChatListModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return m_chats.size();
}

QVariant ChatListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row()<0 || index.row()>=m_chats.size()) return {};
    const Chat& c = m_chats.at(index.row());
    switch(role){
        case IdRole: return c.id;
        case TitleRole: return c.title;
        case LastMessageRole: return c.lastMessage;
        case UnreadRole: return c.unread;
        case OnlineRole: return c.online;
        case TypingRole: return c.typing;
        default: return {};
    }
}

QHash<int,QByteArray> ChatListModel::roleNames() const {
    return {
        {IdRole, "id"},
        {TitleRole, "title"},
        {LastMessageRole, "lastMessage"},
        {UnreadRole, "unread"},
        {OnlineRole, "online"},
        {TypingRole, "typing"}
    };
}

void ChatListModel::addOrUpdateChat(const Chat& c) {
    for (int i=0;i<m_chats.size();++i){
        if (m_chats[i].id == c.id){
            m_chats[i] = c;
            emit dataChanged(index(i), index(i));
            return;
        }
    }
    beginInsertRows(QModelIndex(), m_chats.size(), m_chats.size());
    m_chats.push_back(c);
    endInsertRows();
}

void ChatListModel::setPresence(const QString& chatId, bool online, bool typing){
    for (int i=0;i<m_chats.size();++i){
        if (m_chats[i].id == chatId){
            m_chats[i].online = online;
            m_chats[i].typing = typing;
            emit dataChanged(index(i), index(i), {OnlineRole, TypingRole});
            break;
        }
    }
}
