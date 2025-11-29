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
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_shared_screen
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QStackedWidget *stackedWidget;
    QWidget *pageHome;
    QVBoxLayout *verticalLayout_3;
    QSpacerItem *verticalSpacer_2;
    QLabel *welcomeLabel;
    QHBoxLayout *horizontalLayout_room;
    QSpacerItem *horizontalSpacer_room_left;
    QLineEdit *editRoomId;
    QSpacerItem *horizontalSpacer_room_right;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *btnJoinMeeting;
    QSpacerItem *horizontalSpacer_3;
    QSpacerItem *verticalSpacer_3;
    QWidget *pageMeeting;
    QVBoxLayout *verticalLayout_4;
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
    QHBoxLayout *horizontalLayout_21;
    QLineEdit *chatInput;
    QPushButton *btnSend;

    void setupUi(QMainWindow *shared_screen)
    {
        if (shared_screen->objectName().isEmpty())
            shared_screen->setObjectName("shared_screen");
        shared_screen->setStyleSheet(QString::fromUtf8("\n"
"/* ========== \344\270\273\347\252\227\345\217\243 ========== */\n"
"QMainWindow {\n"
"    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,\n"
"                                stop:0 #f5f5f7, stop:1 #e8e8ed);\n"
"}\n"
"\n"
"QWidget#centralwidget {\n"
"    background: transparent;\n"
"}\n"
"\n"
"/* ========== \347\212\266\346\200\201\346\240\207\347\255\276 ========== */\n"
"QLabel#statusLabel {\n"
"    color: #6e6e73;\n"
"    font-size: 13px;\n"
"    font-weight: 500;\n"
"    padding: 8px 12px;\n"
"    background: rgba(255, 255, 255, 0.6);\n"
"    border-radius: 6px;\n"
"}\n"
"\n"
"/* ========== \345\261\217\345\271\225\351\242\204\350\247\210\345\214\272\345\237\237 ========== */\n"
"QLabel#screenPreview {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,\n"
"                                stop:0 #2c2c2e, stop:1 #1c1c1e);\n"
"    color: #8e8e93;\n"
"    border: 2px solid #48484a;\n"
"    border-radius: 16px;\n"
"    font-size: 18px;\n"
"    font-weight: 500;\n"
"    padding: 20px;\n"
"}\n"
""
                        "\n"
"/* ========== \346\216\247\345\210\266\346\240\217\345\256\271\345\231\250 ========== */\n"
"QWidget#controlsBar {\n"
"    background: rgba(255, 255, 255, 0.95);\n"
"    border-radius: 12px;\n"
"    padding: 8px;\n"
"}\n"
"\n"
"/* ========== \350\201\212\345\244\251\351\235\242\346\235\277 ========== */\n"
"QDockWidget {\n"
"    background: #ffffff;\n"
"    color: #1d1d1f;\n"
"    border: none;\n"
"}\n"
"\n"
"QDockWidget::title {\n"
"    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,\n"
"                                stop:0 #fafafa, stop:1 #f0f0f0);\n"
"    color: #1d1d1f;\n"
"    padding: 12px;\n"
"    font-weight: 600;\n"
"    font-size: 14px;\n"
"    border-bottom: 1px solid #e0e0e0;\n"
"    border-top-left-radius: 8px;\n"
"    border-top-right-radius: 8px;\n"
"}\n"
"\n"
"/* ========== \350\201\212\345\244\251\350\247\206\345\233\276 ========== */\n"
"QTextBrowser {\n"
"    background: #ffffff;\n"
"    color: #1d1d1f;\n"
"    border: 1px solid #e0e0e0;\n"
"    border-radius: 8px;\n"
"    padding"
                        ": 12px;\n"
"    font-size: 13px;\n"
"    selection-background-color: #0a84ff;\n"
"}\n"
"\n"
"QTextBrowser:focus {\n"
"    border: 1px solid #0a84ff;\n"
"}\n"
"\n"
"/* ========== \350\276\223\345\205\245\346\241\206 ========== */\n"
"QLineEdit {\n"
"    background: #f5f5f7;\n"
"    color: #1d1d1f;\n"
"    border: 1px solid #d1d1d6;\n"
"    border-radius: 8px;\n"
"    padding: 10px 14px;\n"
"    font-size: 13px;\n"
"}\n"
"\n"
"QLineEdit:focus {\n"
"    background: #ffffff;\n"
"    border: 2px solid #0a84ff;\n"
"    padding: 9px 13px;\n"
"}\n"
"\n"
"QLineEdit::placeholder {\n"
"    color: #86868b;\n"
"}\n"
"\n"
"/* ========== \345\217\221\351\200\201\346\214\211\351\222\256 ========== */\n"
"QPushButton#btnSend {\n"
"    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,\n"
"                                stop:0 #0a84ff, stop:1 #0077ed);\n"
"    color: #ffffff;\n"
"    border: none;\n"
"    border-radius: 8px;\n"
"    padding: 10px 20px;\n"
"    font-size: 13px;\n"
"    font-weight: 600;\n"
"    min-width: 70px"
                        ";\n"
"}\n"
"\n"
"QPushButton#btnSend:hover {\n"
"    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,\n"
"                                stop:0 #0077ed, stop:1 #006edb);\n"
"}\n"
"\n"
"QPushButton#btnSend:pressed {\n"
"    background: #006edb;\n"
"}\n"
"   "));
        centralwidget = new QWidget(shared_screen);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        stackedWidget = new QStackedWidget(centralwidget);
        stackedWidget->setObjectName("stackedWidget");
        pageHome = new QWidget();
        pageHome->setObjectName("pageHome");
        verticalLayout_3 = new QVBoxLayout(pageHome);
        verticalLayout_3->setSpacing(20);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setContentsMargins(40, 40, 40, 40);
        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_2);

        welcomeLabel = new QLabel(pageHome);
        welcomeLabel->setObjectName("welcomeLabel");
        welcomeLabel->setAlignment(Qt::AlignCenter);
        welcomeLabel->setStyleSheet(QString::fromUtf8("font-size: 24px; font-weight: bold; color: #1d1d1f;"));

        verticalLayout_3->addWidget(welcomeLabel);

        horizontalLayout_room = new QHBoxLayout();
        horizontalLayout_room->setObjectName("horizontalLayout_room");
        horizontalSpacer_room_left = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_room->addItem(horizontalSpacer_room_left);

        editRoomId = new QLineEdit(pageHome);
        editRoomId->setObjectName("editRoomId");
        editRoomId->setMinimumWidth(200);
        editRoomId->setMaximumWidth(300);
        editRoomId->setStyleSheet(QString::fromUtf8("\n"
"                    QLineEdit {\n"
"                        background: #ffffff;\n"
"                        color: #1d1d1f;\n"
"                        border: 1px solid #d1d1d6;\n"
"                        border-radius: 8px;\n"
"                        padding: 12px 14px;\n"
"                        font-size: 14px;\n"
"                    }\n"
"                    QLineEdit:focus {\n"
"                        border: 2px solid #0a84ff;\n"
"                        padding: 11px 13px;\n"
"                    }\n"
"                    "));

        horizontalLayout_room->addWidget(editRoomId);

        horizontalSpacer_room_right = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_room->addItem(horizontalSpacer_room_right);


        verticalLayout_3->addLayout(horizontalLayout_room);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);

        btnJoinMeeting = new QPushButton(pageHome);
        btnJoinMeeting->setObjectName("btnJoinMeeting");
        btnJoinMeeting->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,\n"
