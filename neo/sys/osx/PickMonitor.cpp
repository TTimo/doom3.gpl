/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).  

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "../../idlib/precompiled.h"
#include <Carbon/Carbon.h>
#include "PickMonitor.h"

//====================================================================================
//	CONSTANTS
//====================================================================================

#define kMaxMonitors		16

//====================================================================================
//	TYPES
//====================================================================================

typedef struct
{
	GDHandle	device;
	Rect		origRect;
	Rect		scaledRect;
	int			isMain;
}
Monitor;


//====================================================================================
//	GLOBALS
//====================================================================================
static GDHandle sSelectedDevice;
static int sNumMonitors;
static Monitor sMonitors[kMaxMonitors];

static RGBColor rgbBlack = { 0x0000, 0x0000, 0x0000 };
static RGBColor rgbWhite = { 0xffff, 0xffff, 0xffff };
static RGBColor rgbGray  = { 0x5252, 0x8A8A, 0xCCCC };	// this is the blue used in the Displays control panel

//====================================================================================
//	MACROS
//====================================================================================

#undef PtInRect
#undef OffsetRect
#undef InsetRect
#undef EraseRect
#undef MoveTo
#undef LineTo


//====================================================================================
//	IMPLEMENTATION
//====================================================================================

//-----------------------------------------------------------------------------
//	SetupUserPaneProcs
//-----------------------------------------------------------------------------
// 	Call this to initialize the specified user pane control before displaying 
//	the dialog window. Pass NULL for any user pane procs you don't need to install.

OSErr SetupUserPaneProcs(	ControlRef inUserPane,
							ControlUserPaneDrawProcPtr inDrawProc, 
							ControlUserPaneHitTestProcPtr inHitTestProc,
							ControlUserPaneTrackingProcPtr inTrackingProc)
{
	OSErr	err = noErr;
	ControlUserPaneDrawUPP drawUPP;
	ControlUserPaneHitTestUPP hitTestUPP;
	ControlUserPaneTrackingUPP trackingUPP;
	
	if (0 == inUserPane) return paramErr;
	
	if (inDrawProc && noErr == err)
	{
		drawUPP = NewControlUserPaneDrawUPP(inDrawProc);

		if (0 == drawUPP)
			err = memFullErr;
		else
			err = SetControlData(	inUserPane,
									kControlEntireControl,
									kControlUserPaneDrawProcTag,
									sizeof(ControlUserPaneDrawUPP),
									(Ptr)&drawUPP);
	}
	if (inHitTestProc && noErr == err)
	{
		hitTestUPP = NewControlUserPaneHitTestUPP(inHitTestProc);

		if (0 == hitTestUPP)
			err = memFullErr;
		else
			err = SetControlData(	inUserPane,
									kControlEntireControl, 
									kControlUserPaneHitTestProcTag,
									sizeof(ControlUserPaneHitTestUPP),
									(Ptr)&hitTestUPP);
	}
	if (inTrackingProc && noErr == err)
	{
		trackingUPP = NewControlUserPaneTrackingUPP(inTrackingProc);
		
		if (0 == trackingUPP)
			err = memFullErr;
		else
			err = SetControlData(	inUserPane,
									kControlEntireControl, 
									kControlUserPaneTrackingProcTag,
									sizeof(ControlUserPaneTrackingUPP),
									(Ptr)&trackingUPP);
	}
	
	return err;
}


//-----------------------------------------------------------------------------
//	DisposeUserPaneProcs
//-----------------------------------------------------------------------------
// 	Call this to clean up when you're done with the specified user pane control.

