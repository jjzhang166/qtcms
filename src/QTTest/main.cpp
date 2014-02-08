#include <QApplication>
#include <QtCore/QCoreApplication>
#include "qjawebview.h"
#include "libpcom.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	// Get Application Path
	QString sTemp = QCoreApplication::applicationDirPath();
	// extern libraries path
	QString sExternLib(sTemp + "/exlibs");
	QApplication::addLibraryPath(sExternLib);
	QJaWebView view;
    view.showMaximized();
    return a.exec();
}
