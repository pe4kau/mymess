#include <gtest/gtest.h>
#include "viewmodels/chatlistmodel.h"
#include "viewmodels/messagelistmodel.h"

TEST(ChatListModel, AddAndPresence){
    ChatListModel m;
    Chat c{ "id1", "Title", "Hi", 0, false, false };
    m.addOrUpdateChat(c);
    EXPECT_EQ(m.rowCount(), 1);
    m.setPresence("id1", true, true);
    QModelIndex idx = m.index(0,0);
    EXPECT_TRUE(idx.data(ChatListModel::OnlineRole).toBool());
    EXPECT_TRUE(idx.data(ChatListModel::TypingRole).toBool());
}

TEST(MessageListModel, AddAndStatus){
    MessageListModel mm;
    Message m; m.localId="loc1"; m.text="hello"; m.outgoing=true;
    mm.addMessage(m);
    EXPECT_EQ(mm.rowCount(), 1);
    mm.updateStatusByLocalId("loc1", Message::Status::Delivered);
    QModelIndex idx = mm.index(0,0);
    EXPECT_EQ(idx.data(MessageListModel::StatusRole).toInt(), (int)Message::Status::Delivered);
}
