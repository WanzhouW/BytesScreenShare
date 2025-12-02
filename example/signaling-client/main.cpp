#include "TestClient.hpp"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TestClient window;
    window.show();
    return app.exec();
}
