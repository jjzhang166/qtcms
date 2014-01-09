#include <qwfw_tools.h>
#include <libpcom.h>
#include <guid.h>
#include "PreviewWndTest.h"
#include <QtWebKit/QWebView>
#include <QtWebKit/QWebFrame>
#include <QDate>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtGui/QMessageBox>

PreviewWndTest::PreviewWndTest(QWidget *parent)
    : QWidget(parent)
    ,QWebPluginFWBase(this)
{

    QObject::connect(&m_watcher,SIGNAL(directoryChanged(const QString&)),
        this,SLOT(slotDirectoryChanged(const QString&)));

}

PreviewWndTest::~PreviewWndTest()
{
    m_mapEventProc.clear();
}

int PreviewWndTest::AddPaths(const QString&sDevName)
{
    if(sDevName.isEmpty() )
        return -1;

    m_sDevName = sDevName;
    m_sDevName = QString(":/JAREC/") +QDate::currentDate().toString("yyyy-MM-dd")+ QString("/%1/").arg(m_sDevName);

    QSqlDatabase db;
    if (QSqlDatabase::contains("PreviewWndRecTest"))
    {
        db = QSqlDatabase::database("PreviewWndRecTest"); 
    }
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE","PreviewWndRecTest");
    }
    QString sAppPath = QCoreApplication::applicationDirPath();
    QString sDatabasePath = sAppPath + "/system.db";
    db.setDatabaseName(sDatabasePath);
    db.open();
    QSqlQuery query1(db);
    QString sCommand = QString("select value from general_setting where name='%1'").arg("storage_usedisks");
    query1.exec(sCommand);
    QString sPath;
    while (query1.next())
    {
        sPath = query1.value(0).toString();
    }
    db.close();

    m_sPathList.clear();
    m_sPathList = sPath.split(':',QString::SkipEmptyParts);
    if (m_sPathList.isEmpty() || m_sPathList.at(0) == sPath)
    {
        return 1;
    }
    int j = 0;
    for (; j < m_sPathList.size(); ++j)
    {
        m_sPathList[j] += m_sDevName;
    }
    
    m_watcher.addPaths(m_sPathList);
    
    return 0;
}
int PreviewWndTest::IsPathsAndFilesExist()
{
    int j = 0;
    QString sRichTxt;
    for (; j < m_sPathList.size(); ++j)
    {
        m_qDir.setPath(m_sPathList[j]);
        if (!m_qDir.exists())
        {
            qDebug()<<" The Path is not exist.";
            return -1;
        }
        m_qDir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
        m_qDir.setSorting(QDir::DirsFirst| QDir::Name);
        QFileInfoList fileList = m_qDir.entryInfoList();
        foreach(QFileInfo fi,fileList)
        {
            if (fi.isDir())
            {
                sRichTxt += fi.absoluteFilePath() + QString("\n");
                QDir dirTmp(fi.absoluteFilePath());
                dirTmp.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
                dirTmp.setSorting(QDir::DirsFirst| QDir::Name);
                QFileInfoList fileInfoList = dirTmp.entryInfoList();
                foreach(QFileInfo fileInfo, fileInfoList)
                {
                    if (fileInfo.isFile())
                    {
                        sRichTxt += fileInfo.absoluteFilePath() + QString("\t%1MB\n").arg((float)fileInfo.size()/(1024*1024));
                    }
                }
            }
            if (fi.isFile())
            {
                sRichTxt += fi.absoluteFilePath()+ QString("\t%1MB\n").arg((float)fi.size()/(1024*1024));
            }
        }
    }
    qDebug()<<sRichTxt;
    return 0;
}



void PreviewWndTest::slotDirectoryChanged(const QString& sPath)
{
    qDebug()<<sPath<<" is changing!";
}

int  PreviewWndTest::RemovePaths()
{
    int j = 0, nRet = 0;
    for (; j < m_sPathList.size(); ++j)
    {
        nRet |= rmFiles(m_sPathList[j]);
    }
    return nRet;
}

int PreviewWndTest::rmFiles(QString &sFileName)
{
    m_qDir.setPath(sFileName);
    if (!m_qDir.exists())
    {
        return -1;
    }
    m_qDir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    m_qDir.setSorting(QDir::DirsLast| QDir::Name);
    QFileInfoList fileList = m_qDir.entryInfoList();
    foreach(QFileInfo fi,fileList)
    {
        if (fi.isFile())
            fi.dir().remove(fi.fileName());

        else
            rmFiles(fi.absoluteFilePath());
        if (fi.isDir())
        {
            fi.dir().rmpath(fi.absoluteFilePath());
        }
    }
    return 0;
}

