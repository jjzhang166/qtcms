#ifndef REMOTEPBACKUPTEST_H
#define REMOTEPBACKUPTEST_H

#include <QObject>
#include <QString>
#include <QtTest>

class RemoteBackupTest : public QObject
{
	Q_OBJECT

public:
	RemoteBackupTest();
	~RemoteBackupTest();

private Q_SLOTS:
	void RemoteBackupCase1();
	void RemoteBackupCase2();
	void RemoteBackupCase3();
};

#endif // REMOTEPBACKUPTEST_H
