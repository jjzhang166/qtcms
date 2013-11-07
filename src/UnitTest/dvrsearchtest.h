#ifndef DVRSEARCHTEST_H
#define DVRSEARCHTEST_H

#include <QObject>
#include <QString>
#include <QtTest>

class DvrSearchTest : public QObject
{
	Q_OBJECT

public:
	DvrSearchTest();
	~DvrSearchTest();

private:
	static int cbDeviceFoundTest(QString,QVariantMap,void *);
    static int cbDeviceSetTest(QString,QVariantMap,void *);


	private Q_SLOTS:
  		void DvrSearchCase1();
        void DvrSearchCase2();
        void DvrSearchCase3();
        void DvrSearchCase4();
        void DvrSearchCase5();
        void DvrSearchCase6();
        void DvrSearchCase7();
};

#endif // DVRSEARCHTEST_H
