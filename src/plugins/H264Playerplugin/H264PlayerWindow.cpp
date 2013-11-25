#include "H264PlayerWindow.h"
#include <QtGui/QFileDialog>


H264PlayerWindow::H264PlayerWindow(QWidget* parent)
	:QWidget(parent),
	QWebPluginFWBase(this)
{
}


H264PlayerWindow::~H264PlayerWindow(void)
{
	m_h264Player.Stop();
}


void H264PlayerWindow::Play()
{
	m_h264Player.Play();
}
void H264PlayerWindow::Pause()
{
	m_h264Player.Pause();
}
void H264PlayerWindow::Stop()
{
	m_h264Player.Stop();
}
void H264PlayerWindow::Open()
{
	QString filename = QFileDialog::getOpenFileName((QWidget*)this,"open file",".",tr("h264file(*.h264);;Allfile(*.*)"));
	m_h264Player.OpenFile(filename);
}
void H264PlayerWindow::Init()
{
	m_h264Player.InitSearver(static_cast<QWidget*>(this));
}