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
#include "PreferencesDialog.h"
#include "PickMonitor.h"
#include <list>
#include <set>

static idCVar r_stretched( "r_stretched", "0", CVAR_ARCHIVE | CVAR_BOOL, "Used stretched resolution" );

#define kPref_PrefsDialogAlways CFSTR("PrefsDialogAlways")
#define kPref_PrefsDialogOpenAL CFSTR("UseOpenAL")

#ifndef kAppCreator
#define kAppCreator			 	'DOM3'	// Creator type
#endif

const UInt32 kRes_Stretched 				= (1 << 0);		// set if the resolution is a stretched mode (kCGDisplayModeIsStretched)
const UInt32 kRes_Safe		 				= (1 << 1);		// еее╩(currently unused) set if the resolution is safe (kCGDisplayModeIsSafeForHardware)

// Data to be presented and edited in the prefs dialog
struct PrefInfo
{
	// prefs values
	GameDisplayMode					prefGameDisplayMode;
	CGDirectDisplayID				prefDisplayID;
	int								prefWidth;
	int								prefHeight;
	int								prefDepth;
	Fixed							prefFrequency;
	UInt32							prefResFlags;
	Boolean							prefAlways;
	Boolean							prefOpenAL;
	
	bool							okPressed;		// Set to true if the user pressed the OK button
	
	// The following are private data passed from GameDisplayPreferencesDialog() to it's command handler.
	WindowRef						window;
	ControlRef						fullscreenBtn;
	ControlRef						inAWindowBtn;
	ControlRef						resolutionPopup;
	ControlRef						refreshRatePopup;
	ControlRef						chooseMonitorsBtn;
	ControlRef						alwaysBtn;
	ControlRef						openALBtn;
	
	ValidModeCallbackProc			callback;		// To validate display modes
	
	bool							multiMonitor;	// Does user have multiple monitors
	std::list<Fixed>				refreshRates;	// List of refresh rates available for the selected monitor
	SInt32							freqMenuIndex;
};


#pragma mark -

bool R_GetModeInfo( int *width, int *height, int mode );

static int GetScreenIndexForDisplayID( CGDirectDisplayID inDisplayID ) {
	unsigned int i;
	OSErr err;
	int r_screen = -1;
	CGDisplayCount count;

	err = CGGetActiveDisplayList(0, NULL, &count);
	if (noErr == err) {
		CGDirectDisplayID displays[count];
		err = CGGetActiveDisplayList(count, displays, &count);
		if (noErr == err) {
			for ( i = 0; i < count; i++)
				if (displays[i] == inDisplayID)
					r_screen = i;
		}
	}
	return r_screen;
}

static CGDirectDisplayID GetDisplayIDForScreenIndex( int inScreenIndex ) {
	OSErr err;
	int r_screen = -1;
	CGDisplayCount count;
	
	err = CGGetActiveDisplayList(0, NULL, &count);
	if (noErr == err) {
		CGDirectDisplayID displays[count];
		err = CGGetActiveDisplayList(count, displays, &count);
		if (noErr == err) {
			if ( inScreenIndex >= 0 && inScreenIndex <= count )
				return displays[inScreenIndex];
		}
	}
	return (CGDirectDisplayID)r_screen;
}

    

