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

// -*- mode: objc -*-
#import "../../idlib/precompiled.h"

#import "macosx_local.h"
#import "macosx_sys.h"

#import <AppKit/NSCursor.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSScreen.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSGraphicsContext.h>
#import <AppKit/NSEvent.h>

#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>
#import <Foundation/NSRunLoop.h>
#import <Carbon/Carbon.h>

#import <ApplicationServices/ApplicationServices.h>

#import <sys/types.h>
#import <sys/time.h>
#import <unistd.h>
#include <pthread.h>

static NSDate  *distantPast		= NULL;
static bool		inputActive		= false;
static bool		mouseActive		= false;
static bool		inputRectValid	= NO;
static CGRect	inputRect;
static const void *sKLuchrData	= NULL;
static const void *sKLKCHRData	= NULL;

int	vkeyToDoom3Key[256] = {
	/*0x00*/	'a', 's', 'd', 'f', 'h', 'g', 'z', 'x',
	/*0x08*/	'c', 'v', '?', 'b', 'q', 'w', 'e', 'r',
	/*0x10*/	'y', 't', '1', '2', '3', '4', '6', '5',
	/*0x18*/	'=', '9', '7', '-', '8', '0', ']', 'o',
	/*0x20*/	'u', '[', 'i', 'p', K_ENTER, 'l', 'j', '\'',
	/*0x28*/	'k', ';', '\\', ',', '/', 'n', 'm', '.',
	/*0x30*/	K_TAB, K_SPACE, '`', K_BACKSPACE, '?', K_ESCAPE, '?', K_COMMAND,
	/*0x38*/	K_SHIFT, K_CAPSLOCK, K_ALT, K_CTRL, '?', '?', '?', '?',
	/*0x40*/	'?', K_KP_DEL, '?', K_KP_STAR, '?', K_KP_PLUS, '?', K_KP_NUMLOCK,
	/*0x48*/	'?', '?', '?', K_KP_SLASH, K_KP_ENTER, '?', K_KP_MINUS, '?',
	/*0x50*/	'?', K_KP_EQUALS, K_KP_INS, K_KP_END, K_KP_DOWNARROW, K_KP_PGDN, K_KP_LEFTARROW, K_KP_5,
	/*0x58*/	K_KP_RIGHTARROW, K_KP_HOME, '?', K_KP_UPARROW, K_KP_PGUP, '?', '?', '?',
	/*0x60*/	K_F5, K_F6, K_F7, K_F3, K_F8, K_F9, '?', K_F11,
	/*0x68*/	'?', K_PRINT_SCR, '?', K_F14, '?', K_F10, '?', K_F12,
	/*0x70*/	'?', K_F15, K_INS, K_HOME, K_PGUP, K_DEL, K_F4, K_END,	
	/*0x78*/	K_F2, K_PGDN, K_F1, K_LEFTARROW, K_RIGHTARROW, K_DOWNARROW, K_UPARROW, K_POWER
};

int	vkeyToDoom3Key_French[256] = {
	/*0x00*/	'q',	's',		'd',	'f',		'h',		'g',		'w',		'x',
	/*0x08*/	'c',	'v',		'?',	'b',		'a',		'z',		'e',		'r',
	/*0x10*/	'y',	't',		'1',	'2',		'3',		'4',		'6',		'5',
	/*0x18*/	'-',	'9',		'7',	')',		'8',		'0',		'$',		'o',
	/*0x20*/	'u',	'^',		'i',	'p',		K_ENTER,	'l',		'j',		'ù',
	/*0x28*/	'k',	'm',		0x60,	';',			'=',	'n',		',',			':',
	/*0x30*/	K_TAB, K_SPACE, '<', K_BACKSPACE, '?', K_ESCAPE, '?', K_COMMAND,
	/*0x38*/	K_SHIFT, K_CAPSLOCK, K_ALT, K_CTRL, '?', '?', '?', '?',
	/*0x40*/	'?', K_KP_DEL, '?', K_KP_STAR, '?', K_KP_PLUS, '?', K_KP_NUMLOCK,
	/*0x48*/	'?', '?', '?', K_KP_SLASH, K_KP_ENTER, '?', K_KP_MINUS, '?',
	/*0x50*/	'?', K_KP_EQUALS, K_KP_INS, K_KP_END, K_KP_DOWNARROW, K_KP_PGDN, K_KP_LEFTARROW, K_KP_5,
	/*0x58*/	K_KP_RIGHTARROW, K_KP_HOME, '?', K_KP_UPARROW, K_KP_PGUP, '?', '?', '?',
	/*0x60*/	K_F5, K_F6, K_F7, K_F3, K_F8, K_F9, '?', K_F11,
	/*0x68*/	'?', K_PRINT_SCR, '?', K_F14, '?', K_F10, '?', K_F12,
	/*0x70*/	'?', K_F15, K_INS, K_HOME, K_PGUP, K_DEL, K_F4, K_END,	
	/*0x78*/	K_F2, K_PGDN, K_F1, K_LEFTARROW, K_RIGHTARROW, K_DOWNARROW, K_UPARROW, K_POWER
};

