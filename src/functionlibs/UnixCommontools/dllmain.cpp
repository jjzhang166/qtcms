#include <libpcom.h>
#include "unixcommontools.h"

EXTERN_C IPcomBase * CreateInstance()
{
    UnixCommontools * pInstance = new UnixCommontools;
    IPcomBase * pBase = static_cast<IPcomBase *>(pInstance);
    pBase->AddRef();
    return pInstance;
}
