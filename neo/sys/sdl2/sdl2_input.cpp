#include "../../idlib/precompiled.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_mouse.h" // for SDL_SetRelativeMouseMode

typedef struct poll_keyboard_event_s
{
	int key;
	bool state;
} poll_keyboard_event_t;

typedef struct poll_mouse_event_s
{
	int action;
	int value;
} poll_mouse_event_t;

#define MAX_POLL_EVENTS 50
#define POLL_EVENTS_HEADROOM 2 // some situations require to add several events

extern int g_Quit;

static poll_keyboard_event_t poll_events_keyboard[MAX_POLL_EVENTS + POLL_EVENTS_HEADROOM];
static int poll_keyboard_event_count;
static poll_mouse_event_t poll_events_mouse[MAX_POLL_EVENTS + POLL_EVENTS_HEADROOM];
static int poll_mouse_event_count;

void SDL_QueEvent( sysEventType_t type, int value, int value2, int ptrLength, void *ptr );

#define K_UNKNOWN K_LAST_KEY
#define K_PAUSE   K_LAST_KEY
#define K_NUMLOCKCLEAR K_LAST_KEY
#define K_NONUSBACKSLASH K_LAST_KEY
#define K_APPLICATION K_LAST_KEY

int	vkeyToDoom3Key_US[256] = {
	/*0x00*/	K_UNKNOWN, K_UNKNOWN, K_UNKNOWN, K_UNKNOWN, 'a', 'b', 'c', 'd',
	/*0x08*/	'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
	/*0x10*/	'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
	/*0x18*/	'u', 'v', 'w', 'x', 'y', 'z', '1', '2',
	/*0x20*/	'3', '4', '5', '6', '7', '8', '9', '0',
	/*0x28*/	K_ENTER, K_ESCAPE, K_BACKSPACE, K_TAB, K_SPACE, '-', '=', '(',
	/*0x30*/	')', '/', ' ', ';', '\'', '`', ',', '.',
	/*0x38*/	'/', K_CAPSLOCK, K_F1, K_F2, K_F3, K_F4, K_F5, K_F6,
	/*0x40*/	K_F7, K_F8, K_F9, K_F10, K_F11, K_F12, K_PRINT_SCR, /*SCROLL_LOCK*/' ',
	/*0x48*/	K_PAUSE, K_INS, K_HOME, K_PGUP, K_DEL, K_END, K_PGDN, K_RIGHTARROW,
	/*0x50*/    K_LEFTARROW, K_DOWNARROW, K_UPARROW, K_NUMLOCKCLEAR, K_KP_SLASH, K_KP_STAR, K_KP_MINUS,
	/*0x58*/	K_KP_PLUS, K_KP_ENTER, K_KP_END, K_KP_DOWNARROW, K_KP_END, K_KP_LEFTARROW, K_KP_5, K_KP_RIGHTARROW,
	/*0x60*/	K_KP_HOME, K_KP_UPARROW, K_KP_PGUP, K_KP_INS, K_KP_DEL, K_NONUSBACKSLASH, K_APPLICATION, K_POWER,
	/*0x68*/	K_KP_EQUALS, K_F13, K_F14, K_F15, 0, 0, 0, 0,
	/*0x70*/	0, 0, 0, 0, 0, 0, 0, 0,
	/*0x78*/	0, 0, 0, 0, 0, 0, 0, 0,
};

static const int *vkeyTable = vkeyToDoom3Key_US;

/*
 ==========
 SDL_AddKeyboardPollEvent
 ==========
 */
bool SDL_AddKeyboardPollEvent(int key, bool state) {
	if (poll_keyboard_event_count >= MAX_POLL_EVENTS + POLL_EVENTS_HEADROOM)
		common->FatalError( "poll_keyboard_event_count exceeded MAX_POLL_EVENT + POLL_EVENTS_HEADROOM\n");
	poll_events_keyboard[poll_keyboard_event_count].key = key;
	poll_events_keyboard[poll_keyboard_event_count++].state = state;
	if (poll_keyboard_event_count >= MAX_POLL_EVENTS) {
		common->DPrintf("WARNING: reached MAX_POLL_EVENT poll_keyboard_event_count\n");
		return false;
	}
	return true;
}

bool SDL_AddMousePollEvent(int action, int value) {
	if (poll_mouse_event_count >= MAX_POLL_EVENTS + POLL_EVENTS_HEADROOM)
		common->FatalError( "poll_mouse_event_count exceeded MAX_POLL_EVENT + POLL_EVENTS_HEADROOM\n");
	poll_events_mouse[poll_mouse_event_count].action = action;
	poll_events_mouse[poll_mouse_event_count++].value = value;
	if (poll_mouse_event_count >= MAX_POLL_EVENTS) {
		common->DPrintf("WARNING: reached MAX_POLL_EVENT poll_mouse_event_count\n");
		return false;
	}
	return true;
}


