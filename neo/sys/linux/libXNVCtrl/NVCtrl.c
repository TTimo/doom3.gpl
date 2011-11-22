#define NEED_EVENTS
#define NEED_REPLIES
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xext.h>
#include "extutil.h"
#include "NVCtrlLib.h"
#include "nv_control.h"

#if !defined(XTRHEADS)
#warning XTRHEADS not defined -- this libXNVCtrl.a will not be thread safe!
#endif

static XExtensionInfo _nvctrl_ext_info_data;
static XExtensionInfo *nvctrl_ext_info = &_nvctrl_ext_info_data;
static /* const */ char *nvctrl_extension_name = NV_CONTROL_NAME;

#define XNVCTRLCheckExtension(dpy,i,val) \
  XextCheckExtension (dpy, i, nvctrl_extension_name, val)
#define XNVCTRLSimpleCheckExtension(dpy,i) \
  XextSimpleCheckExtension (dpy, i, nvctrl_extension_name)

static int close_display();
static Bool wire_to_event();
static /* const */ XExtensionHooks nvctrl_extension_hooks = {
    NULL,                               /* create_gc */
    NULL,                               /* copy_gc */
    NULL,                               /* flush_gc */
    NULL,                               /* free_gc */
    NULL,                               /* create_font */
    NULL,                               /* free_font */
    close_display,                      /* close_display */
    wire_to_event,                      /* wire_to_event */
    NULL,                               /* event_to_wire */
    NULL,                               /* error */
    NULL,                               /* error_string */
};

static XEXT_GENERATE_FIND_DISPLAY (find_display, nvctrl_ext_info,
                                   nvctrl_extension_name, 
                                   &nvctrl_extension_hooks,
                                   NV_CONTROL_EVENTS, NULL)

static XEXT_GENERATE_CLOSE_DISPLAY (close_display, nvctrl_ext_info)

Bool XNVCTRLQueryExtension (
    Display *dpy,
    int *event_basep,
    int *error_basep
){
    XExtDisplayInfo *info = find_display (dpy);

    if (XextHasExtension(info)) {
        if (event_basep) *event_basep = info->codes->first_event;
        if (error_basep) *error_basep = info->codes->first_error;
        return True;
    } else {
        return False;
    }
}


Bool XNVCTRLQueryVersion (
    Display *dpy,
    int *major,
    int *minor
){
    XExtDisplayInfo *info = find_display (dpy);
    xnvCtrlQueryExtensionReply rep;
    xnvCtrlQueryExtensionReq   *req;

    if(!XextHasExtension(info))
        return False;

    XNVCTRLCheckExtension (dpy, info, False);

    LockDisplay (dpy);
    GetReq (nvCtrlQueryExtension, req);
    req->reqType = info->codes->major_opcode;
    req->nvReqType = X_nvCtrlQueryExtension;
    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
        UnlockDisplay (dpy);
        SyncHandle ();
        return False;
    }
    if (major) *major = rep.major;
    if (minor) *minor = rep.minor;
    UnlockDisplay (dpy);
    SyncHandle ();
    return True;
}


Bool XNVCTRLIsNvScreen (
    Display *dpy,
    int screen
){
    XExtDisplayInfo *info = find_display (dpy);
    xnvCtrlIsNvReply rep;
    xnvCtrlIsNvReq   *req;
    Bool isnv;

    if(!XextHasExtension(info))
        return False;

    XNVCTRLCheckExtension (dpy, info, False);

    LockDisplay (dpy);
    GetReq (nvCtrlIsNv, req);
    req->reqType = info->codes->major_opcode;
    req->nvReqType = X_nvCtrlIsNv;
    req->screen = screen;
    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
        UnlockDisplay (dpy);
        SyncHandle ();
        return False;
    }
    isnv = rep.isnv;
    UnlockDisplay (dpy);
    SyncHandle ();
    return isnv;
}


void XNVCTRLSetAttribute (
    Display *dpy,
    int screen,
    unsigned int display_mask,
    unsigned int attribute,
    int value
){
    XExtDisplayInfo *info = find_display (dpy);
    xnvCtrlSetAttributeReq *req;

    XNVCTRLSimpleCheckExtension (dpy, info);

    LockDisplay (dpy);
    GetReq (nvCtrlSetAttribute, req);
    req->reqType = info->codes->major_opcode;
    req->nvReqType = X_nvCtrlSetAttribute;
    req->screen = screen;
    req->display_mask = display_mask;
    req->attribute = attribute;
    req->value = value;
    UnlockDisplay (dpy);
    SyncHandle ();
}


Bool XNVCTRLQueryAttribute (
    Display *dpy,
    int screen,
    unsigned int display_mask,
    unsigned int attribute,
    int *value
){
    XExtDisplayInfo *info = find_display (dpy);
    xnvCtrlQueryAttributeReply rep;
    xnvCtrlQueryAttributeReq   *req;
    Bool exists;

    if(!XextHasExtension(info))
        return False;

    XNVCTRLCheckExtension (dpy, info, False);

    LockDisplay (dpy);
    GetReq (nvCtrlQueryAttribute, req);
    req->reqType = info->codes->major_opcode;
    req->nvReqType = X_nvCtrlQueryAttribute;
    req->screen = screen;
    req->display_mask = display_mask;
    req->attribute = attribute;
    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
        UnlockDisplay (dpy);
        SyncHandle ();
        return False;
    }
    if (value) *value = rep.value;
    exists = rep.flags;
    UnlockDisplay (dpy);
    SyncHandle ();
    return exists;
}