int	vkeyToDoom3Key_German[256] = {
	/*0x00*/	'a',	's',		'd',	'f',		'h',		'g',		'y',		'x',
	/*0x08*/	'c',	'v',		'?',	'b',		'q',		'w',		'e',		'r',
	/*0x10*/	'z',	't',		'1',	'2',		'3',		'4',		'6',		'5',
	/*0x18*/	'«',	'9',		'7',	'-',		'8',		'0',		'+',		'o',
	/*0x20*/	'u',	'[',		'i',	'p',		K_ENTER,	'l',		'j',		'\'',
	/*0x28*/	'k',	';',		'#',	',',		'-',		'n',		'm',		'.',
	/*0x30*/	K_TAB, K_SPACE, '`', K_BACKSPACE, '?', K_ESCAPE, '?', K_COMMAND,
	/*0x38*/	K_SHIFT, K_CAPSLOCK, K_ALT, K_CTRL, '?', '?', '?', '?',
	/*0x40*/	'?', K_KP_DEL, '?', K_KP_STAR, '?', K_KP_PLUS, '?', K_KP_NUMLOCK,
	/*0x48*/	'?', '?', '?', K_KP_SLASH, K_KP_ENTER, '?', K_KP_MINUS, '?',
	/*0x50*/	'?', K_KP_EQUALS, K_KP_INS, K_KP_END, K_KP_DOWNARROW, K_KP_PGDN, K_KP_LEFTARROW, K_KP_5,
	/*0x58*/	K_KP_RIGHTARROW, K_KP_HOME, '?', K_KP_UPARROW, K_KP_PGUP, '?', '?', '?',
	/*0x60*/	K_F5, K_F6, K_F7, K_F3, K_F8, K_F9, '?', K_F11,
	/*0x68*/	'?', K_PRINT_SCR, '?', K_F14, '?', K_F10, '?', K_F12,
	/*0x70*/	'?', K_F15, K_INS, K_HOME, K_PGUP, K_DEL, K_F4, K_END,	
	/*0x78*/	K_F2, K_PGDN, K_F1, K_LEFTARROW, K_RIGHTARROW, K_DOWNARROW, K_UPARROW, K_POWER
};

static const int *vkeyTable = vkeyToDoom3Key;	

/*
 ===========
 Sys_InitScanTable
 ===========
 */
void Sys_InitScanTable( void ) {
	KeyboardLayoutRef kbLayout;
	
	idStr lang = cvarSystem->GetCVarString( "sys_lang" );
	if ( lang.Length() == 0 ) {
		lang = "english";
	}

	if ( lang.Icmp( "english" ) == 0 ) {
		vkeyTable = vkeyToDoom3Key;
	} else if ( lang.Icmp( "french" ) == 0 ) {
		vkeyTable = vkeyToDoom3Key_French;
	} else if ( lang.Icmp( "german" ) == 0 ) {
		vkeyTable = vkeyToDoom3Key_German;
	}

	if ( KLGetCurrentKeyboardLayout( &kbLayout )  == 0 ) {
		if ( KLGetKeyboardLayoutProperty( kbLayout, kKLuchrData, &sKLuchrData ) ) {
			common->Warning("KLGetKeyboardLayoutProperty failed");
		}
		if ( !sKLuchrData ) {
			if ( KLGetKeyboardLayoutProperty( kbLayout, kKLKCHRData, &sKLKCHRData ) ) {
				common->Warning("KLGetKeyboardLayoutProperty failed");
			}
		}
	}
	if ( !sKLuchrData && !sKLKCHRData ) {
		common->Warning("Keyboard input initialziation failed");
	}
}

void Sys_InitInput( void ) {
	common->Printf( "------- Input Initialization -------\n" );
	
	if ( !distantPast ) {
		distantPast = [ [ NSDate distantPast ] retain ];
	}

    IN_ActivateMouse();

	inputActive = true;
}

void Sys_ShutdownInput( void ) {
	common->Printf( "------- Input Shutdown -------\n" );

    if ( !inputActive ) {
        return;
    }
    inputActive = false;
    if ( mouseActive ) {
		IN_DeactivateMouse();
    }
	
    common->Printf( "------------------------------\n" );
}

