#ifndef __RECORDERTEST_H_
#define __RECORDERTEST_H_

#include <QObject>
#include <QString>
#include <QtTest>

class RecorderTest :
    public QObject
{
    Q_OBJECT
public:
    RecorderTest(void);
    ~RecorderTest(void);
    void beforeRecorderTest();

    static uint m_sTimeStamp;
    private Q_SLOTS:
        void RecorderTest1();
        void RecorderTest2();
        void RecorderTest3();
        void RecorderTest4();
        void RecorderTest5();
        void RecorderTest6();
		void RecorderTest7();

};

#endif