#include "db/localstore.h"
#include <QVariant>

LocalStore::LocalStore(QObject* parent) : QObject(parent) {}

bool LocalStore::open(const QString& filePath, const QString& passphrase){
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(filePath);
    if (!m_db.open()){
        emit error(m_db.lastError().text());
        return false;
    }
    // Attempt SQLCipher compatibility (ignored by vanilla SQLite)
    if (!passphrase.isEmpty()){
        QSqlQuery pragma(m_db);
        pragma.exec(QString("PRAGMA key='%1';").arg(passphrase.replace("'","''")));
    }
    return initSchema();
}

bool LocalStore::initSchema(){
    QSqlQuery q(m_db);
    if (!q.exec("CREATE TABLE IF NOT EXISTS messages ("
                "id TEXT, localId TEXT, chatId TEXT, author TEXT, text TEXT, imagePath TEXT, "
                "outgoing INTEGER, timestamp TEXT, status INTEGER)")) {
        emit error(q.lastError().text());
        return false;
    }
    return true;
}

bool LocalStore::saveMessage(const Message& m){
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO messages(id,localId,chatId,author,text,imagePath,outgoing,timestamp,status) "
              "VALUES(:id,:localId,:chatId,:author,:text,:imagePath,:outgoing,:timestamp,:status)");
    q.bindValue(":id", m.id);
    q.bindValue(":localId", m.localId);
    q.bindValue(":chatId", m.chatId);
    q.bindValue(":author", m.author);
    q.bindValue(":text", m.text);
    q.bindValue(":imagePath", m.imagePath);
    q.bindValue(":outgoing", m.outgoing);
    q.bindValue(":timestamp", m.timestamp.toString(Qt::ISODate));
    q.bindValue(":status", (int)m.status);
    if (!q.exec()){
        emit error(q.lastError().text());
        return false;
    }
    return true;
}
