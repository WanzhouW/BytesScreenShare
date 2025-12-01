#include "ui/shared_screen.h"

// #include <QApplication>
// #pragma comment(lib, "user32.lib")

// int main(int argc, char *argv[])
// {
//     QApplication a(argc, argv);
//     shared_screen w;
//     w.show();
//     return a.exec();
// }
#include <QApplication>
#include <QDebug>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    qDebug() << "=== Program Start ===";
    
    QApplication a(argc, argv);
    
    try {
        qDebug() << "Creating main window...";
        shared_screen w;
        qDebug() << "Showing window...";
        w.show();
        qDebug() << "Entering event loop...";
        return a.exec();
    }
    catch (const std::exception& e) {
        qCritical() << "Exception:" << e.what();
        QMessageBox::critical(nullptr, "Error", e.what());
    }
}