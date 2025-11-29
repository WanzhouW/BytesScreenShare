/****************************************************************************
** Meta object code from reading C++ file 'shared_screen.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/shared_screen.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'shared_screen.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN13shared_screenE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN13shared_screenE = QtMocHelpers::stringData(
    "shared_screen",
    "on_btnVoiceClicked",
    "",
    "on_btnShareScreenClicked",
    "on_btnChatClicked",
    "on_btnSendClicked",
    "on_btnVideoClicked",
    "on_btnParticipantsClicked",
    "on_btnRecordClicked",
    "on_btnRaiseHandClicked",
    "on_btnLeaveClicked",
    "on_fakeNetworkTick",
    "on_fakeRemoteMsg",
    "on_btnJoinMeetingClicked",
    "onRecorderStateChanged",
    "QMediaRecorder::RecorderState",
    "state",
    "onRecorderError",
    "QMediaRecorder::Error",
    "error",
    "errorString",
    "onRecordingDurationChanged",
    "duration",
    "updateRecordingTime",
    "captureScreen"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN13shared_screenE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  116,    2, 0x08,    1 /* Private */,
       3,    0,  117,    2, 0x08,    2 /* Private */,
       4,    0,  118,    2, 0x08,    3 /* Private */,
       5,    0,  119,    2, 0x08,    4 /* Private */,
       6,    0,  120,    2, 0x08,    5 /* Private */,
       7,    0,  121,    2, 0x08,    6 /* Private */,
       8,    0,  122,    2, 0x08,    7 /* Private */,
       9,    0,  123,    2, 0x08,    8 /* Private */,
      10,    0,  124,    2, 0x08,    9 /* Private */,
      11,    0,  125,    2, 0x08,   10 /* Private */,
      12,    0,  126,    2, 0x08,   11 /* Private */,
      13,    0,  127,    2, 0x08,   12 /* Private */,
      14,    1,  128,    2, 0x08,   13 /* Private */,
      17,    2,  131,    2, 0x08,   15 /* Private */,
      21,    1,  136,    2, 0x08,   18 /* Private */,
      23,    0,  139,    2, 0x08,   20 /* Private */,
      24,    0,  140,    2, 0x08,   21 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 15,   16,
    QMetaType::Void, 0x80000000 | 18, QMetaType::QString,   19,   20,
    QMetaType::Void, QMetaType::LongLong,   22,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject shared_screen::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_ZN13shared_screenE.offsetsAndSizes,
    qt_meta_data_ZN13shared_screenE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN13shared_screenE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<shared_screen, std::true_type>,
        // method 'on_btnVoiceClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnShareScreenClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnChatClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnSendClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnVideoClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnParticipantsClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnRecordClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnRaiseHandClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnLeaveClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_fakeNetworkTick'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_fakeRemoteMsg'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnJoinMeetingClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRecorderStateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QMediaRecorder::RecorderState, std::false_type>,
        // method 'onRecorderError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QMediaRecorder::Error, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onRecordingDurationChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        // method 'updateRecordingTime'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'captureScreen'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void shared_screen::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<shared_screen *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->on_btnVoiceClicked(); break;
        case 1: _t->on_btnShareScreenClicked(); break;
        case 2: _t->on_btnChatClicked(); break;
        case 3: _t->on_btnSendClicked(); break;
        case 4: _t->on_btnVideoClicked(); break;
        case 5: _t->on_btnParticipantsClicked(); break;
        case 6: _t->on_btnRecordClicked(); break;
        case 7: _t->on_btnRaiseHandClicked(); break;
        case 8: _t->on_btnLeaveClicked(); break;
        case 9: _t->on_fakeNetworkTick(); break;
        case 10: _t->on_fakeRemoteMsg(); break;
        case 11: _t->on_btnJoinMeetingClicked(); break;
        case 12: _t->onRecorderStateChanged((*reinterpret_cast< std::add_pointer_t<QMediaRecorder::RecorderState>>(_a[1]))); break;
        case 13: _t->onRecorderError((*reinterpret_cast< std::add_pointer_t<QMediaRecorder::Error>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 14: _t->onRecordingDurationChanged((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1]))); break;
        case 15: _t->updateRecordingTime(); break;
        case 16: _t->captureScreen(); break;
        default: ;
        }
    }
}

const QMetaObject *shared_screen::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *shared_screen::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN13shared_screenE.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int shared_screen::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 17)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 17;
    }
    return _id;
}
QT_WARNING_POP