void processMouseMovedEvent( NSEvent *mouseMovedEvent ) {
    CGMouseDelta dx, dy;
    
    if ( !mouseActive ) {
        return;
	}

#if 0

#define ACT_LIKE_WINDOWS
#ifdef ACT_LIKE_WINDOWS
    cvar_t *in_mouseLowEndSlope = Cvar_Get("in_mouseLowEndSlope", "3.5", CVAR_ARCHIVE);
    if (in_mouseLowEndSlope->value < 1) {
        Cvar_Set("in_mouseLowEndSlope", "1");
    }
#else
    cvar_t *in_mouseLowEndSlope = Cvar_Get("in_mouseLowEndSlope", "1", CVAR_ARCHIVE);
    if (in_mouseLowEndSlope->value < 1) {
        Cvar_Set("in_mouseLowEndSlope", "1");
    }
#endif

    cvar_t *in_mouseHighEndCutoff = Cvar_Get("in_mouseHighEndCutoff", "20", CVAR_ARCHIVE);
    if (in_mouseLowEndSlope->value < 1) {
        Cvar_Set("in_mouseHighEndCutoff", "1");
    }

#endif

    CGGetLastMouseDelta(&dx, &dy);
    
    if ( dx || dy ) {
  #if 0 // this is be handled by the mouse driver clean me out later       
      CGMouseDelta distSqr;
        float m0, N;
        
        distSqr = dx * dx + dy * dy;
        //Com_Printf("distSqr = %d\n", distSqr);

        /* This code is here to help people that like the feel of the Logitech USB Gaming Mouse with the Win98 drivers.  By empirical testing, the Windows drivers seem to be more heavily accelerated at the low end of the curve. */
        //N = in_mouseHighEndCutoff->value;
		N = 1;

        if (distSqr < N*N) {
            float dist, accel, scale;
            
            //m0 = in_mouseLowEndSlope->value;
			m0 = 1;
            dist = sqrt(distSqr);
            accel = (((m0 - 1.0)/(N*N) * dist + (2.0 - 2.0*m0)/N) * dist + m0) * dist;
            
            scale = accel / dist;
            //Com_Printf("dx = %d, dy = %d, dist = %f, accel = %f, scale = %f\n", dx, dy, dist, accel, scale);

            dx *= scale;
            dy *= scale;
        }
#endif
        Posix_QueEvent( SE_MOUSE, dx, dy, 0, NULL );
		Posix_AddMousePollEvent( M_DELTAX, dx );
		Posix_AddMousePollEvent( M_DELTAY, dy );
    }
}

inline bool OSX_LookupCharacter(unsigned short vkey, unsigned int modifiers, bool keyDownFlag, unsigned char *outChar)
{
	UInt32 translated;
	UInt32 deadKeyState = 0;
	UniChar unicodeString[16];
	UniCharCount actualStringLength = 0;
	static UInt32 keyTranslateState = 0;
	
	// Only want character if Translate() returns a single character
	if ( sKLuchrData ) {
		UCKeyTranslate( (UCKeyboardLayout*)sKLuchrData, vkey, keyDownFlag ? kUCKeyActionDown : kUCKeyActionUp, modifiers,
						LMGetKbdType(), 0, &deadKeyState, 16, &actualStringLength, unicodeString );

		if ( actualStringLength == 1 ) {
			*outChar = (unsigned char)unicodeString[0];
			return true;
		}
	}
	else if ( sKLKCHRData ) {
		translated = KeyTranslate( sKLKCHRData, vkey, &keyTranslateState );
		if ( ( translated & 0x00ff0000 ) == 0 ) {
			*outChar = translated & 0xff;
			return true;
		}
	}
	return false;
}

void OSX_ProcessKeyEvent( NSEvent *keyEvent, bool keyDownFlag ) {
	unsigned char character;
	unsigned int modifiers = 0;
	unsigned short vkey = [ keyEvent keyCode ];

	if ( [ keyEvent modifierFlags ] & NSAlphaShiftKeyMask )
		modifiers |= alphaLock;
	if ( [ keyEvent modifierFlags ] & NSShiftKeyMask )
		modifiers |= shiftKey;
	if ( [ keyEvent modifierFlags ] & NSControlKeyMask )
		modifiers |= controlKey;
	if ( [ keyEvent modifierFlags ] & NSAlternateKeyMask )
		modifiers |= optionKey;
	if ( [ keyEvent modifierFlags ] & NSCommandKeyMask )
		modifiers |= cmdKey;
	modifiers >>= 8;
				
	int doomKey = (unsigned char)vkeyTable[vkey];
	Posix_QueEvent( SE_KEY, doomKey, keyDownFlag, 0, NULL );
	if ( keyDownFlag ) {
		if ( OSX_LookupCharacter(vkey, modifiers, keyDownFlag, &character ) && 
			 character != Sys_GetConsoleKey( false ) && character != Sys_GetConsoleKey( true ) ) {
			Posix_QueEvent( SE_CHAR, character, 0, 0, NULL);
		}
	}
	Posix_AddKeyboardPollEvent( doomKey, keyDownFlag );
	
	return;
}

