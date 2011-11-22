#ifndef __NVCTRLLIB_H
#define __NVCTRLLIB_H

#include "NVCtrl.h"

/*
 *  XNVCTRLQueryExtension -
 *
 *  Returns True if the extension exists, returns False otherwise.
 *  event_basep and error_basep are the extension event and error
 *  bases.  Currently, no extension specific errors or events are
 *  defined.
 */

Bool XNVCTRLQueryExtension (
    Display *dpy,
    int *event_basep,
    int *error_basep
);

/*
 *  XNVCTRLQueryVersion -
 *
 *  Returns True if the extension exists, returns False otherwise.
 *  major and minor are the extension's major and minor version
 *  numbers.
 */

Bool XNVCTRLQueryVersion (
    Display *dpy,
    int *major,
    int *minor
);


/*
 *  XNVCTRLIsNvScreen
 *
 *  Returns True is the specified screen is controlled by the NVIDIA
 *  driver.  Returns False otherwise.
 */

Bool XNVCTRLIsNvScreen (
    Display *dpy,
    int screen
);

/*
 *  XNVCTRLSetAttribute -
 *
 *  Sets the attribute to the given value.  The attributes and their
 *  possible values are listed in NVCtrl.h.
 *
 *  Not all attributes require the display_mask parameter; see
 *  NVCtrl.h for details.
 *
 *  Possible errors:
 *     BadValue - The screen or attribute doesn't exist.
 *     BadMatch - The NVIDIA driver is not present on that screen.
 */

void XNVCTRLSetAttribute (
    Display *dpy,
    int screen,
    unsigned int display_mask,
    unsigned int attribute,
    int value
);

/*
 *  XNVCTRLQueryAttribute -
 *
 *  Returns True if the attribute exists.  Returns False otherwise.
 *  If XNVCTRLQueryAttribute returns True, value will contain the
 *  value of the specified attribute.
 *
 *  Not all attributes require the display_mask parameter; see
 *  NVCtrl.h for details.
 *
 *  Possible errors:
 *     BadValue - The screen doesn't exist.
 *     BadMatch - The NVIDIA driver is not present on that screen.
 */


Bool XNVCTRLQueryAttribute (
    Display *dpy,
    int screen,
    unsigned int display_mask,
    unsigned int attribute,
    int *value
);

/*
 *  XNVCTRLQueryStringAttribute -
 *
 *  Returns True if the attribute exists.  Returns False otherwise.
 *  If XNVCTRLQueryStringAttribute returns True, *ptr will point to an
 *  allocated string containing the string attribute requested.  It is
 *  the caller's responsibility to free the string when done.
 *
 *  Possible errors:
 *     BadValue - The screen doesn't exist.
 *     BadMatch - The NVIDIA driver is not present on that screen.
 *     BadAlloc - Insufficient resources to fulfill the request.
 */

Bool XNVCTRLQueryStringAttribute (
    Display *dpy,
    int screen,
    unsigned int display_mask,
    unsigned int attribute,
    char **ptr
);

/*
 * XNVCTRLQueryValidAttributeValues -
 *
 * Returns True if the attribute exists.  Returns False otherwise.  If
 * XNVCTRLQueryValidAttributeValues returns True, values will indicate
 * the valid values for the specified attribute; see the description
 * of NVCTRLAttributeValidValues in NVCtrl.h.
 */

Bool XNVCTRLQueryValidAttributeValues (
    Display *dpy,
    int screen,
    unsigned int display_mask,
    unsigned int attribute,                                 
    NVCTRLAttributeValidValuesRec *values
);

/*
 * XNVCtrlSelectNotify -
 *
 * This enables/disables receiving of NV-CONTROL events.  The type
 * specifies the type of event to enable (currently, the only type is
 * ATTRIBUTE_CHANGED_EVENT); onoff controls whether receiving this
 * type of event should be enabled (True) or disabled (False).
 *
 * Returns True if successful, or False if the screen is not
 * controlled by the NVIDIA driver.
 */

Bool XNVCtrlSelectNotify (
    Display *dpy,
    int screen,
    int type,
    Bool onoff
);



/*
 * XNVCtrlEvent structure
 */

typedef struct {
    int type;
    unsigned long serial;
    Bool send_event;  /* always FALSE, we don't allow send_events */
    Display *display;
    Time time;
    int screen;
    unsigned int display_mask;
    unsigned int attribute;
    int value;
} XNVCtrlAttributeChangedEvent;

typedef union {
    int type;
    XNVCtrlAttributeChangedEvent attribute_changed;
    long pad[24];
} XNVCtrlEvent;


#endif /* __NVCTRLLIB_H */
