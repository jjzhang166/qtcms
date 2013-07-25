#include <QApplication>
#include "qjawebview.h"
#include "libpcom.h"

// {B07F5819-E577-42F4-932B-84AC004D0C49}
static const GUID clsid_null =
{ 0xb07f5819, 0xe577, 0x42f4, { 0x93, 0x2b, 0x84, 0xac, 0x0, 0x4d, 0xc, 0x49 } };

static const GUID iid_test =
{ 0xb07f5819, 0xe577, 0x42f4, { 0x93, 0x2b, 0x84, 0xac, 0x0, 0x4d, 0xc2, 0x49 } };

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    pcomCreateInstance(clsid_null,NULL,iid_test,NULL);

    QJaWebView view;
    view.showMaximized();

    return a.exec();
}