OSErr DisposeUserPaneProcs(ControlRef inUserPane)
{	
	ControlUserPaneDrawUPP drawUPP;
	ControlUserPaneHitTestUPP hitTestUPP;
	ControlUserPaneTrackingUPP trackingUPP;
	Size actualSize;
	OSErr err;
	
	err = GetControlData(inUserPane, kControlEntireControl, kControlUserPaneDrawProcTag, sizeof(ControlUserPaneDrawUPP), (Ptr)&drawUPP, &actualSize);
	if (err == noErr) DisposeControlUserPaneDrawUPP(drawUPP);

	err = GetControlData(inUserPane, kControlEntireControl, kControlUserPaneHitTestProcTag, sizeof(ControlUserPaneHitTestUPP), (Ptr)&hitTestUPP, &actualSize);
	if (err == noErr) DisposeControlUserPaneHitTestUPP(hitTestUPP);

	err = GetControlData(inUserPane, kControlEntireControl, kControlUserPaneTrackingProcTag, sizeof(ControlUserPaneTrackingUPP), (Ptr)&trackingUPP, &actualSize);
	if (err == noErr) DisposeControlUserPaneTrackingUPP(trackingUPP);

	return noErr;
}

#pragma mark -

//-----------------------------------------------------------------------------
//	drawProc
//-----------------------------------------------------------------------------
//	Custom drawProc for our UserPane control.

static pascal void drawProc(ControlRef inControl, SInt16 inPart)
{
	#pragma unused(inControl, inPart)
	
	int i;
	RGBColor saveForeColor;
	RGBColor saveBackColor;
	PenState savePenState;

	GetForeColor(&saveForeColor);	
	GetBackColor(&saveBackColor);	
	GetPenState(&savePenState);

	RGBForeColor(&rgbBlack);
	RGBBackColor(&rgbWhite);
	PenNormal();
	
	for (i = 0; i < sNumMonitors; i++)
	{
		RGBForeColor(&rgbGray);
		PaintRect(&sMonitors[i].scaledRect);
		if (sMonitors[i].isMain)
		{
			Rect r = sMonitors[i].scaledRect;
			InsetRect(&r, 1, 1);
			r.bottom = r.top + 6;
			RGBForeColor(&rgbWhite);
			PaintRect(&r);
			RGBForeColor(&rgbBlack);
			PenSize(1,1);
			MoveTo(r.left, r.bottom);
			LineTo(r.right, r.bottom);
		}
		if (sMonitors[i].device == sSelectedDevice)
		{
			PenSize(3,3);
			RGBForeColor(&rgbBlack);
			FrameRect(&sMonitors[i].scaledRect);
		}
		else
		{
			PenSize(1,1);
			RGBForeColor(&rgbBlack);
			FrameRect(&sMonitors[i].scaledRect);
		}
	}
	
	// restore the original pen state and colors
	RGBForeColor(&saveForeColor);	
	RGBBackColor(&saveBackColor);	
	SetPenState(&savePenState);
}


//-----------------------------------------------------------------------------
//	hitTestProc
//-----------------------------------------------------------------------------
//	Custom hitTestProc for our UserPane control.
//	This allows FindControlUnderMouse() to locate our control, which allows
//	ModalDialog() to call TrackControl() or HandleControlClick() for our control.

static pascal ControlPartCode hitTestProc(ControlRef inControl, Point inWhere)
{
	// return a valid part code so HandleControlClick() will be called
	return kControlButtonPart;
}


//-----------------------------------------------------------------------------
//	trackingProc
//-----------------------------------------------------------------------------
//	Custom trackingProc for our UserPane control.
//	This won't be called for our control unless the kControlHandlesTracking feature
//	bit is specified when the userPane is created.

static pascal ControlPartCode trackingProc (
					ControlRef inControl,
					Point inStartPt,
					ControlActionUPP inActionProc)
{
	#pragma unused (inControl, inStartPt, inActionProc)
	int i;

	for (i = 0; i < sNumMonitors; i++)
	{
		if (PtInRect(inStartPt, &sMonitors[i].scaledRect))
		{
			if (sMonitors[i].device != sSelectedDevice)
			{
				sSelectedDevice = sMonitors[i].device;
				DrawOneControl(inControl);
			}
			break;
		}
	}
	
	return kControlNoPart;
}


#pragma mark -


//-----------------------------------------------------------------------------
//	SetupPickMonitorPane
//-----------------------------------------------------------------------------
//	Call this to initialize the user pane control that is the Pick Monitor
//	control. Pass the ControlRef of the user pane control and a display ID
//	for the monitor you want selected by default (pass 0 for the main monitor).
//	Call this function before displaying the dialog window.

