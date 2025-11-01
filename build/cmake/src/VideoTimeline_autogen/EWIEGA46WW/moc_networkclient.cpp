/****************************************************************************
** Meta object code from reading C++ file 'networkclient.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../src/networkclient.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'networkclient.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.0. It"
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
struct qt_meta_tag_ZN13NetworkClientE_t {};
} // unnamed namespace

template <> constexpr inline auto NetworkClient::qt_create_metaobjectdata<qt_meta_tag_ZN13NetworkClientE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "NetworkClient",
        "scheduleReceived",
        "",
        "schoolStart",
        "schoolEnd",
        "QList<ScheduleBlock>",
        "schedule",
        "playlistReceived",
        "MediaPlaylist",
        "playlist",
        "networkError",
        "error",
        "serverDiscovered",
        "serverUrl",
        "connectionStatusChanged",
        "connected",
        "hostname",
        "pingUpdated",
        "pingMs",
        "onScheduleReplyFinished",
        "onMediaReplyFinished",
        "periodicFetch",
        "measurePing",
        "onPingReplyFinished",
        "attemptReconnection"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'scheduleReceived'
        QtMocHelpers::SignalData<void(const QTime &, const QTime &, const QList<ScheduleBlock> &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QTime, 3 }, { QMetaType::QTime, 4 }, { 0x80000000 | 5, 6 },
        }}),
        // Signal 'playlistReceived'
        QtMocHelpers::SignalData<void(const MediaPlaylist &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 8, 9 },
        }}),
        // Signal 'networkError'
        QtMocHelpers::SignalData<void(const QString &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 11 },
        }}),
        // Signal 'serverDiscovered'
        QtMocHelpers::SignalData<void(const QString &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 13 },
        }}),
        // Signal 'connectionStatusChanged'
        QtMocHelpers::SignalData<void(bool, const QString &, const QString &)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 15 }, { QMetaType::QString, 13 }, { QMetaType::QString, 16 },
        }}),
        // Signal 'connectionStatusChanged'
        QtMocHelpers::SignalData<void(bool, const QString &)>(14, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::Bool, 15 }, { QMetaType::QString, 13 },
        }}),
        // Signal 'connectionStatusChanged'
        QtMocHelpers::SignalData<void(bool)>(14, 2, QMC::AccessPublic | QMC::MethodCloned, QMetaType::Void, {{
            { QMetaType::Bool, 15 },
        }}),
        // Signal 'pingUpdated'
        QtMocHelpers::SignalData<void(int)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 18 },
        }}),
        // Slot 'onScheduleReplyFinished'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onMediaReplyFinished'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'periodicFetch'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'measurePing'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onPingReplyFinished'
        QtMocHelpers::SlotData<void()>(23, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'attemptReconnection'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<NetworkClient, qt_meta_tag_ZN13NetworkClientE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject NetworkClient::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13NetworkClientE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13NetworkClientE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13NetworkClientE_t>.metaTypes,
    nullptr
} };

void NetworkClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<NetworkClient *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->scheduleReceived((*reinterpret_cast<std::add_pointer_t<QTime>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QTime>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QList<ScheduleBlock>>>(_a[3]))); break;
        case 1: _t->playlistReceived((*reinterpret_cast<std::add_pointer_t<MediaPlaylist>>(_a[1]))); break;
        case 2: _t->networkError((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->serverDiscovered((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->connectionStatusChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[3]))); break;
        case 5: _t->connectionStatusChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 6: _t->connectionStatusChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 7: _t->pingUpdated((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->onScheduleReplyFinished(); break;
        case 9: _t->onMediaReplyFinished(); break;
        case 10: _t->periodicFetch(); break;
        case 11: _t->measurePing(); break;
        case 12: _t->onPingReplyFinished(); break;
        case 13: _t->attemptReconnection(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (NetworkClient::*)(const QTime & , const QTime & , const QList<ScheduleBlock> & )>(_a, &NetworkClient::scheduleReceived, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (NetworkClient::*)(const MediaPlaylist & )>(_a, &NetworkClient::playlistReceived, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (NetworkClient::*)(const QString & )>(_a, &NetworkClient::networkError, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (NetworkClient::*)(const QString & )>(_a, &NetworkClient::serverDiscovered, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (NetworkClient::*)(bool , const QString & , const QString & )>(_a, &NetworkClient::connectionStatusChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (NetworkClient::*)(int )>(_a, &NetworkClient::pingUpdated, 7))
            return;
    }
}

const QMetaObject *NetworkClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *NetworkClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13NetworkClientE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int NetworkClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 14;
    }
    return _id;
}

// SIGNAL 0
void NetworkClient::scheduleReceived(const QTime & _t1, const QTime & _t2, const QList<ScheduleBlock> & _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2, _t3);
}

// SIGNAL 1
void NetworkClient::playlistReceived(const MediaPlaylist & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void NetworkClient::networkError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void NetworkClient::serverDiscovered(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void NetworkClient::connectionStatusChanged(bool _t1, const QString & _t2, const QString & _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2, _t3);
}

// SIGNAL 7
void NetworkClient::pingUpdated(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1);
}
QT_WARNING_POP
