#include <QApplication>
#include "qjawebview.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QJaWebView view;
    view.showMaximized();

    return a.exec();
}
