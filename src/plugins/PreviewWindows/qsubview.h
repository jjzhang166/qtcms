#ifndef QSUBVIEW_H
#define QSUBVIEW_H

#include <QWidget>
#include "ui_qsubview.h"

class QSubView : public QWidget
{
	Q_OBJECT

public:
	QSubView(QWidget *parent = 0);
	~QSubView();

	virtual void paintEvent( QPaintEvent * );
private:
	Ui::QSubView ui;
};

#endif // QSUBVIEW_H
