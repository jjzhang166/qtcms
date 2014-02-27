#pragma once
#include <QThread>

class SettingsactivityThread:public QThread
{
	Q_OBJECT
public:
	SettingsactivityThread(void);
	~SettingsactivityThread(void);
public slots:
	void OnDeleteAllDev();
};

