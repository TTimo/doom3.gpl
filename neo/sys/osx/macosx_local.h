
#include <sys/wait.h>

#include "../sys_public.h"

void	OutputDebugString( const char *text );

// input
void 	Sys_InitInput( void );
void 	Sys_ShutdownInput( void );

void	IN_DeactivateMouse( void);
void	IN_ActivateMouse( void);

void	IN_Activate (bool active);
void	IN_Frame (void);

void * wglGetProcAddress(const char *name);

void	Sleep( const int time );

void	Sys_UpdateWindowMouseInputRect( void );
