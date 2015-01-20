#ifndef SUSPENSIONWND_H
#define SUSPENSIONWND_H

#include <QWidget>
#include <QPoint>
#include <QMouseEvent>
#include <QRect>
#include <QList>

typedef void (__cdecl *callbackFc)(QVariantMap info,void *pUser);

class SuspensionWnd : public QWidget
{
	Q_OBJECT

public:
	SuspensionWnd(QWidget *parent = 0);
	~SuspensionWnd();
	void addWnd(QWidget* pWnd);
	void setCbFunc(callbackFc pfunc, void* puser);

private:
	virtual void mousePressEvent(QMouseEvent *);
// 	virtual mouseMoveEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);
	virtual void closeEvent(QCloseEvent *ev);
private:
	QPoint m_pressPoint;
	QList<QWidget*> m_wndList;
	callbackFc m_cbFunc;
	void *m_puser;
};

#endif // SUSPENSIONWND_H