void Sys_DoPreferences( void ) {
	   
	// An NSKeyDown event is not fired if the user holds down Cmd during startup.
	// Cmd is treated purely as a modifier. To capture the user
	// holding down Cmd, you would need to override NSApplication's
	// keydown handler. That's overkill for a single check at
	// startup, use the Carbon GetKeys approach.
	unsigned char km[16];
	const int kMacKeyCodeCommand = 0x37;
	KeyMap *keymap = (KeyMap*)&km;
	GetKeys(*keymap);
	
	Boolean prefAways, keyFound, useOpenAL;
	prefAways = CFPreferencesGetAppBooleanValue ( kPref_PrefsDialogAlways, kCFPreferencesCurrentApplication, &keyFound );
	bool fAlways = prefAways && keyFound;
		
	if ( fAlways || ( km[kMacKeyCodeCommand>>3] >> ( kMacKeyCodeCommand & 7 ) ) & 1 ) {
		GameDisplayInfo info;
		info.mode = cvarSystem->GetCVarBool( "r_fullscreen" ) ? kFullScreen : kWindow;
		info.displayID = GetDisplayIDForScreenIndex( cvarSystem->GetCVarInteger( "r_screen" ) );
		
		int w = 800, h = 600;
		R_GetModeInfo( &w, &h, cvarSystem->GetCVarInteger( "r_mode" ) );
		info.width = w;
		info.height = h;
		info.depth = 32;
		info.frequency = cvarSystem->GetCVarInteger( "r_maxDisplayRefresh" );
		info.windowLoc.x = 0;
		info.windowLoc.y = 0;
		info.flags = 0;
		info.resFlags = 0;
		if ( r_stretched.GetBool() )
			info.resFlags |= kRes_Stretched;
			
		WindowRef prefWindow;
		if ( CreateGameDisplayPreferencesDialog( &info, &prefWindow ) == noErr ) {
			if ( RunGameDisplayPreferencesDialog( &info, prefWindow ) == noErr ) {
				cvarSystem->SetCVarBool( "r_fullscreen",  info.mode == kFullScreen );

				int i = 0;
				int r_mode = -1;
				while ( r_mode == -1 && R_GetModeInfo( &w, &h, i ) ) {
					if ( w == info.width && h == info.height )
						r_mode = i;
					i++;
				}
				cvarSystem->SetCVarInteger( "r_mode", r_mode );
				if ( r_mode == -1 ) {
					cvarSystem->SetCVarInteger( "r_customWidth", info.width );
					cvarSystem->SetCVarInteger( "r_customHeight", info.height );
				}

				float r = (float) info.width / (float) info.height;
				if ( r > 1.7f )
					cvarSystem->SetCVarInteger( "r_aspectRatio", 1 );	// 16:9
				else if ( r > 1.55f )
					cvarSystem->SetCVarInteger( "r_aspectRatio", 2 );	// 16:10
				else
					cvarSystem->SetCVarInteger( "r_aspectRatio", 0 );	// 4:3
				
				r_stretched.SetBool( info.resFlags & kRes_Stretched );
				cvarSystem->SetCVarInteger( "r_screen", GetScreenIndexForDisplayID( info.displayID ) );
				cvarSystem->SetCVarInteger( "r_minDisplayRefresh", (int)FixedToFloat( info.frequency ) );
				cvarSystem->SetCVarInteger( "r_maxDisplayRefresh", (int)FixedToFloat( info.frequency ) );
			}
			else {
				Sys_Quit();		
			}
		}
	}
	useOpenAL = CFPreferencesGetAppBooleanValue (kPref_PrefsDialogOpenAL, kCFPreferencesCurrentApplication, &keyFound);
	if ( keyFound && useOpenAL ) {
		cvarSystem->SetCVarInteger( "com_asyncSound", 1 );
		cvarSystem->SetCVarInteger( "s_useOpenAL", 1 );
	}
	else {
		cvarSystem->SetCVarInteger( "com_asyncSound", 2 );
		cvarSystem->SetCVarInteger( "s_useOpenAL", 0 );
	}
}


#pragma mark -

#define EnablePopupMenuItem(inControl,inMenuItem)		EnableMenuItem(GetControlPopupMenuRef(inControl),inMenuItem)
#define DisablePopupMenuItem(inControl,inMenuItem)		DisableMenuItem(GetControlPopupMenuRef(inControl),inMenuItem)
#define IsPopupMenuItemEnabled(inControl,inMenuItem)	IsMenuItemEnabled(GetControlPopupMenuRef(inControl),inMenuItem)

// Command IDs used in the NIB file
enum
{
	kCmdFullscreen				= 'Full',
	kCmdInAWindow				= 'Wind',
	kCmdResolution				= 'Reso',
	kCmdRefreshRate				= 'Refr',
	kCmdChooseMonitors			= 'Moni',
};

