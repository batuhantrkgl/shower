/****************************************************************************
** Meta object code from reading C++ file 'networkclient.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../networkclient.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'networkclient.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_NetworkClient_t {
    uint offsetsAndSizes[30];
    char stringdata0[14];
    char stringdata1[17];
    char stringdata2[1];
    char stringdata3[12];
    char stringdata4[10];
    char stringdata5[21];
    char stringdata6[9];
    char stringdata7[14];
    char stringdata8[10];
    char stringdata9[6];
    char stringdata10[13];
    char stringdata11[6];
    char stringdata12[24];
    char stringdata13[21];
    char stringdata14[14];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_NetworkClient_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_NetworkClient_t qt_meta_stringdata_NetworkClient = {
    {
        QT_MOC_LITERAL(0, 13),  // "NetworkClient"
        QT_MOC_LITERAL(14, 16),  // "scheduleReceived"
        QT_MOC_LITERAL(31, 0),  // ""
        QT_MOC_LITERAL(32, 11),  // "schoolStart"
        QT_MOC_LITERAL(44, 9),  // "schoolEnd"
        QT_MOC_LITERAL(54, 20),  // "QList<ScheduleBlock>"
        QT_MOC_LITERAL(75, 8),  // "schedule"
        QT_MOC_LITERAL(84, 13),  // "mediaReceived"
        QT_MOC_LITERAL(98, 9),  // "MediaInfo"
        QT_MOC_LITERAL(108, 5),  // "media"
        QT_MOC_LITERAL(114, 12),  // "networkError"
        QT_MOC_LITERAL(127, 5),  // "error"
        QT_MOC_LITERAL(133, 23),  // "onScheduleReplyFinished"
        QT_MOC_LITERAL(157, 20),  // "onMediaReplyFinished"
        QT_MOC_LITERAL(178, 13)   // "periodicFetch"
    },
    "NetworkClient",
    "scheduleReceived",
    "",
    "schoolStart",
    "schoolEnd",
    "QList<ScheduleBlock>",
    "schedule",
    "mediaReceived",
    "MediaInfo",
    "media",
    "networkError",
    "error",
    "onScheduleReplyFinished",
    "onMediaReplyFinished",
    "periodicFetch"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_NetworkClient[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    3,   50,    2, 0x06,    1 /* Public */,
       7,    1,   57,    2, 0x06,    5 /* Public */,
      10,    1,   60,    2, 0x06,    7 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      12,    0,   63,    2, 0x08,    9 /* Private */,
      13,    0,   64,    2, 0x08,   10 /* Private */,
      14,    0,   65,    2, 0x08,   11 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QTime, QMetaType::QTime, 0x80000000 | 5,    3,    4,    6,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, QMetaType::QString,   11,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject NetworkClient::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_NetworkClient.offsetsAndSizes,
    qt_meta_data_NetworkClient,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_NetworkClient_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<NetworkClient, std::true_type>,
        // method 'scheduleReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QTime &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QTime &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QList<ScheduleBlock> &, std::false_type>,
        // method 'mediaReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const MediaInfo &, std::false_type>,
        // method 'networkError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onScheduleReplyFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onMediaReplyFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'periodicFetch'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void NetworkClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<NetworkClient *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->scheduleReceived((*reinterpret_cast< std::add_pointer_t<QTime>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QTime>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QList<ScheduleBlock>>>(_a[3]))); break;
        case 1: _t->mediaReceived((*reinterpret_cast< std::add_pointer_t<MediaInfo>>(_a[1]))); break;
        case 2: _t->networkError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->onScheduleReplyFinished(); break;
        case 4: _t->onMediaReplyFinished(); break;
        case 5: _t->periodicFetch(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (NetworkClient::*)(const QTime & , const QTime & , const QList<ScheduleBlock> & );
            if (_t _q_method = &NetworkClient::scheduleReceived; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (NetworkClient::*)(const MediaInfo & );
            if (_t _q_method = &NetworkClient::mediaReceived; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (NetworkClient::*)(const QString & );
            if (_t _q_method = &NetworkClient::networkError; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject *NetworkClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *NetworkClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_NetworkClient.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int NetworkClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void NetworkClient::scheduleReceived(const QTime & _t1, const QTime & _t2, const QList<ScheduleBlock> & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void NetworkClient::mediaReceived(const MediaInfo & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void NetworkClient::networkError(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
