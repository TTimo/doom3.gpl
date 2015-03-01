#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../../renderer/tr_local.h"
#include "SDL2/SDL.h"

SDL_Window *mainwindow = NULL;
SDL_GLContext maincontext;

bool GLimp_Init( glimpParms_t parms ) 
{
	// this function should create the main window, the associated gl context and fills in the glConfig global
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		
	Uint32 Flags = SDL_WINDOW_OPENGL;
	Uint32 Width  = parms.width;
	Uint32 Height = parms.height;
	if( parms.fullScreen )
	{
#if 0
		// OSX seems to be headed down the fullscreen desktop path
		Flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
#else
		Flags |= SDL_WINDOW_FULLSCREEN;
#endif
		Width = 0;
		Height = 0;
	}
	mainwindow = SDL_CreateWindow( "Doom 3", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Width, Height, Flags );
    if( mainwindow == NULL )
        return false;
	maincontext = SDL_GL_CreateContext(mainwindow);
	
	// get our config strings
	glConfig.vendor_string = (const char *)qglGetString( GL_VENDOR );
	glConfig.renderer_string = (const char *)qglGetString( GL_RENDERER );
	glConfig.version_string = (const char *)qglGetString( GL_VERSION );
	glConfig.extensions_string = (const char *)qglGetString( GL_EXTENSIONS );
	
	glConfig.winWidth = parms.width;
	glConfig.winHeight = parms.height;
	glConfig.vidWidth = parms.width;
	glConfig.vidHeight = parms.height;
	glConfig.isFullscreen = parms.fullScreen;
	if( Flags & SDL_WINDOW_FULLSCREEN_DESKTOP )
	{
#if 0
		// this should work, but doesn't
		SDL_Renderer *renderer = SDL_GetRenderer( mainwindow );
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		SDL_RenderSetLogicalSize( renderer, parms.width, parms.height);
#else
		// this works, but ignores the set resolution
		SDL_DisplayMode mode;
		int index = SDL_GetWindowDisplayIndex(mainwindow);
		SDL_GetCurrentDisplayMode( index, &mode );
		glConfig.winWidth = mode.w;
		glConfig.winHeight = mode.h;
#endif
	}
	
	SDL_GL_GetAttribute( SDL_GL_BUFFER_SIZE, &glConfig.colorBits );
	SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &glConfig.depthBits );
	SDL_GL_GetAttribute( SDL_GL_STENCIL_SIZE, &glConfig.stencilBits );
	glConfig.displayFrequency = 0; // FIXME

	// draw something to show that GL is alive
	qglClearColor( 0.5, 0.5, 0.7, 0 );
	qglClear( GL_COLOR_BUFFER_BIT );
	GLimp_SwapBuffers();
	
	qglClearColor( 0.5, 0.5, 0.7, 0 );
	qglClear( GL_COLOR_BUFFER_BIT );
	GLimp_SwapBuffers();

	return true;
}

void GLimp_Shutdown( void )
{
	SDL_GL_DeleteContext(maincontext);
    SDL_DestroyWindow(mainwindow);
}

GLExtension_t GLimp_ExtensionPointer( const char *name )
{
	return (GLExtension_t)SDL_GL_GetProcAddress(name);
}

void GLimp_SetGamma( unsigned short red[256], unsigned short green[256], unsigned short blue[256] )
{
	if( !mainwindow )
		return;
	if( SDL_SetWindowGammaRamp( mainwindow, red, green, blue ) == -1 )
		common->DPrintf( "SetGamma not supported on this hardware\n" );
	
}

void GLimp_EnableLogging( bool enable )
{
	// this function is empty on OSX build also
}

void GLimp_SwapBuffers( void )
{
	SDL_GL_SwapWindow(mainwindow);
}

void GLimp_ActivateContext( void )
{
	// this doesn't ever seem to be called
}

void GLimp_DeactivateContext( void )
{
	// this doesn't ever seem to be called
}

bool GLimp_SetScreenParms( glimpParms_t parms )
{
	common->DPrintf( "TODO: __GLimp_SetScreenParms\n");
	return false;
}
