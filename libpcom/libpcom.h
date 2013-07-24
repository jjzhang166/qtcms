#ifndef LIBPCOM_H
#define LIBPCOM_H

#include "libpcom_global.h"

// GUID
typedef struct _tagGUID
{
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
} GUID;
typedef GUID CLSID;
typedef GUID IID;
typedef GUID UUID;

// HRESULT
typedef unsigned long HRESULT;

// integer
typedef unsigned long ULONG;

#define interface struct

interface IPcomBase{
    virtual HRESULT __stdcall QueryInterface(const IID & iid,void **ppv) = 0;
    virtual ULONG __stdcall AddRef() = 0;
    virtual ULONG __stdcall Release() = 0;
};

typedef interface IPcomBase IPcomBase,*pIPcomBase;

// operators
LIBPCOMSHARED_EXPORT bool operator==(const GUID &guid1,const GUID &guid2);

#ifdef __cplusplus
extern "C"{
#endif

// functions
LIBPCOMSHARED_EXPORT HRESULT pcomCreateInstance(const CLSID &clsid,IPcomBase *pBase,const IID &iid,void ** ppv);

#ifdef __cplusplus
};
#endif

#endif // LIBPCOM_H