OSErr SetupPickMonitorPane(ControlRef inPane, DisplayIDType inDefaultMonitor)
{
	GDHandle dev = GetDeviceList();
	OSErr err = noErr;
	
	// make the default monitor the selected device
	if (inDefaultMonitor)
		DMGetGDeviceByDisplayID(inDefaultMonitor, &sSelectedDevice, true);
	else
		sSelectedDevice = GetMainDevice();

	// build the list of monitors
	sNumMonitors = 0;
	while (dev && sNumMonitors < kMaxMonitors)
	{
		if (TestDeviceAttribute(dev, screenDevice) && TestDeviceAttribute(dev, screenActive))
		{
			sMonitors[sNumMonitors].device = dev;
			sMonitors[sNumMonitors].origRect = (**dev).gdRect;
			sMonitors[sNumMonitors].isMain = (dev == GetMainDevice());
			sNumMonitors++;
		}
		dev = GetNextDevice(dev);
	}

	// calculate scaled rects
	if (sNumMonitors)
	{
		Rect origPaneRect, paneRect;
		Rect origGrayRect, grayRect, scaledGrayRect;
		float srcAspect, dstAspect, scale;
		int i;
		
		GetControlBounds(inPane, &origPaneRect);
		paneRect = origPaneRect;
		OffsetRect(&paneRect, -paneRect.left, -paneRect.top);
		
		GetRegionBounds(GetGrayRgn(), &origGrayRect);
		grayRect = origGrayRect;
		OffsetRect(&grayRect, -grayRect.left, -grayRect.top);
		
		srcAspect = (float)grayRect.right / (float)grayRect.bottom;
		dstAspect = (float)paneRect.right / (float)paneRect.bottom;
		
		scaledGrayRect = paneRect;
		
		if (srcAspect < dstAspect)
		{
			scaledGrayRect.right = (float)paneRect.bottom * srcAspect;
			scale = (float)scaledGrayRect.right / grayRect.right;
		}
		else
		{
			scaledGrayRect.bottom = (float)paneRect.right / srcAspect;
			scale = (float)scaledGrayRect.bottom / grayRect.bottom;
		}
		
		for (i = 0; i < sNumMonitors; i++)
		{
			Rect r = sMonitors[i].origRect;
			Rect r2 = r;
			
			// normalize rect and scale
			OffsetRect(&r, -r.left, -r.top);
			r.bottom = (float)r.bottom * scale;
			r.right = (float)r.right * scale;
			
			// offset rect wrt gray region
			OffsetRect(&r, (float)(r2.left - origGrayRect.left) * scale, 
							(float)(r2.top - origGrayRect.top) * scale);

			sMonitors[i].scaledRect = r;
		}
		
		// center scaledGrayRect in the pane
		OffsetRect(&scaledGrayRect, (paneRect.right - scaledGrayRect.right) / 2,
					(paneRect.bottom - scaledGrayRect.bottom) / 2);

		// offset monitors to match
		for (i = 0; i < sNumMonitors; i++)
			OffsetRect(&sMonitors[i].scaledRect, scaledGrayRect.left, scaledGrayRect.top);
	}
	else
		return paramErr;
		
	// setup the procs for the pick monitor user pane
	err = SetupUserPaneProcs(inPane, drawProc, hitTestProc, trackingProc);
	return err;
}


//-----------------------------------------------------------------------------
//	TearDownPickMonitorPane
//-----------------------------------------------------------------------------
//	Disposes of everything associated with the Pick Monitor pane. You should
//	call this when disposing the dialog.

OSErr TearDownPickMonitorPane(ControlRef inPane)
{
	OSErr err;
	err = DisposeUserPaneProcs(inPane);
	sNumMonitors = 0;
	return err;
}

#pragma mark -

//------------------------------------------------------------------------------------
// ¥ PickMonitorHandler
//------------------------------------------------------------------------------------
// Our command handler for the PickMonitor dialog.

