#pragma once
#include <QObject>
#include <QString>
#include <QFile>
#include <QApplication>

class ThemeManager : public QObject {
    Q_OBJECT
public:
    enum class Theme { Light, Dark };
    Q_ENUM(Theme)

    explicit ThemeManager(QObject* parent=nullptr) : QObject(parent) {}

    bool applyTheme(Theme t) {
        const char* path = (t==Theme::Dark) ? ":/qss/dark.qss" : ":/qss/light.qss";
        QFile f(path);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
            return true;
        }
        return false;
    }
};
