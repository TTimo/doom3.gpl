#ifndef __NVCONTROL_H
#define __NVCONTROL_H

#define NV_CONTROL_ERRORS 0
#define NV_CONTROL_EVENTS 1
#define NV_CONTROL_NAME "NV-CONTROL"

#define NV_CONTROL_MAJOR 1
#define NV_CONTROL_MINOR 6

#define X_nvCtrlQueryExtension            0
#define X_nvCtrlIsNv                      1
#define X_nvCtrlQueryAttribute            2
#define X_nvCtrlSetAttribute              3
#define X_nvCtrlQueryStringAttribute      4
#define X_nvCtrlQueryValidAttributeValues 5
#define X_nvCtrlSelectNotify              6
#define X_nvCtrlLastRequest              (X_nvCtrlSelectNotify + 1)

typedef struct {
    CARD8 reqType;
    CARD8 nvReqType;
    CARD16 length B16;
} xnvCtrlQueryExtensionReq;
#define sz_xnvCtrlQueryExtensionReq 4

typedef struct {
    BYTE type;   /* X_Reply */
    CARD8 padb1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD16 major B16;
    CARD16 minor B16;
    CARD32 padl4 B32;
    CARD32 padl5 B32;
    CARD32 padl6 B32;
    CARD32 padl7 B32;
    CARD32 padl8 B32;
} xnvCtrlQueryExtensionReply;
#define sz_xnvCtrlQueryExtensionReply 32

typedef struct {
    CARD8 reqType;
    CARD8 nvReqType;
    CARD16 length B16;
    CARD32 screen B32;
} xnvCtrlIsNvReq;
#define sz_xnvCtrlIsNvReq 8

typedef struct {
    BYTE type;   /* X_Reply */
    CARD8 padb1;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD32 isnv B32;
    CARD32 padl4 B32;
    CARD32 padl5 B32;
    CARD32 padl6 B32;
    CARD32 padl7 B32;
    CARD32 padl8 B32;
} xnvCtrlIsNvReply;
#define sz_xnvCtrlIsNvReply 32

typedef struct {
    CARD8 reqType;
    CARD8 nvReqType;
    CARD16 length B16;
    CARD32 screen B32;
    CARD32 display_mask B32;
    CARD32 attribute B32;
} xnvCtrlQueryAttributeReq;
#define sz_xnvCtrlQueryAttributeReq 16

typedef struct {
    BYTE type;
    BYTE pad0;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD32 flags B32;
    INT32  value B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    CARD32 pad7 B32;
} xnvCtrlQueryAttributeReply;
#define sz_xnvCtrlQueryAttributeReply 32

typedef struct {
    CARD8 reqType;
    CARD8 nvReqType;
    CARD16 length B16;
    CARD32 screen B32;
    CARD32 display_mask B32;
    CARD32 attribute B32;
    INT32 value B32;
} xnvCtrlSetAttributeReq;
#define sz_xnvCtrlSetAttributeReq 20

typedef struct {
    CARD8 reqType;
    CARD8 nvReqType;
    CARD16 length B16;
    CARD32 screen B32;
    CARD32 display_mask B32;
    CARD32 attribute B32;
} xnvCtrlQueryStringAttributeReq;
#define sz_xnvCtrlQueryStringAttributeReq 16

/*
 * CtrlQueryStringAttribute reply struct
 * n indicates the length of the string.
 */
typedef struct {
    BYTE type;
    BYTE pad0;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD32 flags B32;
    CARD32 n B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    CARD32 pad7 B32;
} xnvCtrlQueryStringAttributeReply;
#define sz_xnvCtrlQueryStringAttributeReply 32

typedef struct {
    CARD8 reqType;
    CARD8 nvReqType;
    CARD16 length B16;
    CARD32 screen B32;
    CARD32 display_mask B32;
    CARD32 attribute B32;
} xnvCtrlQueryValidAttributeValuesReq;
#define sz_xnvCtrlQueryValidAttributeValuesReq 16

typedef struct {
    BYTE type;
    BYTE pad0;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD32 flags B32;
    INT32  attr_type B32;
    INT32  min B32;
    INT32  max B32;
    CARD32 bits B32;
    CARD32 perms B32;
} xnvCtrlQueryValidAttributeValuesReply;
#define sz_xnvCtrlQueryValidAttributeValuesReply 32

typedef struct {
    CARD8 reqType;
    CARD8 nvReqType;
    CARD16 length B16;
    CARD32 screen B32;
    CARD16 notifyType B16;
    CARD16 onoff B16;
} xnvCtrlSelectNotifyReq;
#define sz_xnvCtrlSelectNotifyReq 12

typedef struct {
    union {
        struct {
            BYTE type;
            BYTE detail;
            CARD16 sequenceNumber B16;
        } u;
        struct {
            BYTE type;
            BYTE detail;
            CARD16 sequenceNumber B16;
            Time time B32;
            CARD32 screen B32;
            CARD32 display_mask B32;
            CARD32 attribute B32;
            CARD32 value B32;
            CARD32 pad0 B32;
            CARD32 pad1 B32;
        } attribute_changed;
    } u;
} xnvctrlEvent;


#endif /* __NVCONTROL_H */
