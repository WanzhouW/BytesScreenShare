#include "Widget.h"
#include "SignalingServer.h"
#include <QPushButton>
#include <QVBoxLayout>

Widget::Widget(QWidget* parent)
    : QWidget(parent), serverRunning(false), server(nullptr)
{
    ui.setupUi(this);

    QPushButton* toggleServerButton = new QPushButton("Start Server", this);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(toggleServerButton);
    setLayout(layout);

    connect(toggleServerButton, &QPushButton::clicked, this, [this, toggleServerButton]() {
        if (serverRunning) {
            stopServer();
            toggleServerButton->setText("Start Server");
        }
        else {
            startServer();
            toggleServerButton->setText("Stop Server");
        }
        });
}

Widget::~Widget()
{
    stopServer();
    if (server) {
        delete server;
        server = nullptr;
    }
}

void Widget::startServer()
{
    if (!server) {
        server = new SignalingServer(QHostAddress::Any, 11290, 2, this);
        server->start(QHostAddress::Any, 11290);
    }
    serverRunning = true;
}

void Widget::stopServer()
{
    if (server) {
        server->stop();
        delete server;
        server = nullptr;
    }
    serverRunning = false;
}