void sendEventForMaskChangeInFlags( int quakeKey, unsigned int modifierMask, unsigned int oldModifierFlags, unsigned int newModifierFlags ) {
    bool oldHadModifier, newHasModifier;

    oldHadModifier = (oldModifierFlags & modifierMask) != 0;
    newHasModifier = (newModifierFlags & modifierMask) != 0;
    if (oldHadModifier != newHasModifier) {
        //NSLog(@"Key %d posted for modifier mask modifierMask", quakeKey);
        Posix_QueEvent( SE_KEY, quakeKey, newHasModifier, 0, NULL);
		Posix_AddKeyboardPollEvent( quakeKey, newHasModifier );
    }
}

void processFlagsChangedEvent( NSEvent *flagsChangedEvent ) {
    static int	oldModifierFlags;
    int			newModifierFlags;

    newModifierFlags = [flagsChangedEvent modifierFlags];
    sendEventForMaskChangeInFlags( K_ALT, NSAlternateKeyMask, oldModifierFlags, newModifierFlags );
    sendEventForMaskChangeInFlags( K_CTRL, NSControlKeyMask, oldModifierFlags, newModifierFlags );
    sendEventForMaskChangeInFlags( K_SHIFT, NSShiftKeyMask, oldModifierFlags, newModifierFlags );
    oldModifierFlags = newModifierFlags;
}

void processSystemDefinedEvent( NSEvent *systemDefinedEvent ) {
    static int oldButtons = 0;
    int buttonsDelta;
    int buttons;
    int isDown;
    
    if ( [systemDefinedEvent subtype] == 7 ) {

        if ( !mouseActive ) {
            return;
		}        
    
		buttons = [systemDefinedEvent data2];
        buttonsDelta = oldButtons ^ buttons;
        
        //common->Printf( "uberbuttons: %08lx %08lx\n", buttonsDelta, buttons );

		if (buttonsDelta & 1) {
            isDown = buttons & 1;
            Posix_QueEvent( SE_KEY, K_MOUSE1, isDown, 0, NULL);
			Posix_AddMousePollEvent( M_ACTION1, isDown );
		}

		if (buttonsDelta & 2) {
            isDown = buttons & 2;
            Posix_QueEvent( SE_KEY, K_MOUSE2, isDown, 0, NULL);
			Posix_AddMousePollEvent( M_ACTION2, isDown );
		}

		if (buttonsDelta & 4) {
            isDown = buttons & 4;
            Posix_QueEvent( SE_KEY, K_MOUSE3, isDown, 0, NULL);
			Posix_AddMousePollEvent( M_ACTION3, isDown );
		}

		if (buttonsDelta & 8) {
            isDown = buttons & 8;
            Posix_QueEvent( SE_KEY, K_MOUSE4, isDown, 0, NULL);
			Posix_AddMousePollEvent( M_ACTION4, isDown );
        }
        
		if (buttonsDelta & 16) {
            isDown = buttons & 16;
            Posix_QueEvent( SE_KEY, K_MOUSE5, isDown, 0, NULL);
			Posix_AddMousePollEvent( M_ACTION5, isDown );
		}
        
        oldButtons = buttons;
    }
}