void Sys_InitInput( void )
{
	SDL_SetRelativeMouseMode(SDL_TRUE);
}

void Sys_ShutdownInput( void )
{
	SDL_SetRelativeMouseMode(SDL_FALSE);
}

void Sys_InitScanTable( void )
{
	idStr lang = cvarSystem->GetCVarString( "sys_lang" );
	if ( lang.Length() == 0 ) {
		lang = "english";
	}
	
	if ( lang.Icmp( "english" ) == 0 )
		vkeyTable = vkeyToDoom3Key_US;
/*
	else if ( lang.Icmp( "french" ) == 0 )
		vkeyTable = vkeyToDoom3Key_French;
	else if ( lang.Icmp( "german" ) == 0 )
		vkeyTable = vkeyToDoom3Key_German;
*/
}

void SDL_PollEvents( void )
{
	SDL_Event sdlevent;
	if( SDL_PollEvent( &sdlevent ))
	{
		switch( sdlevent.type )
		{
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				{
					bool keyDownFlag = sdlevent.type == SDL_KEYDOWN;

					SDL_Keysym keysym = sdlevent.key.keysym;
					SDL_Scancode keycode = keysym.scancode;
					int doomKey = vkeyTable[keycode];
					SDL_QueEvent( SE_KEY, doomKey, keyDownFlag, 0, NULL );
					if( keyDownFlag )
						SDL_QueEvent( SE_CHAR, doomKey, 0, 0, NULL);
					SDL_AddKeyboardPollEvent( doomKey, keyDownFlag );
				}
				break;
			case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEBUTTONDOWN:
				{
					int updown = ( sdlevent.type == SDL_MOUSEBUTTONDOWN );
					int button;
					switch( sdlevent.button.button )
					{
						default:
						case SDL_BUTTON_LEFT:
							button = K_MOUSE1;
							break;
						case SDL_BUTTON_RIGHT:
							button = K_MOUSE2;
							break;
					}
					SDL_QueEvent( SE_KEY, button, updown, 0, NULL);
				}
				break;
			case SDL_MOUSEMOTION:
				SDL_QueEvent( SE_MOUSE, sdlevent.motion.xrel, sdlevent.motion.yrel, 0, NULL);
				// not sure if these are even used (not in main menu at any rate)
				SDL_AddMousePollEvent( M_DELTAX, sdlevent.motion.xrel );
				SDL_AddMousePollEvent( M_DELTAY, sdlevent.motion.yrel );
				break;
			case SDL_QUIT:
				g_Quit = true;
			default:
				break;
		}
		SDL_Event *event = &sdlevent;
		if (event->type == SDL_WINDOWEVENT) {
			switch (event->window.event) {
				case SDL_WINDOWEVENT_SHOWN:
					SDL_Log("Window %d shown", event->window.windowID);
					break;
				case SDL_WINDOWEVENT_HIDDEN:
					SDL_Log("Window %d hidden", event->window.windowID);
					break;
				case SDL_WINDOWEVENT_EXPOSED:
					SDL_Log("Window %d exposed", event->window.windowID);
					break;
				case SDL_WINDOWEVENT_MOVED:
					SDL_Log("Window %d moved to %d,%d",
							event->window.windowID, event->window.data1,
							event->window.data2);
					break;
				case SDL_WINDOWEVENT_RESIZED:
					SDL_Log("Window %d resized to %dx%d",
							event->window.windowID, event->window.data1,
							event->window.data2);
					break;
				case SDL_WINDOWEVENT_MINIMIZED:
					SDL_Log("Window %d minimized", event->window.windowID);
					break;
				case SDL_WINDOWEVENT_MAXIMIZED:
					SDL_Log("Window %d maximized", event->window.windowID);
					break;
				case SDL_WINDOWEVENT_RESTORED:
					SDL_Log("Window %d restored", event->window.windowID);
					break;
				case SDL_WINDOWEVENT_ENTER:
					SDL_Log("Mouse entered window %d",
							event->window.windowID);
					break;
				case SDL_WINDOWEVENT_LEAVE:
					SDL_Log("Mouse left window %d", event->window.windowID);
					break;
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					SDL_Log("Window %d gained keyboard focus",
							event->window.windowID);
					break;
				case SDL_WINDOWEVENT_FOCUS_LOST:
					SDL_Log("Window %d lost keyboard focus",
							event->window.windowID);
					break;
				case SDL_WINDOWEVENT_CLOSE:
					SDL_Log("Window %d closed", event->window.windowID);
					break;
				default:
					SDL_Log("Window %d got unknown event %d",
							event->window.windowID, event->window.event);
					break;
			}
		}
	}
}

