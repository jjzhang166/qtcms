#include "libpcom.h"
#include <QtXml>

bool operator==(const GUID &guid1,const GUID &guid2)
{
    return !memcpy((void*)&guid1,(void*)&guid2,sizeof(GUID));
}


HRESULT pcomCreateInstance(const CLSID &clsid, IPcomBase *pBase, const IID &iid, void **ppv)
{
    QDomDocument ConfFile;
    QFile *file = new QFile("pcom_config.xml");
    file->open(QIODevice::ReadOnly);
    ConfFile.setContent(file);

    QDomNode clsidNode = ConfFile.elementById("CLSID");

    file->close();
    delete file;
    return 0;
}
