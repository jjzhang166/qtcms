#ifndef __DISKSETTINGS_H_
#define __DISKSETTINGS_H_

#include <QObject>
#include <QString>
#include <QtTest>

class DisksSettingTest :
    public QObject
{
    Q_OBJECT
public:
    DisksSettingTest(void);
    ~DisksSettingTest(void);
    void beforeDisksSettingTest();

    private Q_SLOTS:
        void DisksSettingTest1();
        void DisksSettingTest2();
        void DisksSettingTest3();
        void DisksSettingTest4();
        void DisksSettingTest5();
        void DisksSettingTest6();
        void DisksSettingTest7();
        void DisksSettingTest8();

};

#endif