int Sys_PollKeyboardInputEvents( void )
{
	return poll_keyboard_event_count;
}

int Sys_ReturnKeyboardInputEvent( const int n, int &key, bool &state )
{
	if ( n >= poll_keyboard_event_count )
		return 0;
	key = poll_events_keyboard[n].key;
	state = poll_events_keyboard[n].state;
	return 1;
}

void Sys_EndKeyboardInputEvents( void )
{
	//isn't this were it's supposed to be, was missing some key strokes with it set below
	poll_keyboard_event_count = 0;
}

void Sys_GrabMouseCursor( bool grabIt )
{
}

int Sys_PollMouseInputEvents( void )
{
	SDL_PollEvents();
	return poll_mouse_event_count;
}

int Sys_ReturnMouseInputEvent( const int n, int &action, int &value )
{
	if ( n>=poll_mouse_event_count )
		return 0;
	action = poll_events_mouse[ n ].action;
	value = poll_events_mouse[ n ].value;
	return 1;
}

void Sys_EndMouseInputEvents( void )
{
	poll_mouse_event_count = 0;
}

unsigned char Sys_MapCharForKey( int key )
{
	return (unsigned char)key;
}

unsigned char Sys_GetConsoleKey( bool shifted )
{
	return shifted ? '~' : '`';
}

#define	MAX_QUED_EVENTS		256
#define	MASK_QUED_EVENTS	( MAX_QUED_EVENTS - 1 )

static sysEvent_t eventQue[MAX_QUED_EVENTS];
static int eventHead, eventTail;

/*
 ================
 Posix_QueEvent
 
 ptr should either be null, or point to a block of data that can be freed later
 ================
 */
void SDL_QueEvent( sysEventType_t type, int value, int value2,
					int ptrLength, void *ptr ) {
	sysEvent_t *ev;
	
	ev = &eventQue[eventHead & MASK_QUED_EVENTS];
	if (eventHead - eventTail >= MAX_QUED_EVENTS) {
		common->Printf( "Posix_QueEvent: overflow\n" );
		// we are discarding an event, but don't leak memory
		// TTimo: verbose dropped event types?
		if (ev->evPtr) {
			Mem_Free(ev->evPtr);
			ev->evPtr = NULL;
		}
		eventTail++;
	}
	
	eventHead++;
	
	ev->evType = type;
	ev->evValue = value;
	ev->evValue2 = value2;
	ev->evPtrLength = ptrLength;
	ev->evPtr = ptr;
	
#if 0
	common->Printf( "Event %d: %d %d\n", ev->evType, ev->evValue, ev->evValue2 );
#endif
}

void Sys_ClearEvents( void )
{
	eventHead = eventTail = 0;
}

sysEvent_t Sys_GetEvent( void )
{
	static sysEvent_t ev;
	
	// return if we have data
	if (eventHead > eventTail) {
		eventTail++;
		return eventQue[(eventTail - 1) & MASK_QUED_EVENTS];
	}

	// return the empty event with the current time
	memset(&ev, 0, sizeof(ev));
	
	return ev;
}

/*
 ================
 terminal support utilities
 ================
 */

#include <unistd.h>
#define					COMMAND_HISTORY 64

static bool				tty_enabled = false;
static int				input_hide = 0;

idEditField				input_field;
static char				input_ret[256];

static idStr			history[ COMMAND_HISTORY ];	// cycle buffer
static int				history_count = 0;			// buffer fill up
static int				history_start = 0;			// current history start
static int				history_current = 0;			// goes back in history
idEditField				history_backup;				// the base edit line

void tty_Del() {
	char key;
	key = '\b';
	write( STDOUT_FILENO, &key, 1 );
	key = ' ';
	write( STDOUT_FILENO, &key, 1 );
	key = '\b';
	write( STDOUT_FILENO, &key, 1 );
}

void tty_Left() {
	char key = '\b';
	write( STDOUT_FILENO, &key, 1 );
}

void tty_Right() {
	char key = 27;
	write( STDOUT_FILENO, &key, 1 );
	write( STDOUT_FILENO, "[C", 2 );
}

