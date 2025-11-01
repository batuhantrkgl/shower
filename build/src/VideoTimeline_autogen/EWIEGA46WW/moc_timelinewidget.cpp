/****************************************************************************
** Meta object code from reading C++ file 'timelinewidget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/timelinewidget.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'timelinewidget.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN14TimelineWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto TimelineWidget::qt_create_metaobjectdata<qt_meta_tag_ZN14TimelineWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "TimelineWidget",
        "updateCurrentTime",
        "",
        "currentTime",
        "onScheduleReceived",
        "schoolStart",
        "schoolEnd",
        "QList<ScheduleBlock>",
        "schedule",
        "onNetworkError",
        "error"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'updateCurrentTime'
        QtMocHelpers::SlotData<void(const QTime &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QTime, 3 },
        }}),
        // Slot 'onScheduleReceived'
        QtMocHelpers::SlotData<void(const QTime &, const QTime &, const QList<ScheduleBlock> &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QTime, 5 }, { QMetaType::QTime, 6 }, { 0x80000000 | 7, 8 },
        }}),
        // Slot 'onNetworkError'
        QtMocHelpers::SlotData<void(const QString &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<TimelineWidget, qt_meta_tag_ZN14TimelineWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject TimelineWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14TimelineWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14TimelineWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14TimelineWidgetE_t>.metaTypes,
    nullptr
} };

void TimelineWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<TimelineWidget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->updateCurrentTime((*reinterpret_cast<std::add_pointer_t<QTime>>(_a[1]))); break;
        case 1: _t->onScheduleReceived((*reinterpret_cast<std::add_pointer_t<QTime>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QTime>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<QList<ScheduleBlock>>>(_a[3]))); break;
        case 2: _t->onNetworkError((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *TimelineWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TimelineWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14TimelineWidgetE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int TimelineWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
