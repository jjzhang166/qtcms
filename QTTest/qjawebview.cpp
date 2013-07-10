#include "qjawebview.h"
#include <QCoreApplication>
#include <QSettings>
#include <QKeyEvent>

QJaWebView::QJaWebView(QWidget *parent) :
    QWebView(parent)
{
    setWindowFlags(Qt::FramelessWindowHint);

    // Get Application Path
    QString temp = QCoreApplication::applicationDirPath();
    m_sApplicationPath.append(temp);

    // Read Main ini file
    QSettings MainIniFile(m_sApplicationPath + "/MainSet.ini",QSettings::IniFormat);
    QString sTheme = MainIniFile.value(QString("Configure/Theme")).toString();
    QString sThemeDir = MainIniFile.value(sTheme + "/Dir").toString();
    QString sUiDir = m_sApplicationPath + sThemeDir;
    qDebug("%s",sUiDir.toAscii().data());
    load("file:///" + sUiDir);

    // Connect Event
}

void QJaWebView::keyPressEvent(QKeyEvent *ev)
{
    switch(ev->key())
    {
    case Qt::Key_Escape:
        {
            close();
        }
        break;
    }
}