// Control IDs used in the NIB file
static const ControlID	kFullscreenBtn 		= { 'PREF', 1 };
static const ControlID	kInAWindowBtn 		= { 'PREF', 2 };
static const ControlID	kResolutionPopup 	= { 'PREF', 3 };
static const ControlID	kRefreshRatePopup 	= { 'PREF', 4 };
static const ControlID	kChooseMonitorsBtn 	= { 'PREF', 5 };
static const ControlID	kAlwaysBtn 			= { 'PREF', 6 };
static const ControlID	kOpenALBtn 			= { 'PREF', 7 };

struct Res
{
	int width;
	int height;
	int depth;
	UInt32 resFlags;
};

static bool operator< (const Res& a, const Res& b)
{
	if (a.width == b.width)
	{
		if (a.height == b.height)
		{
			if (a.resFlags == b.resFlags)
			{
				return (a.depth < b.depth);
			}
			return (a.resFlags < b.resFlags);
		}
		return (a.height < b.height);
	}
	return (a.width < b.width);
}

inline Res MakeRes(int width, int height, int depth)
{
	Res temp = { width, height, depth, 0 };
	return temp;
}

inline Res MakeRes(int width, int height, int depth, UInt32 resFlags)
{
	Res temp = { width, height, depth, resFlags };
	return temp;
}

static bool ValidDisplayID (CGDirectDisplayID inDisplayID)
{
	unsigned int i;
	CGDisplayErr err;
	CGDisplayCount count;
	
	err = CGGetActiveDisplayList(0, NULL, &count);
	if (noErr == err)
	{
		CGDirectDisplayID displays[count];
		err = CGGetActiveDisplayList(count, displays, &count);
		if (noErr == err)
		{
			for ( i = 0; i < count; i++)
				if (displays[i] == inDisplayID)
					return true;
		}
	}
	return false;
}

static int BuildResolutionList(CGDirectDisplayID inDisplayID, Res *ioList, ValidModeCallbackProc inCallback)
{
	std::set<Res> modes;
	int i, total = 0;
	
	if (inDisplayID == (CGDirectDisplayID)-1)	// special case, not associated with any display
	{
		Res stdModes[] = {	{ 640, 480 }, { 800, 600 }, { 1024, 768 }, { 1152, 768 },
								{ 1280, 854 }, { 1280, 960 }, { 1280, 1024 }, { 1440, 900 } };
		total = sizeof(stdModes) / sizeof(Res);
		for (i = 0; i < total; i++)
		{
			if (inCallback == NULL || inCallback(inDisplayID, stdModes[i].width, stdModes[i].height, 32, 0))
				modes.insert( MakeRes(stdModes[i].width, stdModes[i].height, 32) );
		}
	}
	else
	{	
		CGDirectDisplayID displayID = inDisplayID ? inDisplayID : kCGDirectMainDisplay;
		CFArrayRef modeArrayRef = CGDisplayAvailableModes(displayID);
		CFIndex numModes = CFArrayGetCount(modeArrayRef);
		
		for (i = 0; i < numModes; i++)
		{
			CFDictionaryRef modeRef = (CFDictionaryRef)CFArrayGetValueAtIndex(modeArrayRef, i);
			
			long value = 0;
			CFNumberRef valueRef;
			Boolean success;
			
			valueRef = (CFNumberRef)CFDictionaryGetValue(modeRef, kCGDisplayBitsPerPixel);
			success = CFNumberGetValue(valueRef, kCFNumberLongType, &value);
			int depth = value;
			if (depth != 32) continue;
			
			valueRef = (CFNumberRef)CFDictionaryGetValue(modeRef, kCGDisplayWidth);
			success = CFNumberGetValue(valueRef, kCFNumberLongType, &value);
			int width = value;
			
			valueRef = (CFNumberRef)CFDictionaryGetValue(modeRef, kCGDisplayHeight);
			success = CFNumberGetValue(valueRef, kCFNumberLongType, &value);
			int height = value;
			
			UInt32 resFlags = 0;
			CFBooleanRef boolRef;
			if (CFDictionaryGetValueIfPresent (modeRef, kCGDisplayModeIsStretched, (const void **)&boolRef))
				if (CFBooleanGetValue (boolRef))
					resFlags |= kRes_Stretched;
			
			
			if (inCallback)
				success = inCallback(displayID, width, height, depth, 0);
			else
				success = true;
			
			if (success)
				modes.insert(MakeRes(width, height, depth, resFlags));
		}
	}
	
	total = modes.size();
	
	if (ioList)
	{
		std::set<Res>::iterator it = modes.begin();
		for (i = 0; it != modes.end(); i++)
			ioList[i] = *it++;
	}
	
	return total;	
}




