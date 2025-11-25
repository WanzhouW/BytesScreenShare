#include "shared_screen.h"

#include <QApplication>
#pragma comment(lib, "user32.lib")

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    shared_screen w;
    w.show();
    return a.exec();
}