#include <libpcom.h>
#include <QtXml>

bool operator==(const GUID &guid1,const GUID &guid2)
{
    return !memcmp((void*)&guid1,(void*)&guid2,sizeof(GUID));
}


long pcomCreateInstance(const CLSID &clsid, IPcomBase *pBase, const IID &iid, void **ppv)
{
    long lRet = S_OK;
    QString sReqClsid;
	sReqClsid.append(QString(pcomGUID2String(clsid)));
    QString sAppPath = QCoreApplication::applicationDirPath();
    QDomDocument ConfFile;
    QFile *file = new QFile(sAppPath + "/pcom_config.xml");
    file->open(QIODevice::ReadOnly);
    ConfFile.setContent(file);

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
            QString sModulePath = sAppPath + QString("/") + sFileName;
            QLibrary Module(sModulePath);

            typedef IPcomBase * (*lpCreateInstance)();
            lpCreateInstance pCreateInstance = (lpCreateInstance)Module.resolve("CreateInstance");
            IPcomBase * pInstance = pCreateInstance();
            if (NULL != pBase)
            {
                pBase = pInstance = pCreateInstance();
            }
            lRet = pInstance->QueryInterface(iid,ppv);
			pInstance->Release();

            bFound = true;
            break;
        }
    }

    file->close();
    delete file;

    if (!bFound){
        lRet = E_NOINTERFACE;
    }

    return lRet;
}


char * pcomGUID2String(const GUID &guid)
{
	static char sRet[64] = {0};
    QString sGuid;
    sGuid.sprintf("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                  guid.Data1,
                  guid.Data2,
                  guid.Data3,
                  *((unsigned char *)guid.Data4),
				  *((unsigned char *)(guid.Data4 + 1)),
                  *((unsigned char *)(guid.Data4 + 2)),
				  *((unsigned char *)(guid.Data4 + 3)),
                  *((unsigned char *)(guid.Data4 + 4)),
				  *((unsigned char *)(guid.Data4 + 5)),
				  *((unsigned char *)(guid.Data4 + 6)),
				  *((unsigned char *)(guid.Data4 + 7))
                  );
	strcpy(sRet,sGuid.toAscii().data());
    return sRet;
}

GUID pcomString2GUID(const QString &sGuid)
{
	GUID ret;
	QStringList guidData = sGuid.split(QChar('-'));
	ret.Data1 = guidData.at(0).toULong((bool *)0,16);
	ret.Data2 = guidData.at(1).toUShort((bool *)0,16);
	ret.Data3 = guidData.at(2).toUShort((bool *)0,16);
	*((unsigned char *)(ret.Data4)) = guidData.at(3).left(2).toUShort(NULL,16);
	*((unsigned char *)(ret.Data4 + 1)) = guidData.at(3).right(2).toUShort(NULL,16);
	*((unsigned char *)(ret.Data4 + 2)) = guidData.at(4).left(2).toUShort(NULL,16);
	*((unsigned char *)(ret.Data4 + 3)) = guidData.at(4).mid(2,2).toUShort(NULL,16);
	*((unsigned char *)(ret.Data4 + 4)) = guidData.at(4).mid(4,2).toUShort(NULL,16);
	*((unsigned char *)(ret.Data4 + 5)) = guidData.at(4).mid(6,2).toUShort(NULL,16);
	*((unsigned char *)(ret.Data4 + 6)) = guidData.at(4).mid(8,2).toUShort(NULL,16);
	*((unsigned char *)(ret.Data4 + 7)) = guidData.at(4).mid(10,2).toUShort(NULL,16);
	return ret;
}