"                                stop:0 #0a84ff, stop:1 #0077ed);\n"
"    color: white;\n"
"    border: none;\n"
"    border-radius: 12px;\n"
"    padding: 16px 20px;\n"
"    font-size: 16px;\n"
"    font-weight: 600;\n"
"    width: 50px;\n"
"}\n"
"QPushButton:hover {\n"
"    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,\n"
"                                stop:0 #0077ed, stop:1 #006edb);\n"
"}\n"
"QPushButton:pressed {\n"
"    background: #006edb;\n"
"}"));
        btnJoinMeeting->setMinimumWidth(180);
        btnJoinMeeting->setMaximumWidth(80);

        horizontalLayout_2->addWidget(btnJoinMeeting);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_3);


        verticalLayout_3->addLayout(horizontalLayout_2);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_3->addItem(verticalSpacer_3);

        stackedWidget->addWidget(pageHome);
        pageMeeting = new QWidget();
        pageMeeting->setObjectName("pageMeeting");
        verticalLayout_4 = new QVBoxLayout(pageMeeting);
        verticalLayout_4->setSpacing(16);
        verticalLayout_4->setObjectName("verticalLayout_4");
        verticalLayout_4->setContentsMargins(24, 24, 24, 24);
        screenPreview = new QLabel(pageMeeting);
        screenPreview->setObjectName("screenPreview");
        screenPreview->setAlignment(Qt::AlignCenter);
        screenPreview->setMinimumSize(QSize(800, 450));

        verticalLayout_4->addWidget(screenPreview);

        verticalSpacer = new QSpacerItem(20, 10, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_4->addItem(verticalSpacer);

        controlsBar = new QWidget(pageMeeting);
        controlsBar->setObjectName("controlsBar");
        controlsBar->setMinimumHeight(80);
        horizontalLayout = new QHBoxLayout(controlsBar);
        horizontalLayout->setSpacing(12);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(16, 0, 16, 0);
        statusLabel = new QLabel(controlsBar);
        statusLabel->setObjectName("statusLabel");

        horizontalLayout->addWidget(statusLabel);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout_4->addWidget(controlsBar);

        stackedWidget->addWidget(pageMeeting);

        verticalLayout->addWidget(stackedWidget);

        shared_screen->setCentralWidget(centralwidget);
        dockChat = new QDockWidget(shared_screen);
        dockChat->setObjectName("dockChat");
        dockChat->setMinimumWidth(320);
        dockChat->setMaximumWidth(420);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName("dockWidgetContents");
        verticalLayout_2 = new QVBoxLayout(dockWidgetContents);
        verticalLayout_2->setSpacing(12);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setContentsMargins(16, 16, 16, 16);
        chatView = new QTextBrowser(dockWidgetContents);
        chatView->setObjectName("chatView");
        chatView->setOpenExternalLinks(false);

        verticalLayout_2->addWidget(chatView);

        horizontalLayout_21 = new QHBoxLayout();
        horizontalLayout_21->setSpacing(10);
        horizontalLayout_21->setObjectName("horizontalLayout_21");
        chatInput = new QLineEdit(dockWidgetContents);
        chatInput->setObjectName("chatInput");

        horizontalLayout_21->addWidget(chatInput);

        btnSend = new QPushButton(dockWidgetContents);
        btnSend->setObjectName("btnSend");

        horizontalLayout_21->addWidget(btnSend);


        verticalLayout_2->addLayout(horizontalLayout_21);

        dockChat->setWidget(dockWidgetContents);
        shared_screen->addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dockChat);

        retranslateUi(shared_screen);

        stackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(shared_screen);
    } // setupUi

    void retranslateUi(QMainWindow *shared_screen)
    {
        shared_screen->setWindowTitle(QCoreApplication::translate("shared_screen", "\345\261\217\345\271\225\345\205\261\344\272\253\344\274\232\350\256\256", nullptr));
        welcomeLabel->setText(QCoreApplication::translate("shared_screen", "\346\254\242\350\277\216\344\275\277\347\224\250\345\261\217\345\271\225\345\205\261\344\272\253\344\274\232\350\256\256", nullptr));
        editRoomId->setPlaceholderText(QCoreApplication::translate("shared_screen", "\350\257\267\350\276\223\345\205\245\346\210\277\351\227\264\345\217\267", nullptr));
        btnJoinMeeting->setText(QCoreApplication::translate("shared_screen", "\345\212\240\345\205\245\344\274\232\350\256\256", nullptr));
        screenPreview->setText(QCoreApplication::translate("shared_screen", "\345\261\217\345\271\225\351\242\204\350\247\210\345\214\272\345\237\237\n"
"\347\202\271\345\207\273\"\345\205\261\344\272\253\345\261\217\345\271\225\"\345\274\200\345\247\213", nullptr));
        statusLabel->setText(QCoreApplication::translate("shared_screen", "\347\212\266\346\200\201:\346\234\252\350\277\236\346\216\245", nullptr));
        dockChat->setWindowTitle(QCoreApplication::translate("shared_screen", "\360\237\222\254 \350\201\212\345\244\251", nullptr));
        chatInput->setPlaceholderText(QCoreApplication::translate("shared_screen", "\350\276\223\345\205\245\346\266\210\346\201\257...", nullptr));
        btnSend->setText(QCoreApplication::translate("shared_screen", "\345\217\221\351\200\201", nullptr));
    } // retranslateUi

};

namespace Ui {
    class shared_screen: public Ui_shared_screen {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SHARED_SCREEN_H