static pascal OSStatus PickMonitorHandler( EventHandlerCallRef inHandler, EventRef inEvent, void* inUserData )
{
	#pragma unused( inHandler )
	
	HICommand			cmd;
	OSStatus			result = eventNotHandledErr;
	WindowRef			theWindow = (WindowRef)inUserData;

	// The direct object for a 'process commmand' event is the HICommand.
	// Extract it here and switch off the command ID.

	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( cmd ), NULL, &cmd );

	switch ( cmd.commandID )
	{
		case kHICommandOK:			
			QuitAppModalLoopForWindow( theWindow );
			result = noErr;
			break;
		
		case kHICommandCancel:			
			// Setting sSelectedDevice to zero will signal that the user cancelled.
			sSelectedDevice = 0;
			QuitAppModalLoopForWindow( theWindow );
			result = noErr;
			break;

	}	
	return result;
}


#pragma mark -

//-----------------------------------------------------------------------------
// CanUserPickMonitor
//-----------------------------------------------------------------------------
// Returns true if more than one monitor is available to choose from.

Boolean CanUserPickMonitor (void)
{
	GDHandle dev = GetDeviceList();
	OSErr err = noErr;
	int numMonitors;
	
	// build the list of monitors
	numMonitors = 0;
	while (dev && numMonitors < kMaxMonitors)
	{
		if (TestDeviceAttribute(dev, screenDevice) && TestDeviceAttribute(dev, screenActive))
		{
			numMonitors++;
		}
		dev = GetNextDevice(dev);
	}

	if (numMonitors > 1) return true;
	else return false;
}

//-----------------------------------------------------------------------------
// PickMonitor
//-----------------------------------------------------------------------------
// Prompts for a monitor. Returns userCanceledErr if the user cancelled.

OSStatus PickMonitor (DisplayIDType *inOutDisplayID, WindowRef parentWindow)
{
	WindowRef theWindow;
	OSStatus status = noErr;
	static const ControlID	kUserPane 		= { 'MONI', 1 };
	
	// Fetch the dialog

	IBNibRef aslNib;
	CFBundleRef theBundle = CFBundleGetMainBundle();
	status = CreateNibReferenceWithCFBundle(theBundle, CFSTR("ASLCore"), &aslNib);
	status = ::CreateWindowFromNib(aslNib, CFSTR( "Pick Monitor" ), &theWindow );
	if (status != noErr)
	{
		assert(false);
		return userCanceledErr;
	}

#if 0
	// Put game name in window title. By default the title includes the token <<<kGameName>>>.

	Str255 windowTitle;
	GetWTitle(theWindow, windowTitle);
	FormatPStringWithGameName(windowTitle);
	SetWTitle(theWindow, windowTitle);
#endif
		
	// Set up the controls

	ControlRef monitorPane;
	GetControlByID( theWindow, &kUserPane, &monitorPane );
	assert(monitorPane);

	SetupPickMonitorPane(monitorPane, *inOutDisplayID);

	// Create our UPP and install the handler.

	EventTypeSpec cmdEvent = { kEventClassCommand, kEventCommandProcess };
	EventHandlerUPP handler = NewEventHandlerUPP( PickMonitorHandler );
	InstallWindowEventHandler( theWindow, handler, 1, &cmdEvent, theWindow, NULL );
	
	// Show the window

	if (parentWindow)
		ShowSheetWindow( theWindow, parentWindow );
	else
		ShowWindow( theWindow );

	// Now we run modally. We will remain here until the PrefHandler
	// calls QuitAppModalLoopForWindow if the user clicks OK or
	// Cancel.

	RunAppModalLoopForWindow( theWindow );

	// OK, we're done. Dispose of our window and our UPP.
	// We do the UPP last because DisposeWindow can send out
	// CarbonEvents, and we haven't explicitly removed our
	// handler. If we disposed the UPP, the Toolbox might try
	// to call it. That would be bad.

	TearDownPickMonitorPane(monitorPane);
	if (parentWindow)
		HideSheetWindow( theWindow );
	DisposeWindow( theWindow );
	DisposeEventHandlerUPP( handler );

	// Return settings to caller

	if (sSelectedDevice != 0)
	{
		// Read back the controls
		DMGetDisplayIDByGDevice (sSelectedDevice, &*inOutDisplayID, true);
		return noErr;
	}
	else
		return userCanceledErr;

}

