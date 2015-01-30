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
	void setDrawRect(QRect rect);
	void setOriginGeog(QRect rect);
	QWidget* getTopWnd();
	virtual void changeEvent(QEvent *);
signals:
	void sigClose();
private:
	virtual void mousePressEvent(QMouseEvent *);
 	virtual void mouseMoveEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);
	virtual void mouseDoubleClickEvent(QMouseEvent *);
	virtual void closeEvent(QCloseEvent *ev);
	virtual void paintEvent(QPaintEvent *);
	virtual bool event(QEvent *ev);
private:
	QPoint m_pressPoint;
	QList<QWidget*> m_wndList;
	callbackFc m_cbFunc;
	void *m_puser;
	bool m_posInRect;
	bool m_bMinimized;
	QRect m_drawRect;
	QPoint m_lastMovePos;
	QRect m_originRect;
};

#endif // SUSPENSIONWND_H
