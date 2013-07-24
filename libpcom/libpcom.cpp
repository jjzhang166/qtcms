#include "libpcom.h"


bool operator==(const GUID &guid1,const GUID &guid2)
{
    return !memcpy((void*)&guid1,(void*)&guid2,sizeof(GUID));
}


HRESULT pcomCreateInstance(const CLSID &clsid, IPcomBase *pBase, const IID &iid, void **ppv)
{
    // 从配置文件中获取库信息
    return 0;
}
