#ifndef LOCALRECORDSEARCHTEST_H
#define LOCALRECORDSEARCHTEST_H

#include <QObject>
#include <QString>
#include <QtTest>

class LocalRecordSearchTest : public QObject
{
    Q_OBJECT

public:
    LocalRecordSearchTest();
    ~LocalRecordSearchTest();
    static int cbProc(QString evName, QVariantMap evMap, void *pUser);
private:
    void beforeTest();

    private Q_SLOTS:
        void TestCase1();
        void TestCase2();
        void TestCase3();

};

#endif // LOCALRECORDSEARCHTEST_H
