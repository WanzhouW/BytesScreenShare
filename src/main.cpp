#include "shared_screen.h"
#include "src/Capture/ScreenCaptureService.h"

#include <QApplication>
#pragma comment(lib, "user32.lib")

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	ScreenCaptureService screenService;

    auto widget = screenService.getVideoPreviewWidget();
    widget->resize(800, 600);
    widget->show();

    screenService.startCapture();

    // shared_screen w;
    // w.show();
    return a.exec();
}