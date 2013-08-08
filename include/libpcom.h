#ifndef LIBPCOM_H
#define LIBPCOM_H

#include "libpcom_global.h"

// GUID
#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID
{
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
} GUID;
#endif
typedef GUID CLSID;
typedef GUID IID;
typedef GUID UUID;

#define interface struct

interface IPcomBase{
    virtual long __stdcall QueryInterface(const IID & iid,void **ppv) = 0;
    virtual unsigned long __stdcall AddRef() = 0;
    virtual unsigned long __stdcall Release() = 0;
};

typedef interface IPcomBase IPcomBase,*pIPcomBase;

// Error code
#define MAKE_ERROR(x)   (0x80000000 | x)
#define S_OK 0
#define E_NOINTERFACE MAKE_ERROR(1)


// operators
LIBPCOMSHARED_EXPORT bool operator==(const GUID &guid1,const GUID &guid2);

#ifdef __cplusplus
extern "C"{
#endif

// functions
LIBPCOMSHARED_EXPORT char * pcomGUID2String(const GUID &guid);

LIBPCOMSHARED_EXPORT long pcomCreateInstance(const CLSID &clsid,IPcomBase *pBase,const IID &iid,void ** ppv);

LIBPCOMSHARED_EXPORT GUID pcomString2GUID(const QString &sGuid);

#ifdef __cplusplus
};
#endif

#endif // LIBPCOM_H
