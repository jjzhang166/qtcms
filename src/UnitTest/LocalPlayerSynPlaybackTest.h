#ifndef __LOCALPLAYERSYNPLAYBACKTEST_H
#define __LOCALPLAYERSYNPLAYBACKTEST_H

#include <QObject>
#include "LocalPlayerSynPlaybackWnd.h"
#include <QString>
#include <QtTest>
#include "IWindowDivMode.h"

class LocalPlayerSynPlaybackTest : public QWidget
{
    Q_OBJECT

public:
    LocalPlayerSynPlaybackTest();
    ~LocalPlayerSynPlaybackTest();
    static int cbProc(QString evName, QVariantMap evMap, void *pUser);
private:
    void beforeTest();

    private Q_SLOTS:
        void TestCase1();
        void TestCase2();
        void TestCase3();
        void TestCase4();
        void TestCase5();
        void TestCase6();
private:
    LocalPlayerSynPlaybackWnd m_PlaybackWnd[4];
    IWindowDivMode *m_DivMode;
    QList<QWidget *> m_PlaybackWndList;
};


#endif