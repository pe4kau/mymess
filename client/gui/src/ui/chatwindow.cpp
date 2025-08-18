#include "ui/chatwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDateTime>
#include <QTimer>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QScrollBar>
#include <QMenuBar>

namespace {
class MessageDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& idx) const override {
        p->save();
        const bool outgoing = idx.data(MessageListModel::OutgoingRole).toBool();
        const QString text = idx.data(MessageListModel::TextRole).toString();
        const QString img = idx.data(MessageListModel::ImagePathRole).toString();
        const int status = idx.data(MessageListModel::StatusRole).toInt();
        QRect r = opt.rect.adjusted(10, 6, -10, -6);
        int bubbleW = r.width()*0.75;
        QRect bubble;
        if (outgoing) bubble = QRect(r.right()-bubbleW, r.top(), bubbleW, r.height());
        else bubble = QRect(r.left(), r.top(), bubbleW, r.height());

        // bubble
        QColor bg = outgoing ? QColor(0,120,215,40) : QColor(128,128,128,40);
        p->setBrush(bg);
        p->setPen(Qt::NoPen);
        p->drawRoundedRect(bubble, 10,10);

        p->setPen(opt.palette.windowText().color());
        int y = bubble.top()+10;
        if (!img.isEmpty()){
            QPixmap pm(img);
            QSize size = pm.size().scaled(300,300, Qt::KeepAspectRatio);
            QRect pr(bubble.left()+10, y, size.width(), size.height());
            p->drawPixmap(pr, pm.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            y = pr.bottom()+8;
        }
        p->drawText(QRect(bubble.left()+10, y, bubble.width()-60, bubble.height()-20), Qt::TextWordWrap, text);

        // status checkmarks (simple text icons for portability)
        QString s = "";
        if (outgoing){
            if (status >= (int)Message::Status::Sent) s = u"✓"_qs;
            if (status >= (int)Message::Status::Delivered) s = u"✓✓"_qs;
            if (status >= (int)Message::Status::Read) s = u"✓✓"_qs; // same, but could colorize in real app
        }
        p->drawText(QRect(bubble.right()-40, bubble.bottom()-20, 30, 16), Qt::AlignRight|Qt::AlignVCenter, s);

        p->restore();
    }
    QSize sizeHint(const QStyleOptionViewItem& opt, const QModelIndex& idx) const override {
        QString text = idx.data(MessageListModel::TextRole).toString();
        QString img = idx.data(MessageListModel::ImagePathRole).toString();
        int h = 40;
        if (!text.isEmpty()) h += (text.size()/30)*14;
        if (!img.isEmpty()) h += 200;
        return QSize(opt.rect.width(), h);
    }
};
}

ChatWindow::ChatWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi();
    setupMenu();
}

void ChatWindow::setupMenu(){
    auto themeMenu = menuBar()->addMenu(tr("Theme"));
    auto actLight = themeMenu->addAction(tr("Light"));
    auto actDark = themeMenu->addAction(tr("Dark"));
    connect(actLight, &QAction::triggered, this, [this]{ m_theme.applyTheme(ThemeManager::Theme::Light); });
    connect(actDark, &QAction::triggered, this, [this]{ m_theme.applyTheme(ThemeManager::Theme::Dark); });
}

