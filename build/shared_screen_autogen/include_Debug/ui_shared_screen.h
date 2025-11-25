/********************************************************************************
** Form generated from reading UI file 'shared_screen.ui'
**
** Created by: Qt User Interface Compiler version 6.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SHARED_SCREEN_H
#define UI_SHARED_SCREEN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_shared_screen
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QLabel *screenPreview;
    QSpacerItem *verticalSpacer;
    QWidget *controlsBar;
    QHBoxLayout *horizontalLayout;
    QLabel *statusLabel;
    QSpacerItem *horizontalSpacer;
    QDockWidget *dockChat;
    QWidget *dockWidgetContents;
    QVBoxLayout *verticalLayout_2;
    QTextBrowser *chatView;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *chatInput;
    QPushButton *btnSend;

    void setupUi(QMainWindow *shared_screen)
    {
        if (shared_screen->objectName().isEmpty())
            shared_screen->setObjectName("shared_screen");
        shared_screen->resize(900, 750);
        shared_screen->setStyleSheet(QString::fromUtf8("\n"
"QMainWindow {\n"
"    background-color: #1c1c1e;\n"
"}\n"
"\n"
"QWidget#centralwidget {\n"
"    background-color: #1c1c1e;\n"
"}\n"
"\n"
"/* \351\273\230\350\256\244\346\214\211\351\222\256\346\240\267\345\274\217 - \344\274\232\350\242\253\344\273\243\347\240\201\344\270\255\347\232\204\346\240\267\345\274\217\350\246\206\347\233\226 */\n"
"QPushButton {\n"
"    background-color: #2c2c2e;\n"
"    color: #ffffff;\n"
"    border: none;\n"
"    border-radius: 8px;\n"
"    padding: 10px 20px;\n"
"    font-size: 14px;\n"
"    font-weight: 500;\n"
"    min-width: 40px;\n"
"}\n"
"\n"
"QPushButton:hover {\n"
"    background-color: #20e212ff;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: #ef560fff;\n"
"}\n"
"\n"
"QPushButton:checked {\n"
"    background-color: #0a84ff;\n"
"    color: #ffffff;\n"
"}\n"
"\n"
"QLabel#statusLabel {\n"
"    color: #8e8e93;\n"
"    font-size: 13px;\n"
"    padding-left: 10px;\n"
"}\n"
"\n"
"QLabel#screenPreview {\n"
"    background-color: #2c2c2e;\n"
"    color: #8e8e9"
                        "3;\n"
"    border: 2px dashed #48484a;\n"
"    border-radius: 12px;\n"
"    font-size: 16px;\n"
"}\n"
"\n"
"QDockWidget {\n"
"    background-color: #2c2c2e;\n"
"    color: #ffffff;\n"
"    titlebar-close-icon: url(close.png);\n"
"    titlebar-normal-icon: url(float.png);\n"
"}\n"
"\n"
"QDockWidget::title {\n"
"    background-color: #2c2c2e;\n"
"    color: #ffffff;\n"
"    padding: 10px;\n"
"    font-weight: bold;\n"
"    font-size: 14px;\n"
"}\n"
"\n"
"QTextBrowser {\n"
"    background-color: #1c1c1e;\n"
"    color: #ffffff;\n"
"    border: none;\n"
"    border-radius: 8px;\n"
"    padding: 10px;\n"
"    font-size: 13px;\n"
"}\n"
"\n"
"QLineEdit {\n"
"    background-color: #2c2c2e;\n"
"    color: #ffffff;\n"
"    border: 1px solid #48484a;\n"
"    border-radius: 6px;\n"
"    padding: 8px 12px;\n"
"    font-size: 13px;\n"
"}\n"
"\n"
"QLineEdit:focus {\n"
"    border: 1px solid #0a84ff;\n"
"}\n"
"\n"
"QPushButton#btnSend {\n"
"    background-color: #0a84ff;\n"
"    min-width: 70px;\n"
"}\n"
"\n"
"QPushButton#btn"
                        "Send:hover {\n"
"    background-color: #0077ed;\n"
"}\n"
"\n"
"QPushButton#btnSend:pressed {\n"
"    background-color: #006edb;\n"
"}\n"
"   "));
        centralwidget = new QWidget(shared_screen);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(20, 20, 4, 4);
        screenPreview = new QLabel(centralwidget);
        screenPreview->setObjectName("screenPreview");
        screenPreview->setAlignment(Qt::AlignCenter);
        screenPreview->setMinimumSize(QSize(800, 450));

        verticalLayout->addWidget(screenPreview);

        verticalSpacer = new QSpacerItem(20, 20, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        controlsBar = new QWidget(centralwidget);
        controlsBar->setObjectName("controlsBar");
        controlsBar->setMinimumHeight(5);
        horizontalLayout = new QHBoxLayout(controlsBar);
        horizontalLayout->setSpacing(5);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        statusLabel = new QLabel(controlsBar);
        statusLabel->setObjectName("statusLabel");

        horizontalLayout->addWidget(statusLabel);

        horizontalSpacer = new QSpacerItem(5, 5, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout->addWidget(controlsBar);

        shared_screen->setCentralWidget(centralwidget);
        dockChat = new QDockWidget(shared_screen);
        dockChat->setObjectName("dockChat");
        dockChat->setMinimumWidth(300);
        dockChat->setMaximumWidth(400);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName("dockWidgetContents");
        verticalLayout_2 = new QVBoxLayout(dockWidgetContents);
        verticalLayout_2->setSpacing(10);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setContentsMargins(10, 10, 10, 10);
        chatView = new QTextBrowser(dockWidgetContents);
        chatView->setObjectName("chatView");
        chatView->setOpenExternalLinks(false);

        verticalLayout_2->addWidget(chatView);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(10);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        chatInput = new QLineEdit(dockWidgetContents);
        chatInput->setObjectName("chatInput");

        horizontalLayout_2->addWidget(chatInput);

        btnSend = new QPushButton(dockWidgetContents);
        btnSend->setObjectName("btnSend");

        horizontalLayout_2->addWidget(btnSend);


        verticalLayout_2->addLayout(horizontalLayout_2);

        dockChat->setWidget(dockWidgetContents);
        shared_screen->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockChat);

        retranslateUi(shared_screen);

        QMetaObject::connectSlotsByName(shared_screen);
    } // setupUi

    void retranslateUi(QMainWindow *shared_screen)
    {
        shared_screen->setWindowTitle(QCoreApplication::translate("shared_screen", "\345\261\217\345\271\225\345\205\261\344\272\253\344\274\232\350\256\256", nullptr));
        screenPreview->setText(QCoreApplication::translate("shared_screen", "\345\261\217\345\271\225\351\242\204\350\247\210\345\214\272\345\237\237\n"
"\347\202\271\345\207\273\"\345\205\261\344\272\253\345\261\217\345\271\225\"\345\274\200\345\247\213", nullptr));
        statusLabel->setText(QCoreApplication::translate("shared_screen", "\347\212\266\346\200\201\357\274\232\346\234\252\350\277\236\346\216\245", nullptr));
        dockChat->setWindowTitle(QCoreApplication::translate("shared_screen", "\360\237\222\254", nullptr));
        chatInput->setPlaceholderText(QCoreApplication::translate("shared_screen", "\350\276\223\345\205\245\346\266\210\346\201\257...", nullptr));
        btnSend->setText(QCoreApplication::translate("shared_screen", "\345\217\221\351\200\201", nullptr));
    } // retranslateUi

};

namespace Ui {
    class shared_screen: public Ui_shared_screen {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SHARED_SCREEN_H
