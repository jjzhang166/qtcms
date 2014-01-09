#ifndef __PREVIEWWNDTEST_H__
#define __PREVIEWWNDTEST_H__

#include "previewwindowstest_global.h"
#include <QMutex>
#include <qwfw.h>
#include "IWindowDivMode.h"
#include <QtGui/QWidget>
#include <QDir>  
#include <QFileInfo>


class PreviewWndTest :public QWidget,
    public QWebPluginFWBase
{
    Q_OBJECT
public:
    PreviewWndTest(QWidget *parent = 0);
    ~PreviewWndTest();

    public slots:
        void AddEventProc( const QString sEvent,QString sProc ){m_mapEventProc.insertMulti(sEvent,sProc);};

        int AddPaths(const QString&sDevName);
        int IsPathsAndFilesExist();
        int RemovePaths();
private:
    QFileSystemWatcher m_watcher;
    QString m_sDevName;
    QDir  m_qDir;
    QStringList m_sPathList;

private:
    int rmFiles(QString &sFileName);
    private slots:
        void slotDirectoryChanged(const QString& path);

};

#endif // __PREVIEWWNDTEST_H__
