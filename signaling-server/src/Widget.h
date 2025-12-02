#pragma once

#include <QtWidgets/QWidget>
#include "ui_Widget.h"
#include "SignalingServer.h"

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    void startServer();
    void stopServer();

private:
    bool serverRunning;
    SignalingServer* server;
    Ui::WidgetClass ui;
};