void processEvent( NSEvent *event ) {
	NSEventType eventType;

	if ( !inputActive ) {
		return;
	}

	eventType = [ event type ];

	switch ( eventType ) {
		// These four event types are ignored since we do all of our mouse down/up process via the uber-mouse system defined event.	 We have to accept these events however since they get enqueued and the queue will fill up if we don't.
	case NSLeftMouseDown:
	case NSLeftMouseUp:
	case NSRightMouseDown:
	case NSRightMouseUp:
		//NSLog( @"ignore simple mouse event %@", event );
		return;		
	case NSMouseMoved:
	case NSLeftMouseDragged:
	case NSRightMouseDragged:
		processMouseMovedEvent( event );
		return;
	case NSKeyDown:
		// Send ALL command key-ups to Quake, but not command key-downs, otherwise if the user hits a key, presses command, and lets up on the key, the key-up won't register.
		if ( [ event modifierFlags ] & NSCommandKeyMask ) {
			NSLog( @"command key up ignored: %@", event );
			break;
		}
	case NSKeyUp:
		OSX_ProcessKeyEvent( event, eventType == NSKeyDown );
		return;
	case NSFlagsChanged:
		processFlagsChangedEvent( event );
		return;
	case NSSystemDefined:
		processSystemDefinedEvent( event );
		return;
	case NSScrollWheel:
		if ([event deltaY] < 0.0) {
			Posix_QueEvent( SE_KEY, K_MWHEELDOWN, true, 0, NULL );
			Posix_QueEvent( SE_KEY, K_MWHEELDOWN, false, 0, NULL );
			Posix_AddMousePollEvent( M_DELTAZ, -1 );
		} else {
			Posix_QueEvent( SE_KEY, K_MWHEELUP, true, 0, NULL );
			Posix_QueEvent( SE_KEY, K_MWHEELUP, false, 0, NULL );
			Posix_AddMousePollEvent( M_DELTAZ, 1 );
		}
		return;
	default:
		//NSLog( @"handle event %@", event );
		break;
	}
    [NSApp sendEvent:event];
}

void Posix_PollInput( void ) {
    NSEvent *event;
    unsigned int eventMask;
    
    eventMask = NSAnyEventMask;
     
    while ( ( event = [ NSApp nextEventMatchingMask: eventMask
							  untilDate: distantPast
							  inMode: NSDefaultRunLoopMode
							  dequeue:YES ] ) ) {
		processEvent( event );
	}
}

void Sys_PreventMouseMovement( CGPoint point ) {
    CGEventErr err;

    //common->Printf( "**** Calling CGAssociateMouseAndMouseCursorPosition(false)\n" );
    err = CGAssociateMouseAndMouseCursorPosition( false );
    if ( err != CGEventNoErr ) {
        common->Error( "Could not disable mouse movement, CGAssociateMouseAndMouseCursorPosition returned %d\n", err );
    }

    // Put the mouse in the position we want to leave it at
    err = CGWarpMouseCursorPosition( point );
    if ( err != CGEventNoErr ) {
        common->Error( "Could not disable mouse movement, CGWarpMouseCursorPosition returned %d\n", err );
    }
}

void Sys_ReenableMouseMovement() {
    CGEventErr err;

    //common->Printf( "**** Calling CGAssociateMouseAndMouseCursorPosition(true)\n" );
    err = CGAssociateMouseAndMouseCursorPosition( true );
    if ( err != CGEventNoErr ) {
        common->Error( "Could not reenable mouse movement, CGAssociateMouseAndMouseCursorPosition returned %d\n", err );
    }

    // Leave the mouse where it was -- don't warp here.
}

void Sys_LockMouseInInputRect(CGRect rect) {
    CGPoint center;

    center.x = rect.origin.x + rect.size.width / 2.0;
    center.y = rect.origin.y + rect.size.height / 2.0;

    // Now, put the mouse in the middle of the input rect (anywhere over it would do)
    // and don't allow it to move.  This means that the user won't be able to accidentally
    // select another application.
    Sys_PreventMouseMovement(center);
}

void Sys_SetMouseInputRect(CGRect newRect) {
    inputRectValid = YES;
    inputRect = newRect;

    if ( mouseActive ) {
        Sys_LockMouseInInputRect( inputRect );
	}
}

void IN_ActivateMouse( void ) {
    if ( mouseActive ) {
        return;
    }
    if ( inputRectValid ) {
        // Make sure that if window moved we don't hose the user...
        Sys_UpdateWindowMouseInputRect();
    }
    Sys_LockMouseInInputRect( inputRect );
    CGDisplayHideCursor( Sys_DisplayToUse() );
    mouseActive = true;
}

void IN_DeactivateMouse( void ) {
    if ( !mouseActive ) {
        return;
    }
    Sys_ReenableMouseMovement();
    CGDisplayShowCursor( Sys_DisplayToUse() );
    mouseActive = false;
}

/*
===============
Sys_MapCharForKey
===============
*/
unsigned char Sys_MapCharForKey( int key ) {
	return (unsigned char)key;
}

/*
 ===============
 Sys_GetConsoleKey
 ===============
 */
unsigned char Sys_GetConsoleKey( bool shifted ) {
	if ( vkeyTable == vkeyToDoom3Key_French ) {
		return shifted ? '>' : '<';
	}
	else {
		return shifted ? '~' : '`';
	}

}

