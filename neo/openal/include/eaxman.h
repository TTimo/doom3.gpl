/*
*/
#ifndef __EAXMANH
#define __EAXMANH

#define COM_NO_WINDOWS_H
#include <objbase.h>
#include "eax3.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//#define CLSID_EAXMANAGER CLSID_EAX20_Manager
//#define IID_IEaxManager IID_EAX20_Manager
#define EM_MAX_NAME 32

#define EMFLAG_IDDEFAULT (-1)
#define EMFLAG_IDNONE (-2)
#define EMFLAG_LOCKPOSITION 1
#define EMFLAG_LOADFROMMEMORY 2
#define EMFLAG_NODIFFRACTION 4

typedef struct _EMPOINT {
 float fX;
 float fY;
 float fZ;
} EMPOINT;
typedef EMPOINT FAR *LPEMPOINT;

typedef struct _LISTENERATTRIBUTES {
 float fDistanceFactor;
 float fRolloffFactor;
 float fDopplerFactor;
} LISTENERATTRIBUTES;
typedef LISTENERATTRIBUTES FAR *LPLISTENERATTRIBUTES;

typedef struct _SOURCEATTRIBUTES {
 EAXBUFFERPROPERTIES eaxAttributes;
 unsigned long       ulInsideConeAngle;
 unsigned long       ulOutsideConeAngle;
 long                lConeOutsideVolume;
 float               fConeXdir;
 float               fConeYdir;
 float               fConeZdir;
 float               fMinDistance;
 float               fMaxDistance;
 long                lDupCount;
 long                lPriority;
} SOURCEATTRIBUTES; 
typedef SOURCEATTRIBUTES FAR *LPSOURCEATTRIBUTES;

typedef struct _MATERIALATTRIBUTES {
 long  lLevel;
 float fLFRatio;
 float fRoomRatio;
 DWORD dwFlags;
} MATERIALATTRIBUTES;
typedef MATERIALATTRIBUTES FAR *LPMATERIALATTRIBUTES;

#define EMMATERIAL_OBSTRUCTS 1
#define EMMATERIAL_OCCLUDES 3

typedef struct _DIFFRACTIONBOX {
 long    lSubspaceID;
 EMPOINT empMin;
 EMPOINT empMax;
} DIFFRACTIONBOX;
typedef DIFFRACTIONBOX FAR *LPDIFFRACTIONBOX;

// {7CE4D6E6-562F-11d3-8812-005004062F83}
DEFINE_GUID(CLSID_EAXMANAGER, 0x60b721a1, 0xf7c8, 0x11d2, 0xa0, 0x2e, 0x0, 0x50, 0x4, 0x6, 0x18, 0xb8);

#ifdef __cplusplus
struct IEaxManager;
#endif // __cplusplus

typedef struct IEaxManager *LPEAXMANAGER;

// {7CE4D6E8-562F-11d3-8812-005004062F83}
DEFINE_GUID(IID_IEaxManager, 0x60b721a2, 0xf7c8, 0x11d2, 0xa0, 0x2e, 0x0, 0x50, 0x4, 0x6, 0x18, 0xb8);

#undef INTERFACE
#define INTERFACE IEaxManager

extern HRESULT __stdcall EaxManagerCreate(LPEAXMANAGER*);
typedef HRESULT (__stdcall *LPEAXMANAGERCREATE)(LPEAXMANAGER*);

DECLARE_INTERFACE_(IEaxManager, IUnknown)
{
 // IUnknown methods
 STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID *) PURE;
 STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
 STDMETHOD_(ULONG,Release)       (THIS) PURE;

 STDMETHOD(GetDataSetSize) (THIS_ unsigned long*, DWORD) PURE;
 STDMETHOD(LoadDataSet) (THIS_ char*, DWORD) PURE;
 STDMETHOD(FreeDataSet) (THIS_ DWORD) PURE;
 STDMETHOD(GetListenerAttributes) (THIS_ LPLISTENERATTRIBUTES) PURE;
 STDMETHOD(GetSourceID) (THIS_ char*, long*) PURE;
 STDMETHOD(GetSourceAttributes) (THIS_ long, LPSOURCEATTRIBUTES) PURE;
 STDMETHOD(GetSourceNumInstances) (THIS_ long, long*) PURE;
 STDMETHOD(GetSourceInstancePos) (THIS_ long, long, LPEMPOINT) PURE;
 STDMETHOD(GetEnvironmentID) (THIS_ char*, long*) PURE;
 STDMETHOD(GetEnvironmentAttributes) (THIS_ long, LPEAXLISTENERPROPERTIES) PURE;
 STDMETHOD(GetMaterialID) (THIS_ char*, long*) PURE;
 STDMETHOD(GetMaterialAttributes) (THIS_ long, LPMATERIALATTRIBUTES) PURE;
 STDMETHOD(GetGeometrySetID) (THIS_ char*, long*) PURE;
 STDMETHOD(GetListenerDynamicAttributes) (THIS_ long, LPEMPOINT, long*, DWORD) PURE;
 STDMETHOD(GetSourceDynamicAttributes) (THIS_ long, LPEMPOINT, long*, float*, long*, float*, float*, LPEMPOINT, DWORD) PURE;
// STDMETHOD(GetSubSpaceID) (THIS_ long, LPEMPOINT, long *) PURE;
 STDMETHOD(GetEnvironmentName) (THIS_ long, char *szString, long lStrlen) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IEaxManager_QueryInterface(p,a,b)                           (p)->lpVtbl->QueryInterface(p,a,b)
