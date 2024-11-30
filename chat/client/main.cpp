#include <QApplication>
#include <QFile>

#include "chat/client/widget/mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    if (QFile qss("chat/client/style/stylesheet.qss"); qss.open(QFile::ReadOnly)) {
        qDebug("Open success");
        QString style = QLatin1String(qss.readAll());
        a.setStyleSheet(style);
        qss.close();
    } else {
        qDebug("Open failed");
    }

    MainWindow w;
    w.show();
    return a.exec();
}
