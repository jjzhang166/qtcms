/****************************************************************************
** Meta object code from reading C++ file 'group_test.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../group_test.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'group_test.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_group_test[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      22,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x08,
      34,   11,   11,   11, 0x08,
      60,   11,   11,   11, 0x08,
      85,   11,   11,   11, 0x08,
     110,   11,   11,   11, 0x08,
     137,   11,   11,   11, 0x08,
     163,   11,   11,   11, 0x08,
     193,   11,   11,   11, 0x08,
     227,   11,   11,   11, 0x08,
     256,   11,   11,   11, 0x08,
     292,   11,   11,   11, 0x08,
     323,   11,   11,   11, 0x08,
     359,   11,   11,   11, 0x08,
     395,   11,   11,   11, 0x08,
     427,   11,   11,   11, 0x08,
     464,   11,   11,   11, 0x08,
     498,   11,   11,   11, 0x08,
     531,   11,   11,   11, 0x08,
     572,   11,   11,   11, 0x08,
     607,   11,   11,   11, 0x08,
     646,   11,   11,   11, 0x08,
     687,   11,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_group_test[] = {
    "group_test\0\0Group_AddGroup_test()\0"
    "Group_IsGroupExist_test()\0"
    "Group_RemoveGroup_test()\0"
    "Group_ModifyGroup_test()\0"
    "Group_GetGroupCount_test()\0"
    "Group_GetGroupList_test()\0"
    "Group_GetGroupName_int_test()\0"
    "Group_GetGroupName_QString_test()\0"
    "Group_IsChannelExists_test()\0"
    "Group_IsR_Channel_GroupExist_test()\0"
    "Group_AddChannelInGroup_test()\0"
    "Group_RemoveChannelFromGroup_test()\0"
    "Group_ModifyGroupChannelName_test()\0"
    "Group_MoveChannelToGroup_test()\0"
    "Group_GetGroupChannelName_int_test()\0"
    "Group_GetGroupChannelCount_test()\0"
    "Group_GetGroupChannelList_test()\0"
    "Group_GetGroupChannelName_qstring_test()\0"
    "Group_GetChannelIdFromGroup_test()\0"
    "Group_GetChannelIdFromGroup_one_test()\0"
    "Group_GetChannelInfoFromGroup_int_test()\0"
    "Group_GetChannelInfoFromGroup_QVariant_test()\0"
};

void group_test::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        group_test *_t = static_cast<group_test *>(_o);
        switch (_id) {
        case 0: _t->Group_AddGroup_test(); break;
        case 1: _t->Group_IsGroupExist_test(); break;
        case 2: _t->Group_RemoveGroup_test(); break;
        case 3: _t->Group_ModifyGroup_test(); break;
        case 4: _t->Group_GetGroupCount_test(); break;
        case 5: _t->Group_GetGroupList_test(); break;
        case 6: _t->Group_GetGroupName_int_test(); break;
        case 7: _t->Group_GetGroupName_QString_test(); break;
        case 8: _t->Group_IsChannelExists_test(); break;
        case 9: _t->Group_IsR_Channel_GroupExist_test(); break;
        case 10: _t->Group_AddChannelInGroup_test(); break;
        case 11: _t->Group_RemoveChannelFromGroup_test(); break;
        case 12: _t->Group_ModifyGroupChannelName_test(); break;
        case 13: _t->Group_MoveChannelToGroup_test(); break;
        case 14: _t->Group_GetGroupChannelName_int_test(); break;
        case 15: _t->Group_GetGroupChannelCount_test(); break;
        case 16: _t->Group_GetGroupChannelList_test(); break;
        case 17: _t->Group_GetGroupChannelName_qstring_test(); break;
        case 18: _t->Group_GetChannelIdFromGroup_test(); break;
        case 19: _t->Group_GetChannelIdFromGroup_one_test(); break;
        case 20: _t->Group_GetChannelInfoFromGroup_int_test(); break;
        case 21: _t->Group_GetChannelInfoFromGroup_QVariant_test(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData group_test::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject group_test::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_group_test,
      qt_meta_data_group_test, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &group_test::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *group_test::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *group_test::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_group_test))
        return static_cast<void*>(const_cast< group_test*>(this));
    return QObject::qt_metacast(_clname);
}

int group_test::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 22)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
