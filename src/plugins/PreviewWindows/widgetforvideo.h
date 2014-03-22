#pragma once
#include <QWidget>
#include <QPushButton>
class widgetforvideo:public QWidget
{
public:
	widgetforvideo(QWidget *parent=0);
	~widgetforvideo();
private:
	void paintEvent(QPaintEvent *aEvent);
	bool btest;
};