static void BuildRefreshRates(CGDirectDisplayID inDisplayID, int inWidth, int inHeight, std::list<Fixed>* inList, ValidModeCallbackProc inCallback)
{
	CGDirectDisplayID displayID = inDisplayID ? inDisplayID : kCGDirectMainDisplay;
	
	CFArrayRef modeArrayRef = CGDisplayAvailableModes(displayID);
	CFIndex numModes = CFArrayGetCount(modeArrayRef);

	inList->clear();
	
	for (int i = 0; i < numModes; i++)
	{
		CFDictionaryRef modeRef = (CFDictionaryRef)CFArrayGetValueAtIndex(modeArrayRef, i);
		
		long value = 0;
		CFNumberRef valueRef;
		Boolean success;
		
		valueRef = (CFNumberRef)CFDictionaryGetValue(modeRef, kCGDisplayBitsPerPixel);
		success = CFNumberGetValue(valueRef, kCFNumberLongType, &value);
		int depth = value;
		if (depth != 32) continue;
		
		valueRef = (CFNumberRef)CFDictionaryGetValue(modeRef, kCGDisplayWidth);
		success = CFNumberGetValue(valueRef, kCFNumberLongType, &value);
		int width = value;
		
		valueRef = (CFNumberRef)CFDictionaryGetValue(modeRef, kCGDisplayHeight);
		success = CFNumberGetValue(valueRef, kCFNumberLongType, &value);
		int height = value;
		
		if (width == inWidth && height == inHeight)
		{
			double freqDouble;
			valueRef = (CFNumberRef)CFDictionaryGetValue(modeRef, kCGDisplayRefreshRate);
			success = CFNumberGetValue(valueRef, kCFNumberDoubleType, &freqDouble);
			Fixed	freq = FloatToFixed(freqDouble);
			if (inCallback)
				success = inCallback(displayID, width, height, depth, freq);
			else
				success = true;
			if (success)
				inList->push_back(freq);
		}
	}

	// Disallow 0, which we reserve to mean "automatic"
	inList->remove(0);
	
	inList->sort();
	
	// Remove duplicates - yes they can occur.
	inList->unique();
}

static void BuildRefreshPopupButton(ControlRef inControl, std::list<Fixed>* inList)
{
	MenuRef menu = GetControlPopupMenuRef(inControl);
	assert(menu);
	if (!menu) return;

	// The menu has two permanent items - "Auto" & a divider line. Delete everything else.
	DeleteMenuItems(menu, 3, CountMenuItems(menu)-2);
	
	for (std::list<Fixed>::const_iterator iter = inList->begin(); iter != inList->end(); ++iter)
	{
		float value = FixedToFloat(*iter);
		CFStringRef menuString = CFStringCreateWithFormat (kCFAllocatorDefault, 0, CFSTR("%g Hz"), value);
		InsertMenuItemTextWithCFString(menu, menuString, CountMenuItems(menu), 0, 0);
	}
	
	SetControlMaximum(inControl, CountMenuItems(menu));
}

static SInt32 FindRefreshPopupMenuItem(std::list<Fixed>* inList, Fixed inFrequency)
{
	SInt32 index = 3;	// skip over the "Auto" and divider ine
	for (std::list<Fixed>::const_iterator iter = inList->begin(); iter != inList->end(); ++iter)
	{
		if (*iter == inFrequency)
			return index;
		index++;
	}
	return 1;	// Return the "Automatic" item if we didn't find a match
}