void ChatWindow::setupUi(){
    auto central = new QWidget(this);
    auto mainLay = new QVBoxLayout(central);
    auto header = new QWidget(central);
    auto hl = new QHBoxLayout(header);
    m_headerTitle = new QLabel(tr("Select a chat"), header);
    m_headerTitle->setObjectName("chatTitle");
    m_headerStatus = new QLabel(tr(""), header);
    m_headerStatus->setObjectName("chatStatus");
    hl->addWidget(m_headerTitle);
    hl->addStretch();
    hl->addWidget(m_headerStatus);
    mainLay->addWidget(header);

    auto splitter = new QSplitter(Qt::Horizontal, central);
    m_chatListView = new QListView(splitter);
    m_messageListView = new QListView(splitter);
    m_messageListView->setItemDelegate(new MessageDelegate(m_messageListView));
    splitter->addWidget(m_chatListView);
    splitter->addWidget(m_messageListView);
    splitter->setStretchFactor(0,1);
    splitter->setStretchFactor(1,3);
    mainLay->addWidget(splitter);

    auto inputBar = new QWidget(central);
    auto il = new QHBoxLayout(inputBar);
    m_input = new QLineEdit(inputBar);
    m_fileBtn = new QPushButton(tr("Файл"), inputBar);
    m_sendBtn = new QPushButton(tr("Отправить"), inputBar);
    il->addWidget(m_input, 1);
    il->addWidget(m_fileBtn);
    il->addWidget(m_sendBtn);
    mainLay->addWidget(inputBar);

    setCentralWidget(central);

    // models
    m_chatModel = new ChatListModel(this);
    m_msgModel = new MessageListModel(this);
    m_chatListView->setModel(m_chatModel);
    m_messageListView->setModel(m_msgModel);

    // demo data
    Chat c1{ "chat_1", "Alice", "Привет!", 0, true, false };
    Chat c2{ "chat_2", "Bob", "До встречи", 2, false, false };
    m_chatModel->addOrUpdateChat(c1);
    m_chatModel->addOrUpdateChat(c2);

    connect(m_sendBtn, &QPushButton::clicked, this, &ChatWindow::onSendClicked);
    connect(m_fileBtn, &QPushButton::clicked, this, &ChatWindow::onFileClicked);
    connect(m_chatListView, &QListView::clicked, this, &ChatWindow::onChatSelected);
    connect(m_input, &QLineEdit::textEdited, this, &ChatWindow::onTypingChanged);

    m_typingTimer = new QTimer(this);
    m_typingTimer->setSingleShot(true);
    m_typingTimer->setInterval(1500);
    connect(m_typingTimer, &QTimer::timeout, this, &ChatWindow::onTypingChanged);
}

void ChatWindow::setNetworkClient(INetworkClient* client){
    if (m_net) m_net->disconnect(this);
    m_net = client;
    if (!m_net) return;
    connect(m_net, &INetworkClient::messageReceived, this, &ChatWindow::onNetworkMessageReceived);
    connect(m_net, &INetworkClient::messageStatusChanged, this, &ChatWindow::onNetworkStatusChanged);
    connect(m_net, &INetworkClient::chatPresenceChanged, this, &ChatWindow::onPresenceChanged);
}

void ChatWindow::onSendClicked(){
    if (m_currentChatId.isEmpty()) return;
    const QString text = m_input->text().trimmed();
    if (text.isEmpty()) return;
    QString localId = m_net ? m_net->sendMessage(m_currentChatId, text) : QString::number(QDateTime::currentMSecsSinceEpoch());
    Message m;
    m.chatId = m_currentChatId;
    m.text = text;
    m.outgoing = true;
    m.localId = localId;
    m.status = Message::Status::Sent;
    m_msgModel->addMessage(m);
    m_input->clear();
}

void ChatWindow::onFileClicked(){
    if (m_currentChatId.isEmpty()) return;
    QString path = QFileDialog::getOpenFileName(this, tr("Choose file"));
    if (path.isEmpty()) return;
    QString localId = m_net ? m_net->sendFile(m_currentChatId, path) : QString::number(QDateTime::currentMSecsSinceEpoch());
    Message m;
    m.chatId = m_currentChatId;
    m.imagePath = path;
    m.outgoing = true;
    m.localId = localId;
    m.status = Message::Status::Sent;
    m_msgModel->addMessage(m);
}

void ChatWindow::onChatSelected(const QModelIndex& index){
    if (!index.isValid()) return;
    m_currentChatId = index.data(ChatListModel::IdRole).toString();
    const QString title = index.data(ChatListModel::TitleRole).toString();
    bool online = index.data(ChatListModel::OnlineRole).toBool();
    bool typing = index.data(ChatListModel::TypingRole).toBool();
    m_headerTitle->setText(title);
    m_headerStatus->setText(typing ? tr("печатает…") : (online ? tr("онлайн") : tr("оффлайн")));
    // requestHistory(currentChatId, /*limit*/ 50); // initiated on open
}

void ChatWindow::onTypingChanged(){
    if (!m_net || m_currentChatId.isEmpty()) return;
    static bool state = false;
    state = !state; // send toggles to simulate start/stop
    m_net->setTyping(m_currentChatId, state);
    m_typingTimer->start();
}

void ChatWindow::onNetworkMessageReceived(const QString& chatId, const Message& msg){
    if (chatId != m_currentChatId) return;
    m_msgModel->addMessage(msg);
}

void ChatWindow::onNetworkStatusChanged(const QString& chatId, const QString& localId, Message::Status st){
    if (chatId != m_currentChatId) return;
    m_msgModel->updateStatusByLocalId(localId, st);
}

void ChatWindow::onPresenceChanged(const QString& chatId, bool online, bool typing){
    if (chatId != m_currentChatId) return;
    m_headerStatus->setText(typing ? tr("печатает…") : (online ? tr("онлайн") : tr("оффлайн")));
}
