#pragma once
#include <QMainWindow>
#include <QListView>
#include <QTreeView>
#include <QSplitter>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QToolBar>
#include <QStandardItemModel>
#include "viewmodels/chatlistmodel.h"
#include "viewmodels/messagelistmodel.h"
#include "inetworkclient.h"
#include "thememanager.h"
#include "ui/lightboxdialog.h"

class ChatWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit ChatWindow(QWidget* parent=nullptr);
    void setNetworkClient(INetworkClient* client);

private slots:
    void onSendClicked();
    void onFileClicked();
    void onChatSelected(const QModelIndex &index);
    void onTypingChanged();
    void onNetworkMessageReceived(const QString& chatId, const Message& msg);
    void onNetworkStatusChanged(const QString& chatId, const QString& localId, Message::Status st);
    void onPresenceChanged(const QString& chatId, bool online, bool typing);

private:
    void setupUi();
    void setupMenu();

    QListView* m_chatListView = nullptr;
    QListView* m_messageListView = nullptr;
    QLineEdit* m_input = nullptr;
    QPushButton* m_sendBtn = nullptr;
    QPushButton* m_fileBtn = nullptr;
    QLabel* m_headerTitle = nullptr;
    QLabel* m_headerStatus = nullptr;

    ChatListModel* m_chatModel = nullptr;
    MessageListModel* m_msgModel = nullptr;
    ThemeManager m_theme;

    QString m_currentChatId;
    INetworkClient* m_net = nullptr;
    QTimer* m_typingTimer = nullptr;
};