static void BuildResolutionPopupButton(ControlRef inControl, CGDirectDisplayID inDisplayID, ValidModeCallbackProc inCallback)
{
	// Get the list of valid resolutions
	int count = BuildResolutionList(inDisplayID, NULL, inCallback);
	Res resList[count];
	BuildResolutionList(inDisplayID, resList, inCallback);
	
	// Clear the menu
	MenuRef menu = GetControlPopupMenuRef(inControl);
	assert(menu);
	if (!menu) return;
	DeleteMenuItems(menu, 1, CountMenuItems(menu));

	OSStatus err;

	while (count--)
	{
		CFStringRef menuString = CFStringCreateWithFormat (kCFAllocatorDefault, 0, CFSTR("%d x %d %@"),
			resList[count].width, resList[count].height, (resList[count].resFlags & kRes_Stretched) ? CFSTR("(Stretched)") : CFSTR(""));
		InsertMenuItemTextWithCFString (menu, menuString, 0, 0, 0);
		err = SetMenuItemProperty (menu, 1, kAppCreator, 'Res ', sizeof(resList[count]), &resList[count]);
	}
	
	SetControlMaximum(inControl, CountMenuItems(menu));
}

static void GetResolutionFromPopupMenuItem(ControlRef inControl, MenuItemIndex inItem, int *outX, int *outY, int *outDepth, UInt32 *outResFlags)
{
	MenuRef menu = GetControlPopupMenuRef(inControl);
	Res res;
	OSStatus err;
	
	err = GetMenuItemProperty (menu, inItem, kAppCreator, 'Res ', sizeof(res), NULL, &res);
	if (!err)
	{
		*outX = res.width;
		*outY = res.height;
		*outResFlags = res.resFlags;
		*outDepth = 32;
	}
}

static void AdjustResolutionPopupMenu(ControlRef inControl, CGDirectDisplayID inDisplayID, bool isFullscreen, int& screenwidth, int& screenheight, int& screendepth, UInt32& screenResFlags)
{
	int screenX = INT_MAX, screenY = INT_MAX;

	// In windowed mode, you have to disable resolutions that are larger than the current screen size
	if (!isFullscreen)
	{
		screenX = (int)CGDisplayPixelsWide(inDisplayID);
		screenY = (int)CGDisplayPixelsHigh(inDisplayID);
	}
	
	MenuRef menu = GetControlPopupMenuRef(inControl);
	int resX, resY, depth;
	UInt32 resFlags;
	int count = CountMenuItems(menu);
	int item;
		
	for( item = 1; item <= count; item++)
	{
		GetResolutionFromPopupMenuItem(inControl, item, &resX, &resY, &depth, &resFlags);

		if (screenX < resX || screenY < resY)
			DisablePopupMenuItem(inControl, item);
		else
			EnablePopupMenuItem(inControl, item);
			
		if (resX == screenwidth && resY == screenheight && depth == screendepth && resFlags == screenResFlags)
			SetControlValue(inControl, item);
	}
	
	// If we just disabled the current item, then choose something else.
	if (!IsPopupMenuItemEnabled(inControl, GetControlValue (inControl)))
	{
		for(item = 1; item <= count; item++)
		{
			if (IsPopupMenuItemEnabled(inControl, item))
			{
				SetControlValue(inControl, item);
				GetResolutionFromPopupMenuItem(inControl, item, &screenwidth, &screenheight, &screendepth, &screenResFlags);
				break;
			}
		}
	}
}

static void AdjustDisplayControls(PrefInfo *prefInfo)
{
	// Build new resolution popup and select appropriate resolution
	if ((prefInfo->prefGameDisplayMode != kFullScreen))
	{
		BuildResolutionPopupButton(prefInfo->resolutionPopup, (CGDirectDisplayID)-1, prefInfo->callback);
		if (prefInfo->multiMonitor)
			EnableControl(prefInfo->chooseMonitorsBtn);
	}
	else
	{
		BuildResolutionPopupButton(prefInfo->resolutionPopup, prefInfo->prefDisplayID, prefInfo->callback);
		if (prefInfo->multiMonitor)
			EnableControl(prefInfo->chooseMonitorsBtn);
	}
	AdjustResolutionPopupMenu(prefInfo->resolutionPopup, prefInfo->prefDisplayID,
		prefInfo->prefGameDisplayMode == kFullScreen, 
		prefInfo->prefWidth, prefInfo->prefHeight, prefInfo->prefDepth, prefInfo->prefResFlags);
	
	// Build new refresh popup and select appropriate rate
	BuildRefreshRates(prefInfo->prefDisplayID, prefInfo->prefWidth, prefInfo->prefHeight,
		&prefInfo->refreshRates, prefInfo->callback);
	BuildRefreshPopupButton(prefInfo->refreshRatePopup, &prefInfo->refreshRates);

	if (prefInfo->refreshRates.size() == 0)
	{	// No refresh rates, so pick Auto
		prefInfo->freqMenuIndex = 1;
		prefInfo->prefFrequency = 0;
	}
	else
	{
		prefInfo->freqMenuIndex = FindRefreshPopupMenuItem(&prefInfo->refreshRates, prefInfo->prefFrequency);
		if (prefInfo->freqMenuIndex == 1)
			prefInfo->prefFrequency = 0;	// just in case FindRefreshPopupMenuItem didn't find prefInfo->prefFrequency
	}
	SetControlValue (prefInfo->refreshRatePopup, prefInfo->freqMenuIndex);

	// Disable refresh rate if NOT fullscreen
	if ((prefInfo->prefGameDisplayMode != kFullScreen) || (prefInfo->refreshRates.size() == 0))
		DisableControl (prefInfo->refreshRatePopup);
	else
		EnableControl (prefInfo->refreshRatePopup);
}

