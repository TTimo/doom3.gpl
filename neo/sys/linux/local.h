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
#ifndef __LINUX_LOCAL_H__
#define __LINUX_LOCAL_H__

extern glconfig_t glConfig;

// glimp.cpp

//#define ID_ENABLE_DGA

#if defined( ID_ENABLE_DGA )
#include <X11/extensions/xf86dga.h>
#endif
#include <X11/extensions/xf86vmode.h>
#include <X11/XKBlib.h>

extern Display *dpy;
extern Window win;

// input.cpp
extern bool dga_found;
void Sys_XEvents();
void Sys_XUninstallGrabs();

#define KEY_MASK (KeyPressMask | KeyReleaseMask)
#define MOUSE_MASK (ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ButtonMotionMask )
#define X_MASK (KEY_MASK | MOUSE_MASK | VisibilityChangeMask | StructureNotifyMask )

#ifndef ID_GL_HARDLINK
bool GLimp_dlopen();
void GLimp_dlclose();

void GLimp_BindLogging();
void GLimp_BindNative();
#endif

#endif
