/****************************************************************************
** Meta object code from reading C++ file 'mainviewwidget.h'
**
** Created: Mon Oct 15 16:59:18 2007
**      by: The Qt Meta Object Compiler version 59 (Qt 4.3.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mainviewwidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainviewwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.3.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

static const uint qt_meta_data_MainViewWidget[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x05,

 // slots: signature, parameters, type, tag, flags
      32,   30,   15,   15, 0x0a,
      73,   61,   15,   15, 0x0a,
     112,   15,   15,   15, 0x0a,
     127,   15,   15,   15, 0x0a,
     138,   15,   15,   15, 0x0a,
     154,   15,   15,   15, 0x0a,
     167,   61,   15,   15, 0x0a,
     204,   15,   15,   15, 0x0a,
     218,   15,   15,   15, 0x0a,
     242,  240,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MainViewWidget[] = {
    "MainViewWidget\0\0faceChanged()\0s\0"
    "slotOrderingChanged(QString)\0item,column\0"
    "slotfontSelected(QTreeWidgetItem*,int)\0"
    "slotInfoFont()\0slotView()\0slotglyphInfo()\0"
    "slotSearch()\0slotFontAction(QTreeWidgetItem*,int)\0"
    "slotEditAll()\0slotCleanFontAction()\0"
    "z\0slotZoom(int)\0"
};

const QMetaObject MainViewWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_MainViewWidget,
      qt_meta_data_MainViewWidget, 0 }
};

const QMetaObject *MainViewWidget::metaObject() const
{
    return &staticMetaObject;
}

void *MainViewWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainViewWidget))
	return static_cast<void*>(const_cast< MainViewWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int MainViewWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: faceChanged(); break;
        case 1: slotOrderingChanged((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: slotfontSelected((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: slotInfoFont(); break;
        case 4: slotView(); break;
        case 5: slotglyphInfo(); break;
        case 6: slotSearch(); break;
        case 7: slotFontAction((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 8: slotEditAll(); break;
        case 9: slotCleanFontAction(); break;
        case 10: slotZoom((*reinterpret_cast< int(*)>(_a[1]))); break;
        }
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void MainViewWidget::faceChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