#pragma mark -

static pascal OSStatus PrefHandler( EventHandlerCallRef inHandler, EventRef inEvent, void* inUserData )
{
	#pragma unused( inHandler )
	
	HICommand			cmd;
	OSStatus			result = eventNotHandledErr;
	PrefInfo*			prefInfo = (PrefInfo*)inUserData;

	// The direct object for a 'process commmand' event is the HICommand.
	// Extract it here and switch off the command ID.

	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( cmd ), NULL, &cmd );

	switch ( cmd.commandID )
	{
		case kHICommandOK:
			
			prefInfo->okPressed = true;
			
			prefInfo->prefAlways = GetControlValue (prefInfo->alwaysBtn);
			prefInfo->prefOpenAL = GetControlValue (prefInfo->openALBtn);
			
			CFPreferencesSetAppValue (kPref_PrefsDialogAlways, 
									  prefInfo->prefAlways ? kCFBooleanTrue : kCFBooleanFalse,
									  kCFPreferencesCurrentApplication);

			CFPreferencesSetAppValue (kPref_PrefsDialogOpenAL, 
									  prefInfo->prefOpenAL ? kCFBooleanTrue : kCFBooleanFalse,
									  kCFPreferencesCurrentApplication);
			
			CFPreferencesAppSynchronize (kCFPreferencesCurrentApplication);

			QuitAppModalLoopForWindow( prefInfo->window );
			result = noErr;
			break;
		
		case kHICommandCancel:
			
			prefInfo->okPressed = false;
			
			QuitAppModalLoopForWindow( prefInfo->window );
			result = noErr;
			break;
		
		case kCmdFullscreen:
		case kCmdInAWindow:
			if (cmd.commandID == kCmdFullscreen)
				prefInfo->prefGameDisplayMode = kFullScreen;
			else
				prefInfo->prefGameDisplayMode = kWindow;
			SetControlValue (prefInfo->fullscreenBtn, prefInfo->prefGameDisplayMode == kFullScreen);
			SetControlValue (prefInfo->inAWindowBtn, 1 - (prefInfo->prefGameDisplayMode == kFullScreen));
			if (prefInfo->prefGameDisplayMode == kFullScreen)
				EnableControl (prefInfo->refreshRatePopup);
			else 
				DisableControl (prefInfo->refreshRatePopup);
			if (prefInfo->multiMonitor)
				EnableControl (prefInfo->chooseMonitorsBtn);
			else
				DisableControl (prefInfo->chooseMonitorsBtn);
				
			// Adjust resolutions, refresh rates
			AdjustDisplayControls(prefInfo);
			result = noErr;
			break;
		

		case kCmdChooseMonitors:
		{
			PickMonitor((DisplayIDType*)&prefInfo->prefDisplayID, prefInfo->window);
			// Adjust resolutions, refresh rates for potentially new display ID
			AdjustDisplayControls(prefInfo);
			break;
		}

		case kCmdResolution:
		{
			// Pick a new resolution
			int item = GetControlValue(prefInfo->resolutionPopup);
			GetResolutionFromPopupMenuItem(prefInfo->resolutionPopup, item, &prefInfo->prefWidth, &prefInfo->prefHeight, &prefInfo->prefDepth, &prefInfo->prefResFlags);
	
			// Adjust refresh menu
			BuildRefreshRates(prefInfo->prefDisplayID, prefInfo->prefWidth, prefInfo->prefHeight, &prefInfo->refreshRates, prefInfo->callback);
			BuildRefreshPopupButton(prefInfo->refreshRatePopup, &prefInfo->refreshRates);
			prefInfo->freqMenuIndex = FindRefreshPopupMenuItem(&prefInfo->refreshRates, prefInfo->prefFrequency);
			if (prefInfo->freqMenuIndex == 1)
				prefInfo->prefFrequency = 0;	// just in case FindRefreshPopupMenuItem didn't find prefInfo->prefFrequency
			SetControlValue (prefInfo->refreshRatePopup, prefInfo->freqMenuIndex);

			// Disable refresh rate if NOT fullscreen
			if ((prefInfo->prefGameDisplayMode != kFullScreen) || (prefInfo->refreshRates.size() == 0))
				DisableControl (prefInfo->refreshRatePopup);
			else
				EnableControl (prefInfo->refreshRatePopup);
			
			break;
		}

		case kCmdRefreshRate:
		{
			// Keep prefInfo->prefFrequency updated for the other controls to reference
			prefInfo->freqMenuIndex = GetControlValue (prefInfo->refreshRatePopup);
			if (prefInfo->freqMenuIndex == 1)
				prefInfo->prefFrequency = 0;
			else
			{
				std::list<Fixed>::const_iterator iter = prefInfo->refreshRates.begin();
				for (int i = 0; i < prefInfo->freqMenuIndex-3; i++)
					iter++;
				prefInfo->prefFrequency = *iter;
			}
			break;
		}


	}	
	return result;
}

