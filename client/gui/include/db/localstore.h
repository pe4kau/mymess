#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "utils/message.h"

class LocalStore : public QObject {
    Q_OBJECT
public:
    explicit LocalStore(QObject* parent=nullptr);
    bool open(const QString& filePath, const QString& passphrase = QString());
    bool saveMessage(const Message& m);
signals:
    void error(const QString&);
private:
    QSqlDatabase m_db;
    bool initSchema();
};
