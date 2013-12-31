#ifndef __SETRECORDTIMETEST_H_
#define __SETRECORDTIMETEST_H_

#include <QObject>
#include <QString>
#include <QtTest>

class SetRecordTimeTest :
    public QObject
{
    Q_OBJECT
public:
    SetRecordTimeTest(void);
    ~SetRecordTimeTest(void);
    void beforeSetRecordTimeTest();

    private Q_SLOTS:
        void SetRecordTimeTest1();
        void SetRecordTimeTest2();
        void SetRecordTimeTest3();
        void SetRecordTimeTest4();
        void SetRecordTimeTest5();
        void SetRecordTimeTest6();

};

#endif