#pragma mark -

static DEFINE_ONE_SHOT_HANDLER_GETTER(PrefHandler)

OSStatus CreateGameDisplayPreferencesDialog(const GameDisplayInfo *inGDInfo,
											WindowRef *outWindow, ValidModeCallbackProc inCallback)
{
	OSStatus err = noErr;
	
	// Build up a structure to pass to the window handler we are about
	// to install. We store the window itself, as well as the original
	// states of our settings. We use this to revert if the user clicks
	// the cancel button.
	
	static PrefInfo prefInfo;
	
	prefInfo.prefGameDisplayMode = inGDInfo->mode;
	prefInfo.prefDisplayID = inGDInfo->displayID;
	prefInfo.prefWidth = inGDInfo->width;
	prefInfo.prefHeight = inGDInfo->height;
	prefInfo.prefDepth = inGDInfo->depth;
	prefInfo.prefFrequency = inGDInfo->frequency;
	prefInfo.prefResFlags = inGDInfo->resFlags;
	prefInfo.window = NULL;
	prefInfo.okPressed = false;
	
	Boolean result;
	Boolean keyFound;
	result = CFPreferencesGetAppBooleanValue (kPref_PrefsDialogAlways, kCFPreferencesCurrentApplication, &keyFound);
	prefInfo.prefAlways = result && keyFound;
	result = CFPreferencesGetAppBooleanValue (kPref_PrefsDialogOpenAL, kCFPreferencesCurrentApplication, &keyFound);
	prefInfo.prefOpenAL = result && keyFound;
	
	prefInfo.callback = inCallback;
	
	// If DoPreferences is called at the start of the game, prefInfo.prefDisplayID needs to be checked
	// to see if it is still a valid display ID.
	
	if (!ValidDisplayID(prefInfo.prefDisplayID))
		prefInfo.prefDisplayID = kCGDirectMainDisplay;	// revert to main
	
	// Fetch the dialog
	
	IBNibRef aslNib;
	CFBundleRef theBundle = CFBundleGetMainBundle();
	err = CreateNibReferenceWithCFBundle(theBundle, CFSTR("ASLCore"), &aslNib);
	err = ::CreateWindowFromNib(aslNib, CFSTR( "Preferences" ), &prefInfo.window );
	if (err != noErr)
		return err;
	SetWRefCon(prefInfo.window, (long)&prefInfo);
	
	// Locate all the controls
	
	GetControlByID( prefInfo.window, &kFullscreenBtn, &prefInfo.fullscreenBtn );			assert(prefInfo.fullscreenBtn);
	GetControlByID( prefInfo.window, &kInAWindowBtn, &prefInfo.inAWindowBtn );				assert(prefInfo.inAWindowBtn);
	GetControlByID( prefInfo.window, &kResolutionPopup, &prefInfo.resolutionPopup );		assert(prefInfo.resolutionPopup);
	GetControlByID( prefInfo.window, &kRefreshRatePopup, &prefInfo.refreshRatePopup );		assert(prefInfo.refreshRatePopup);
	GetControlByID( prefInfo.window, &kChooseMonitorsBtn, &prefInfo.chooseMonitorsBtn );	assert(prefInfo.chooseMonitorsBtn);
	GetControlByID( prefInfo.window, &kAlwaysBtn, &prefInfo.alwaysBtn );					assert(prefInfo.alwaysBtn);
	GetControlByID( prefInfo.window, &kOpenALBtn, &prefInfo.openALBtn );					assert(prefInfo.openALBtn);
	
	
	
	// Disable the "choose monitor" button if we've only got one to pick from
	
	prefInfo.multiMonitor = CanUserPickMonitor();
	
	if (!prefInfo.multiMonitor)
	{
		DisableControl (prefInfo.chooseMonitorsBtn);
		prefInfo.prefDisplayID = 0;
	}
	
	// Prepare the resolutions and refresh rates popup menus
	AdjustDisplayControls(&prefInfo);
	
	// Set up the controls
	
	SetControlValue (prefInfo.refreshRatePopup, prefInfo.freqMenuIndex);
	SetControlValue (prefInfo.fullscreenBtn, prefInfo.prefGameDisplayMode == kFullScreen);
	SetControlValue (prefInfo.inAWindowBtn, prefInfo.prefGameDisplayMode == kWindow);
	SetControlValue (prefInfo.alwaysBtn, prefInfo.prefAlways);
	SetControlValue (prefInfo.openALBtn, prefInfo.prefOpenAL);

	
	// Create our UPP and install the handler.
	
	EventTypeSpec cmdEvent = { kEventClassCommand, kEventCommandProcess };
	EventHandlerUPP handler = GetPrefHandlerUPP();
	InstallWindowEventHandler( prefInfo.window, handler, 1, &cmdEvent, &prefInfo, NULL );
	
	// Position and show the window
	RepositionWindow( prefInfo.window, NULL, kWindowAlertPositionOnMainScreen );
	
	if (outWindow)
		*outWindow = prefInfo.window;
	
	return err;
}


