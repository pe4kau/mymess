#include <QApplication>
#include <QFile>
#include "ui/chatwindow.h"
#include "thememanager.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ThemeManager theme;
    // default to light
    theme.applyTheme(ThemeManager::Theme::Light);

    ChatWindow w;
    w.show();

    return app.exec();
}
