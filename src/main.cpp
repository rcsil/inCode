#include <QApplication>
#include "MainWindow.h"

void setDarkTheme(QApplication &app);

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    setDarkTheme(app);
    MainWindow window;
    window.show();
    return app.exec();
}