//------------------------------------------------------------------------------------
// е RunGameDisplayPreferencesDialog
//------------------------------------------------------------------------------------
// Runs the Mac-specific preferences dialog.

OSStatus RunGameDisplayPreferencesDialog(GameDisplayInfo *outGDInfo, WindowRef inWindow)
{
	PrefInfo *prefInfo = (PrefInfo*)GetWRefCon(inWindow);
	
	ShowWindow( inWindow );
	
	// Now we run modally. We will remain here until the PrefHandler
	// calls QuitAppModalLoopForWindow if the user clicks OK or
	// Cancel.
	
	RunAppModalLoopForWindow( inWindow );
	
	// OK, we're done. Dispose of our window.
	// TODO: Are we supposed to uninstall event handlers?
	DisposeWindow( inWindow );
	
	// Return settings to caller
	
	if (prefInfo->okPressed)
	{		
		outGDInfo->mode = prefInfo->prefGameDisplayMode;
		outGDInfo->width = prefInfo->prefWidth;
		outGDInfo->height = prefInfo->prefHeight;
		outGDInfo->depth = prefInfo->prefDepth;
		outGDInfo->frequency = prefInfo->prefFrequency;
		outGDInfo->resFlags = prefInfo->prefResFlags;
		outGDInfo->displayID = prefInfo->prefDisplayID;
	}
	
	return prefInfo->okPressed ? noErr : userCanceledErr;
}