// clear the display of the line currently edited
// bring cursor back to beginning of line
void tty_Hide() {
	int len, buf_len;
	if ( !tty_enabled ) {
		return;
	}
	if ( input_hide ) {
		input_hide++;
		return;
	}
	// clear after cursor
	len = strlen( input_field.GetBuffer() ) - input_field.GetCursor();
	while ( len > 0 ) {
		tty_Right();
		len--;
	}
	buf_len = strlen( input_field.GetBuffer() );
	while ( buf_len > 0 ) {
		tty_Del();
		buf_len--;
	}
	input_hide++;
}

// show the current line
void tty_Show() {
	//	int i;
	if ( !tty_enabled ) {
		return;
	}
	assert( input_hide > 0 );
	input_hide--;
	if ( input_hide == 0 ) {
		char *buf = input_field.GetBuffer();
		if ( buf[0] ) {
			write( STDOUT_FILENO, buf, strlen( buf ) );
			int back = strlen( buf ) - input_field.GetCursor();
			while ( back > 0 ) {
				tty_Left();
				back--;
			}
		}
	}
}

void tty_FlushIn() {
	char key;
	while ( read(0, &key, 1) != -1 ) {
		Sys_Printf( "'%d' ", key );
	}
	Sys_Printf( "\n" );
}

/*
 ================
 Posix_ConsoleInput
 Checks for a complete line of text typed in at the console.
 Return NULL if a complete line is not ready.
 ================
 */