#define IEaxManager_AddRef(p)                                       (p)->lpVtbl->AddRef(p)
#define IEaxManager_Release(p)                                      (p)->lpVtbl->Release(p)
#define IEaxManager_GetDataSetSize(p,a,b)                           (p)->lpVtbl->GetDataSetSize(p,a,b)
#define IEaxManager_LoadDataSet(p,a,b)                              (p)->lpVtbl->LoadDataSet(p,a,b)
#define IEaxManager_FreeDataSet(p,a)                                (p)->lpVtbl->FreeDataSet(p,a)
#define IEaxManager_GetListenerAttributes(p,a)                      (p)->lpVtbl->GetListenerAttributes(p,a)
#define IEaxManager_GetSourceID(p,a,b)                              (p)->lpVtbl->GetSourceID(p,a,b)
#define IEaxManager_GetSourceAttributes(p,a,b)                      (p)->lpVtbl->GetSourceAttributes(p,a,b)
#define IEaxManager_GetSourceNumInstances(p,a,b)                    (p)->lpVtbl->GetSourceNumInstances(p,a,b)
#define IEaxManager_GetSourceInstancePos(p,a,b,c)                   (p)->lpVtbl->GetSourceInstancePos(p,a,b,c)
#define IEaxManager_GetEnvironmentID(p,a,b)                         (p)->lpVtbl->GetEnvironmentID(p,a,b)
#define IEaxManager_GetEnvironmentAttributes(p,a,b)                 (p)->lpVtbl->GetEnvironmentAttributes(p,a,b)
#define IEaxManager_GetMaterialID(p,a,b)                            (p)->lpVtbl->GetMaterialID(p,a,b)
#define IEaxManager_GetMaterialAttributes(p,a,b)                    (p)->lpVtbl->GetMaterialAttributes(p,a,b)
#define IEaxManager_GetGeometrySetID(p,a,b)                         (p)->lpVtbl->GetGeometrySetID(p,a,b)
#define IEaxManager_GetListenerDynamicAttributes(p,a,b,c,d)         (p)->lpVtbl->GetListenerDynamicAttributes(p,a,b,c,d)
#define IEaxManager_GetSourceDynamicAttributes(p,a,b,c,d,e,f,g,h,i) (p)->lpVtbl->GetSourceDynamicAttributes(p,a,b,c,d,e,f,g,h,i)
//#define IEaxManager_GetSubSpaceID(p,a,b,c)							(p)->lpVtbl->GetSubSpaceID(p,a,b,c)
#define IEaxManager_GetEnvironmentName(p,a,b,c)						(p)->lpVtbl->GetEnvironmentName(p,a,b,c)
#else
#define IEaxManager_QueryInterface(p,a,b)                           (p)->QueryInterface(a,b)
#define IEaxManager_AddRef(p)                                       (p)->AddRef()
#define IEaxManager_Release(p)                                      (p)->Release()
#define IEaxManager_GetDataSetSize(p,a,b)                           (p)->GetDataSetSize(a,b)
#define IEaxManager_LoadDataSet(p,a,b)                              (p)->LoadDataSet(a,b)
#define IEaxManager_FreeDataSet(p,a)                                (p)->FreeDataSet(a)
#define IEaxManager_GetListenerAttributes(p,a)                      (p)->GetListenerAttributes(a)
#define IEaxManager_GetSourceID(p,a,b)                              (p)->GetSourceID(a,b)
#define IEaxManager_GetSourceAttributes(p,a,b)                      (p)->GetSourceAttributes(a,b)
#define IEaxManager_GetSourceNumInstances(p,a,b)                    (p)->GetSourceNumInstances(a,b)
#define IEaxManager_GetSourceInstancePos(p,a,b,c)                   (p)->GetSourceInstancePos(a,b,c)
#define IEaxManager_GetEnvironmentID(p,a,b)                         (p)->GetEnvironmentID(a,b)
#define IEaxManager_GetEnvironmentAttributes(p,a,b)                 (p)->GetEnvironmentAttributes(a,b)
#define IEaxManager_GetMaterialID(p,a,b)                            (p)->GetMaterialID(a,b)
#define IEaxManager_GetMaterialAttributes(p,a,b)                    (p)->GetMaterialAttributes(a,b)
#define IEaxManager_GetGeometrySetID(p,a,b)                         (p)->GetGeometrySetID(a,b)
#define IEaxManager_GetListenerDynamicAttributes(p,a,b,c,d)         (p)->GetListenerDynamicAttributes(a,b,c,d)
#define IEaxManager_GetSourceDynamicAttributes(p,a,b,c,d,e,f,g,h,i) (p)->GetSourceDynamicAttributes(a,b,c,d,e,f,g,h,i)
//#define IEaxManager_GetSubSpaceID(p,a,b,c)							(p)->GetSubSpaceID(a,b,c)
#define IEaxManager_GetEnvironmentName(p,a,b,c)						(p)->GetEnvironmentName(a,b,c)
#endif

#define EM_OK 0
#define EM_INVALIDID        MAKE_HRESULT(1, FACILITY_ITF, 1)
#define EM_IDNOTFOUND       MAKE_HRESULT(1, FACILITY_ITF, 2)
#define EM_FILENOTFOUND     MAKE_HRESULT(1, FACILITY_ITF, 3)
#define EM_FILEINVALID      MAKE_HRESULT(1, FACILITY_ITF, 4)
#define EM_VERSIONINVALID   MAKE_HRESULT(1, FACILITY_ITF, 5)
#define EM_INSTANCENOTFOUND MAKE_HRESULT(1, FACILITY_ITF, 6)

#ifdef __cplusplus
};
#endif // __cplusplus

#endif