Bool XNVCTRLQueryStringAttribute (
    Display *dpy,
    int screen,
    unsigned int display_mask,
    unsigned int attribute,
    char **ptr
){
    XExtDisplayInfo *info = find_display (dpy);
    xnvCtrlQueryStringAttributeReply rep;
    xnvCtrlQueryStringAttributeReq   *req;
    Bool exists;
    int length, numbytes, slop;

    if (!ptr) return False;

    if(!XextHasExtension(info))
        return False;

    XNVCTRLCheckExtension (dpy, info, False);

    LockDisplay (dpy);
    GetReq (nvCtrlQueryStringAttribute, req);
    req->reqType = info->codes->major_opcode;
    req->nvReqType = X_nvCtrlQueryStringAttribute;
    req->screen = screen;
    req->display_mask = display_mask;
    req->attribute = attribute;
    if (!_XReply (dpy, (xReply *) &rep, 0, False)) {
        UnlockDisplay (dpy);
        SyncHandle ();
        return False;
    }
    length = rep.length;
    numbytes = rep.n;
    slop = numbytes & 3;
    *ptr = (char *) Xmalloc(numbytes);
    if (! *ptr) {
        _XEatData(dpy, length);
        UnlockDisplay (dpy);
        SyncHandle ();
        return False;
    } else {
        _XRead(dpy, (char *) *ptr, numbytes);
        if (slop) _XEatData(dpy, 4-slop);
    }
    exists = rep.flags;
    UnlockDisplay (dpy);
    SyncHandle ();
    return exists;
}

Bool XNVCTRLQueryValidAttributeValues (
    Display *dpy,
    int screen,
    unsigned int display_mask,
    unsigned int attribute,                                 
    NVCTRLAttributeValidValuesRec *values
){
    XExtDisplayInfo *info = find_display (dpy);
    xnvCtrlQueryValidAttributeValuesReply rep;
    xnvCtrlQueryValidAttributeValuesReq   *req;
    Bool exists;
    
    if (!values) return False;

    if(!XextHasExtension(info))
        return False;

    XNVCTRLCheckExtension (dpy, info, False);

    LockDisplay (dpy);
    GetReq (nvCtrlQueryValidAttributeValues, req);
    req->reqType = info->codes->major_opcode;
    req->nvReqType = X_nvCtrlQueryValidAttributeValues;
    req->screen = screen;
    req->display_mask = display_mask;
    req->attribute = attribute;
    if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
        UnlockDisplay (dpy);
        SyncHandle ();
        return False;
    }
    exists = rep.flags;
    values->type = rep.attr_type;
    if (rep.attr_type == ATTRIBUTE_TYPE_RANGE) {
        values->u.range.min = rep.min;
        values->u.range.max = rep.max;
    }
    if (rep.attr_type == ATTRIBUTE_TYPE_INT_BITS) {
        values->u.bits.ints = rep.bits;
    }
    values->permissions = rep.perms;
    UnlockDisplay (dpy);
    SyncHandle ();
    return exists;
}

Bool XNVCtrlSelectNotify (
    Display *dpy,
    int screen,
    int type,
    Bool onoff
){
    XExtDisplayInfo *info = find_display (dpy);
    xnvCtrlSelectNotifyReq *req;

    if(!XextHasExtension (info))
        return False;

    XNVCTRLCheckExtension (dpy, info, False);

    LockDisplay (dpy);
    GetReq (nvCtrlSelectNotify, req);
    req->reqType = info->codes->major_opcode;
    req->nvReqType = X_nvCtrlSelectNotify;
    req->screen = screen;
    req->notifyType = type;
    req->onoff = onoff;
    UnlockDisplay (dpy);
    SyncHandle ();

    return True;
}

static Bool wire_to_event (Display *dpy, XEvent *host, xEvent *wire)
{
    XExtDisplayInfo *info = find_display (dpy);
    XNVCtrlEvent *re = (XNVCtrlEvent *) host;
    xnvctrlEvent *event = (xnvctrlEvent *) wire;

    XNVCTRLCheckExtension (dpy, info, False);
    
    switch ((event->u.u.type & 0x7F) - info->codes->first_event) {
    case ATTRIBUTE_CHANGED_EVENT:
        re->attribute_changed.type = event->u.u.type & 0x7F;
        re->attribute_changed.serial = 
            _XSetLastRequestRead(dpy, (xGenericReply*) event);
        re->attribute_changed.send_event = ((event->u.u.type & 0x80) != 0);
        re->attribute_changed.display = dpy;
        re->attribute_changed.time = event->u.attribute_changed.time;
        re->attribute_changed.screen = event->u.attribute_changed.screen;
        re->attribute_changed.display_mask =
            event->u.attribute_changed.display_mask;
        re->attribute_changed.attribute = event->u.attribute_changed.attribute;
        re->attribute_changed.value = event->u.attribute_changed.value;
        break;
    default:
        return False;
    }
    
    return True;
}

