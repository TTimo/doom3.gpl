#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#ifndef GL_EXT_abgr
#include <OpenGL/glext.h>
#endif

// This can be defined to use the CGLMacro.h support which avoids looking up
// the current context.
//#define USE_CGLMACROS

#ifdef USE_CGLMACROS
#include "macosx_local.h"
#define cgl_ctx glw_state._cgl_ctx
#include <OpenGL/CGLMacro.h>
#endif

