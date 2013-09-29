#include <QtGui/QApplication>
#include <QtTest/QTest>
#include "unittest.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	qDebug("%s",argv[0]);
	
	UnitTest tc;
	QTest::qExec(&tc, argc, argv);

	return 0;
}
