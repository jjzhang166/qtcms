#ifndef QPREVIEWWINDOWS_H
#define QPREVIEWWINDOWS_H

#include <QWidget>
#include "qsubview.h"

class QPreviewWindows : public QWidget
{
	Q_OBJECT

public:
	QPreviewWindows(QWidget *parent = 0);
	~QPreviewWindows();

	virtual void resizeEvent( QResizeEvent * );
private:
	QSubView * m_PreviewWnd[4];
};

#endif // QPREVIEWWINDOWS_H
