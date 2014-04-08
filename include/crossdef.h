#ifndef CROSSDEF_H
#define CROSSDEF_H

#ifndef _WIN32
#define __stdcall
#define __cdecl
#endif



#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif
#endif



#endif // CROSSDEF_H
