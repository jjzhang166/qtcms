#include "libpcom.h"
#include <QtXml>

bool operator==(const GUID &guid1,const GUID &guid2)
{
    return !memcpy((void*)&guid1,(void*)&guid2,sizeof(GUID));
}


long pcomCreateInstance(const CLSID &clsid, IPcomBase *pBase, const IID &iid, void **ppv)
{
    QString sReqClsid = pcomGUID2String(clsid);
    QString sAppPath = QCoreApplication::applicationDirPath();
    QDomDocument ConfFile;
    QFile *file = new QFile(sAppPath + "/pcom_config.xml");
    bool bOpened = file->open(QIODevice::ReadOnly);
    qDebug("bOpened:%d",bOpened);
    bool bSetContent = ConfFile.setContent(file);
    qDebug("bSetContent:%d",bSetContent);

    QDomNode clsidNode = ConfFile.elementsByTagName("CLSID").at(0);
    QDomNodeList itemList = clsidNode.childNodes();
    int n;
    bool bFound = false;
    for (n = 0; n < itemList.count(); n++)
    {
        QDomNode item = itemList.at(n);
        QString sClsid = item.toElement().attribute("clsid");
        if(sReqClsid == sClsid){
            QString sFileName =item.toElement().attribute("file");
            QString sModulePath = sAppPath + sFileName;
            QLibrary Module(sModulePath);
            long (*pCreateInstance)(IPcomBase **ppCom);
            Module.resolve("CreateInstance");
            bFound = true;
            break;
        }
    }

    file->close();
    delete file;

    if (!bFound){
        return E_NOINTERFACE;
    }

    return S_OK;
}


char *pcomGUID2String(const GUID &guid)
{
    QString sGuid;
    sGuid.sprintf("%08X-%04X-%04X-%04X-%04X08X",
                  guid.Data1,
                  guid.Data2,
                  guid.Data3,
                  *((unsigned short *)guid.Data4),
                  *((unsigned short *)(guid.Data4 + 2)),
                  *((unsigned short *)(guid.Data4 + 4))
                  );
    return sGuid.toAscii().data();
}
