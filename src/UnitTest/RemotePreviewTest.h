#ifndef REMOTEPREVIEWTEST_H
#define REMOTEPREVIEWTEST_H

#include <QObject>
#include <QString>
#include <QtTest>

class RemotePreviewTest : public QObject
{
	Q_OBJECT

public:
	RemotePreviewTest();
	~RemotePreviewTest();

private:
	static int cbLiveStream(QString,QVariantMap,void *);
    static int cbOther(QString,QVariantMap,void *);


	private Q_SLOTS:
		void RemotePreviewCase1();
		void RemotePreviewCase2();
		void RemotePreviewCase3();
		void RemotePreviewCase4();
		void RemotePreviewCase5();
		void RemotePreviewCase6();
		void RemotePreviewCase7();
		void RemotePreviewCase8();
};

#endif // REMOTEPREVIEWTEST_H
