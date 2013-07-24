#include "qjawebview.h"
#include <QCoreApplication>
#include <QSettings>
#include <QKeyEvent>
#include "qpreviewactivity.h"


QJaWebView::QJaWebView(QWidget *parent) :
    QWebView(parent)
{
	// initialize activities
    CreatInstance(AI_PREVIEW,(void **)&m_Activities[AI_PREVIEW]);

    setWindowFlags(Qt::FramelessWindowHint);

    // Get Application Path
    QString temp = QCoreApplication::applicationDirPath();
    m_sApplicationPath.append(temp);

	// Connect Signals
    connect(this,SIGNAL(loadFinished(bool)),this,SLOT(OnLoad(bool)));

    // Read Main ini file
    QSettings MainIniFile(m_sApplicationPath + "/MainSet.ini",QSettings::IniFormat);
    QString sTheme = MainIniFile.value(QString("Configure/Theme")).toString();
    QString sThemeDir = MainIniFile.value(sTheme + "/Dir").toString();
    QString sUiDir = m_sApplicationPath + sThemeDir;
    qDebug("%s",sUiDir.toAscii().data());
    load("file:///" + sUiDir);

}


QJaWebView::~QJaWebView()
{
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

void QJaWebView::OnLoad( bool bOk )
{
	QWebFrame * MainFrame;
	MainFrame = page()->mainFrame();
	if (MainFrame->title() == "preview")
	{
        m_Activities[AI_PREVIEW]->Active(MainFrame);
	}
}