char *SDL_ConsoleInput( void ) {
	if ( tty_enabled ) {
		int		ret;
		char	key;
		bool	hidden = false;
		while ( ( ret = read( STDIN_FILENO, &key, 1 ) ) > 0 ) {
			if ( !hidden ) {
				tty_Hide();
				hidden = true;
			}
			switch ( key ) {
				case 1:
					input_field.SetCursor( 0 );
					break;
				case 5:
					input_field.SetCursor( strlen( input_field.GetBuffer() ) );
					break;
				case 127:
				case 8:
					input_field.CharEvent( K_BACKSPACE );
					break;
				case '\n':
					idStr::Copynz( input_ret, input_field.GetBuffer(), sizeof( input_ret ) );
					assert( hidden );
					tty_Show();
					write( STDOUT_FILENO, &key, 1 );
					input_field.Clear();
					if ( history_count < COMMAND_HISTORY ) {
						history[ history_count ] = input_ret;
						history_count++;
					} else {
						history[ history_start ] = input_ret;
						history_start++;
						history_start %= COMMAND_HISTORY;
					}
					history_current = 0;
					return input_ret;
				case '\t':
					input_field.AutoComplete();
					break;
				case 27: {
					// enter escape sequence mode
					ret = read( STDIN_FILENO, &key, 1 );
					if ( ret <= 0 ) {
						Sys_Printf( "dropping sequence: '27' " );
						tty_FlushIn();
						assert( hidden );
						tty_Show();
						return NULL;
					}
					switch ( key ) {
						case 79:
							ret = read( STDIN_FILENO, &key, 1 );
							if ( ret <= 0 ) {
								Sys_Printf( "dropping sequence: '27' '79' " );
								tty_FlushIn();
								assert( hidden );
								tty_Show();
								return NULL;
							}
							switch ( key ) {
								case 72:
									// xterm only
									input_field.SetCursor( 0 );
									break;
								case 70:
									// xterm only
									input_field.SetCursor( strlen( input_field.GetBuffer() ) );
									break;
								default:
									Sys_Printf( "dropping sequence: '27' '79' '%d' ", key );
									tty_FlushIn();
									assert( hidden );
									tty_Show();
									return NULL;
							}
							break;
						case 91: {
							ret = read( STDIN_FILENO, &key, 1 );
							if ( ret <= 0 ) {
								Sys_Printf( "dropping sequence: '27' '91' " );
								tty_FlushIn();
								assert( hidden );
								tty_Show();
								return NULL;
							}
							switch ( key ) {
								case 49: {
									ret = read( STDIN_FILENO, &key, 1 );
									if ( ret <= 0 || key != 126 ) {
										Sys_Printf( "dropping sequence: '27' '91' '49' '%d' ", key );
										tty_FlushIn();
										assert( hidden );
										tty_Show();
										return NULL;
									}
									// only screen and linux terms
									input_field.SetCursor( 0 );
									break;
								}
								case 50: {
									ret = read( STDIN_FILENO, &key, 1 );
									if ( ret <= 0 || key != 126 ) {
										Sys_Printf( "dropping sequence: '27' '91' '50' '%d' ", key );
										tty_FlushIn();
										assert( hidden );
										tty_Show();
										return NULL;
									}
									// all terms
									input_field.KeyDownEvent( K_INS );
									break;
								}
								case 52: {
									ret = read( STDIN_FILENO, &key, 1 );
									if ( ret <= 0 || key != 126 ) {
										Sys_Printf( "dropping sequence: '27' '91' '52' '%d' ", key );
										tty_FlushIn();
										assert( hidden );
										tty_Show();
										return NULL;
									}
									// only screen and linux terms
									input_field.SetCursor( strlen( input_field.GetBuffer() ) );
									break;
								}
								case 51: {
									ret = read( STDIN_FILENO, &key, 1 );
									if ( ret <= 0 ) {
										Sys_Printf( "dropping sequence: '27' '91' '51' " );
										tty_FlushIn();
										assert( hidden );
										tty_Show();
										return NULL;
									}
									if ( key == 126 ) {
										input_field.KeyDownEvent( K_DEL );
										break;
									}
									Sys_Printf( "dropping sequence: '27' '91' '51' '%d'", key );
									tty_FlushIn();
									assert( hidden );
									tty_Show();
									return NULL;
								}
								case 65:
								case 66: {
									// history
									if ( history_current == 0 ) {
										history_backup = input_field;
									}
									if ( key == 65 ) {
										// up
										history_current++;
									} else {
										// down
										history_current--;
									}
									// history_current cycle:
									// 0: current edit
									// 1 .. Min( COMMAND_HISTORY, history_count ): back in history
									if ( history_current < 0 ) {
										history_current = Min( COMMAND_HISTORY, history_count );
									} else {
										history_current %= Min( COMMAND_HISTORY, history_count ) + 1;
									}
									int index = -1;
									if ( history_current == 0 ) {
										input_field = history_backup;
									} else {
										index = history_start + Min( COMMAND_HISTORY, history_count ) - history_current;
										index %= COMMAND_HISTORY;
										assert( index >= 0 && index < COMMAND_HISTORY );
										input_field.SetBuffer( history[ index ] );
									}
									assert( hidden );
									tty_Show();
									return NULL;
								}
								case 67:
									input_field.KeyDownEvent( K_RIGHTARROW );
									break;
								case 68:
									input_field.KeyDownEvent( K_LEFTARROW );
									break;
								default:
									Sys_Printf( "dropping sequence: '27' '91' '%d' ", key );
									tty_FlushIn();
									assert( hidden );
									tty_Show();
									return NULL;
							}
							break;
						}
						default:
							Sys_Printf( "dropping sequence: '27' '%d' ", key );
							tty_FlushIn();
							assert( hidden );
							tty_Show();
							return NULL;
					}
					break;
				}
				default:
					if ( key >= ' ' ) {
						input_field.CharEvent( key );
						break;
					}
					Sys_Printf( "dropping sequence: '%d' ", key );
					tty_FlushIn();
					assert( hidden );
					tty_Show();
					return NULL;
			}
		}
		if ( hidden ) {
			tty_Show();
		}
		return NULL;
	} else {
		// disabled on OSX. works fine from a terminal, but launching from Finder is causing trouble
		// I'm pretty sure it could be re-enabled if needed, and just handling the Finder failure case right (TTimo)
#ifndef MACOS_X
		// no terminal support - read only complete lines
		int				len;
		fd_set			fdset;
		struct timeval	timeout;
		
		FD_ZERO( &fdset );
		FD_SET( STDIN_FILENO, &fdset );
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		if ( select( 1, &fdset, NULL, NULL, &timeout ) == -1 || !FD_ISSET( 0, &fdset ) ) {
			return NULL;
		}
		
		len = read( 0, input_ret, sizeof( input_ret ) );
		if ( len == 0 ) {
			// EOF
			return NULL;
		}
		
		if ( len < 1 ) {
			Sys_Printf( "read failed: %s\n", strerror( errno ) );	// something bad happened, cancel this line and print an error
			return NULL;
		}
		
		if ( len == sizeof( input_ret ) ) {
			Sys_Printf( "read overflow\n" );	// things are likely to break, as input will be cut into pieces
		}
		
		input_ret[ len-1 ] = '\0';		// rip off the \n and terminate
		return input_ret;
#endif
	}
	return NULL;
}

void Sys_GenerateEvents( void )
{
	char *s;
	if ( ( s = SDL_ConsoleInput() ) ) {
		char *b;
		int len;
		
		len = strlen( s ) + 1;
		b = (char *)Mem_Alloc( len );
		strcpy( b, s );
		SDL_QueEvent( SE_CONSOLE, 0, 0, len, b );
	}

}
