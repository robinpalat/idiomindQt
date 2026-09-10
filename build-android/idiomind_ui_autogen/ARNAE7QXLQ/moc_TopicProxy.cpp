/****************************************************************************
** Meta object code from reading C++ file 'TopicProxy.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/ui/controllers/TopicProxy.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TopicProxy.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN8idiomind2ui10TopicProxyE_t {};
} // unnamed namespace

template <> constexpr inline auto idiomind::ui::TopicProxy::qt_create_metaobjectdata<qt_meta_tag_ZN8idiomind2ui10TopicProxyE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "idiomind::ui::TopicProxy",
        "activeTopicChanged",
        "",
        "name",
        "activeLanguageChanged",
        "tlng",
        "topicsChanged",
        "indexChanged",
        "noteChanged",
        "metadataChanged",
        "tasksChanged",
        "learntChanged",
        "refreshTopics",
        "selectTopic",
        "activeTopicStts",
        "refreshTasks",
        "importIdmnd",
        "filePath",
        "saveNote",
        "text",
        "performAction",
        "markItemAsLearned",
        "trgt",
        "restartPractice",
        "loadActiveTopic",
        "saveActiveTopic",
        "stateImage",
        "stts",
        "itemAt",
        "QVariantMap",
        "index",
        "itemCount",
        "activeTopic",
        "activeLanguage",
        "topicCount",
        "topicModel",
        "TopicListModel*",
        "indexModel",
        "IndexListModel*",
        "note",
        "taskModel",
        "TaskListModel*",
        "learntModel",
        "nwrd",
        "nsnt",
        "nimg",
        "dtec",
        "autr",
        "slng",
        "ctgy",
        "levl",
        "nsze",
        "actionLabel"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'activeTopicChanged'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'activeLanguageChanged'
        QtMocHelpers::SignalData<void(const QString &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 },
        }}),
        // Signal 'topicsChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'indexChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'noteChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'metadataChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'tasksChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'learntChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'refreshTopics'
        QtMocHelpers::MethodData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'selectTopic'
        QtMocHelpers::MethodData<void(const QString &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Method 'activeTopicStts'
        QtMocHelpers::MethodData<QString() const>(14, 2, QMC::AccessPublic, QMetaType::QString),
        // Method 'refreshTasks'
        QtMocHelpers::MethodData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'importIdmnd'
        QtMocHelpers::MethodData<bool(const QString &)>(16, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 17 },
        }}),
        // Method 'saveNote'
        QtMocHelpers::MethodData<void(const QString &)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 19 },
        }}),
        // Method 'performAction'
        QtMocHelpers::MethodData<bool()>(20, 2, QMC::AccessPublic, QMetaType::Bool),
        // Method 'markItemAsLearned'
        QtMocHelpers::MethodData<bool(const QString &)>(21, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 22 },
        }}),
        // Method 'restartPractice'
        QtMocHelpers::MethodData<bool()>(23, 2, QMC::AccessPublic, QMetaType::Bool),
        // Method 'loadActiveTopic'
        QtMocHelpers::MethodData<void()>(24, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'saveActiveTopic'
        QtMocHelpers::MethodData<void()>(25, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'stateImage'
        QtMocHelpers::MethodData<QString(const QString &) const>(26, 2, QMC::AccessPublic, QMetaType::QString, {{
            { QMetaType::QString, 27 },
        }}),
        // Method 'itemAt'
        QtMocHelpers::MethodData<QVariantMap(int) const>(28, 2, QMC::AccessPublic, 0x80000000 | 29, {{
            { QMetaType::Int, 30 },
        }}),
        // Method 'itemCount'
        QtMocHelpers::MethodData<int() const>(31, 2, QMC::AccessPublic, QMetaType::Int),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'activeTopic'
        QtMocHelpers::PropertyData<QString>(32, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'activeLanguage'
        QtMocHelpers::PropertyData<QString>(33, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 1),
        // property 'topicCount'
        QtMocHelpers::PropertyData<int>(34, QMetaType::Int, QMC::DefaultPropertyFlags, 2),
        // property 'topicModel'
        QtMocHelpers::PropertyData<TopicListModel*>(35, 0x80000000 | 36, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'indexModel'
        QtMocHelpers::PropertyData<IndexListModel*>(37, 0x80000000 | 38, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'note'
        QtMocHelpers::PropertyData<QString>(39, QMetaType::QString, QMC::DefaultPropertyFlags, 4),
        // property 'taskModel'
        QtMocHelpers::PropertyData<TaskListModel*>(40, 0x80000000 | 41, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'learntModel'
        QtMocHelpers::PropertyData<IndexListModel*>(42, 0x80000000 | 38, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'stts'
        QtMocHelpers::PropertyData<QString>(27, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'nwrd'
        QtMocHelpers::PropertyData<QString>(43, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'nsnt'
        QtMocHelpers::PropertyData<QString>(44, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'nimg'
        QtMocHelpers::PropertyData<QString>(45, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'dtec'
        QtMocHelpers::PropertyData<QString>(46, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'autr'
        QtMocHelpers::PropertyData<QString>(47, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'slng'
        QtMocHelpers::PropertyData<QString>(48, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'tlng'
        QtMocHelpers::PropertyData<QString>(5, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'ctgy'
        QtMocHelpers::PropertyData<QString>(49, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'levl'
        QtMocHelpers::PropertyData<QString>(50, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'nsze'
        QtMocHelpers::PropertyData<QString>(51, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
        // property 'actionLabel'
        QtMocHelpers::PropertyData<QString>(52, QMetaType::QString, QMC::DefaultPropertyFlags, 5),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<TopicProxy, qt_meta_tag_ZN8idiomind2ui10TopicProxyE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject idiomind::ui::TopicProxy::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8idiomind2ui10TopicProxyE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8idiomind2ui10TopicProxyE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN8idiomind2ui10TopicProxyE_t>.metaTypes,
    nullptr
} };

void idiomind::ui::TopicProxy::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<TopicProxy *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->activeTopicChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->activeLanguageChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->topicsChanged(); break;
        case 3: _t->indexChanged(); break;
        case 4: _t->noteChanged(); break;
        case 5: _t->metadataChanged(); break;
        case 6: _t->tasksChanged(); break;
        case 7: _t->learntChanged(); break;
        case 8: _t->refreshTopics(); break;
        case 9: _t->selectTopic((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 10: { QString _r = _t->activeTopicStts();
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 11: _t->refreshTasks(); break;
        case 12: { bool _r = _t->importIdmnd((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 13: _t->saveNote((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 14: { bool _r = _t->performAction();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 15: { bool _r = _t->markItemAsLearned((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 16: { bool _r = _t->restartPractice();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 17: _t->loadActiveTopic(); break;
        case 18: _t->saveActiveTopic(); break;
        case 19: { QString _r = _t->stateImage((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        case 20: { QVariantMap _r = _t->itemAt((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<QVariantMap*>(_a[0]) = std::move(_r); }  break;
        case 21: { int _r = _t->itemCount();
            if (_a[0]) *reinterpret_cast<int*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (TopicProxy::*)(const QString & )>(_a, &TopicProxy::activeTopicChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (TopicProxy::*)(const QString & )>(_a, &TopicProxy::activeLanguageChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (TopicProxy::*)()>(_a, &TopicProxy::topicsChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (TopicProxy::*)()>(_a, &TopicProxy::indexChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (TopicProxy::*)()>(_a, &TopicProxy::noteChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (TopicProxy::*)()>(_a, &TopicProxy::metadataChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (TopicProxy::*)()>(_a, &TopicProxy::tasksChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (TopicProxy::*)()>(_a, &TopicProxy::learntChanged, 7))
            return;
    }
    if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 7:
        case 4:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< IndexListModel* >(); break;
        case 6:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< TaskListModel* >(); break;
        case 3:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< TopicListModel* >(); break;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QString*>(_v) = _t->activeTopic(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->activeLanguage(); break;
        case 2: *reinterpret_cast<int*>(_v) = _t->topicCount(); break;
        case 3: *reinterpret_cast<TopicListModel**>(_v) = _t->topicModel(); break;
        case 4: *reinterpret_cast<IndexListModel**>(_v) = _t->indexModel(); break;
        case 5: *reinterpret_cast<QString*>(_v) = _t->note(); break;
        case 6: *reinterpret_cast<TaskListModel**>(_v) = _t->taskModel(); break;
        case 7: *reinterpret_cast<IndexListModel**>(_v) = _t->learntModel(); break;
        case 8: *reinterpret_cast<QString*>(_v) = _t->stts(); break;
        case 9: *reinterpret_cast<QString*>(_v) = _t->nwrd(); break;
        case 10: *reinterpret_cast<QString*>(_v) = _t->nsnt(); break;
        case 11: *reinterpret_cast<QString*>(_v) = _t->nimg(); break;
        case 12: *reinterpret_cast<QString*>(_v) = _t->dtec(); break;
        case 13: *reinterpret_cast<QString*>(_v) = _t->autr(); break;
        case 14: *reinterpret_cast<QString*>(_v) = _t->slng(); break;
        case 15: *reinterpret_cast<QString*>(_v) = _t->tlng(); break;
        case 16: *reinterpret_cast<QString*>(_v) = _t->ctgy(); break;
        case 17: *reinterpret_cast<QString*>(_v) = _t->levl(); break;
        case 18: *reinterpret_cast<QString*>(_v) = _t->nsze(); break;
        case 19: *reinterpret_cast<QString*>(_v) = _t->actionLabel(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 1: _t->setActiveLanguage(*reinterpret_cast<QString*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *idiomind::ui::TopicProxy::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *idiomind::ui::TopicProxy::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8idiomind2ui10TopicProxyE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int idiomind::ui::TopicProxy::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 22)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 22)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 22;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    }
    return _id;
}

// SIGNAL 0
void idiomind::ui::TopicProxy::activeTopicChanged(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void idiomind::ui::TopicProxy::activeLanguageChanged(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void idiomind::ui::TopicProxy::topicsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void idiomind::ui::TopicProxy::indexChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void idiomind::ui::TopicProxy::noteChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void idiomind::ui::TopicProxy::metadataChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void idiomind::ui::TopicProxy::tasksChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void idiomind::ui::TopicProxy::learntChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}
QT_WARNING_POP
