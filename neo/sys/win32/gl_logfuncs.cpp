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

static void APIENTRY logAccum(GLenum op, GLfloat value) {
	fprintf( tr.logFile, "glAccum %s %g\n", EnumString(op), value );
	dllAccum(op, value);
}

static void APIENTRY logAlphaFunc(GLenum func, GLclampf ref) {
	fprintf( tr.logFile, "glAlphaFunc %s %g\n", EnumString(func), ref );
	dllAlphaFunc(func, ref);
}

static GLboolean APIENTRY logAreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences) {
// unknown type: "const GLuint *" name: "textures"
// unknown type: "GLboolean *" name: "residences"
	fprintf( tr.logFile, "glAreTexturesResident %d 'const GLuint * textures' 'GLboolean * residences'\n", n );
	return dllAreTexturesResident(n, textures, residences);
}

static void APIENTRY logArrayElement(GLint i) {
	fprintf( tr.logFile, "glArrayElement %d\n", i );
	dllArrayElement(i);
}

static void APIENTRY logBegin(GLenum mode) {
	fprintf( tr.logFile, "glBegin %s\n", EnumString(mode) );
	dllBegin(mode);
}

static void APIENTRY logBindTexture(GLenum target, GLuint texture) {
	fprintf( tr.logFile, "glBindTexture %s %d\n", EnumString(target), texture );
	dllBindTexture(target, texture);
}

static void APIENTRY logBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap) {
// unknown type: "const GLubyte *" name: "bitmap"
	fprintf( tr.logFile, "glBitmap %d %d %g %g %g %g 'const GLubyte * bitmap'\n", width, height, xorig, yorig, xmove, ymove );
	dllBitmap(width, height, xorig, yorig, xmove, ymove, bitmap);
}

static void APIENTRY logBlendFunc(GLenum sfactor, GLenum dfactor) {
	fprintf( tr.logFile, "glBlendFunc %s %s\n", EnumString(sfactor), EnumString(dfactor) );
	dllBlendFunc(sfactor, dfactor);
}

static void APIENTRY logCallList(GLuint list) {
	fprintf( tr.logFile, "glCallList %d\n", list );
	dllCallList(list);
}

static void APIENTRY logCallLists(GLsizei n, GLenum type, const GLvoid *lists) {
// unknown type: "const GLvoid *" name: "lists"
	fprintf( tr.logFile, "glCallLists %d %s 'const GLvoid * lists'\n", n, EnumString(type) );
	dllCallLists(n, type, lists);
}

static void APIENTRY logClear(GLbitfield mask) {
// unknown type: "GLbitfield" name: "mask"
	fprintf( tr.logFile, "glClear 'GLbitfield mask'\n" );
	dllClear(mask);
}

static void APIENTRY logClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
	fprintf( tr.logFile, "glClearAccum %g %g %g %g\n", red, green, blue, alpha );
	dllClearAccum(red, green, blue, alpha);
}

static void APIENTRY logClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
	fprintf( tr.logFile, "glClearColor %g %g %g %g\n", red, green, blue, alpha );
	dllClearColor(red, green, blue, alpha);
}

static void APIENTRY logClearDepth(GLclampd depth) {
// unknown type: "GLclampd" name: "depth"
	fprintf( tr.logFile, "glClearDepth 'GLclampd depth'\n" );
	dllClearDepth(depth);
}

static void APIENTRY logClearIndex(GLfloat c) {
	fprintf( tr.logFile, "glClearIndex %g\n", c );
	dllClearIndex(c);
}

static void APIENTRY logClearStencil(GLint s) {
	fprintf( tr.logFile, "glClearStencil %d\n", s );
	dllClearStencil(s);
}

static void APIENTRY logClipPlane(GLenum plane, const GLdouble *equation) {
// unknown type: "const GLdouble *" name: "equation"
	fprintf( tr.logFile, "glClipPlane %s 'const GLdouble * equation'\n", EnumString(plane) );
	dllClipPlane(plane, equation);
}

static void APIENTRY logColor3b(GLbyte red, GLbyte green, GLbyte blue) {
	fprintf( tr.logFile, "glColor3b %d %d %d\n", red, green, blue );
	dllColor3b(red, green, blue);
}

static void APIENTRY logColor3bv(const GLbyte *v) {
// unknown type: "const GLbyte *" name: "v"
	fprintf( tr.logFile, "glColor3bv 'const GLbyte * v'\n" );
	dllColor3bv(v);
}

static void APIENTRY logColor3d(GLdouble red, GLdouble green, GLdouble blue) {
	fprintf( tr.logFile, "glColor3d %g %g %g\n", red, green, blue );
	dllColor3d(red, green, blue);
}

static void APIENTRY logColor3dv(const GLdouble *v) {
// unknown type: "const GLdouble *" name: "v"
	fprintf( tr.logFile, "glColor3dv 'const GLdouble * v'\n" );
	dllColor3dv(v);
}

static void APIENTRY logColor3f(GLfloat red, GLfloat green, GLfloat blue) {
	fprintf( tr.logFile, "glColor3f %g %g %g\n", red, green, blue );
	dllColor3f(red, green, blue);
}

static void APIENTRY logColor3fv(const GLfloat *v) {
// unknown type: "const GLfloat *" name: "v"
	fprintf( tr.logFile, "glColor3fv 'const GLfloat * v'\n" );
	dllColor3fv(v);
}

static void APIENTRY logColor3i(GLint red, GLint green, GLint blue) {
	fprintf( tr.logFile, "glColor3i %d %d %d\n", red, green, blue );
	dllColor3i(red, green, blue);
}

static void APIENTRY logColor3iv(const GLint *v) {
// unknown type: "const GLint *" name: "v"
	fprintf( tr.logFile, "glColor3iv 'const GLint * v'\n" );
	dllColor3iv(v);
}

static void APIENTRY logColor3s(GLshort red, GLshort green, GLshort blue) {
	fprintf( tr.logFile, "glColor3s %d %d %d\n", red, green, blue );
	dllColor3s(red, green, blue);
}

static void APIENTRY logColor3sv(const GLshort *v) {
// unknown type: "const GLshort *" name: "v"
	fprintf( tr.logFile, "glColor3sv 'const GLshort * v'\n" );
	dllColor3sv(v);
}

static void APIENTRY logColor3ub(GLubyte red, GLubyte green, GLubyte blue) {
	fprintf( tr.logFile, "glColor3ub %d %d %d\n", red, green, blue );
	dllColor3ub(red, green, blue);
}

static void APIENTRY logColor3ubv(const GLubyte *v) {
// unknown type: "const GLubyte *" name: "v"
	fprintf( tr.logFile, "glColor3ubv 'const GLubyte * v'\n" );
	dllColor3ubv(v);
}

static void APIENTRY logColor3ui(GLuint red, GLuint green, GLuint blue) {
	fprintf( tr.logFile, "glColor3ui %d %d %d\n", red, green, blue );
	dllColor3ui(red, green, blue);
}

static void APIENTRY logColor3uiv(const GLuint *v) {
// unknown type: "const GLuint *" name: "v"
	fprintf( tr.logFile, "glColor3uiv 'const GLuint * v'\n" );
	dllColor3uiv(v);
}

static void APIENTRY logColor3us(GLushort red, GLushort green, GLushort blue) {
	fprintf( tr.logFile, "glColor3us %d %d %d\n", red, green, blue );
	dllColor3us(red, green, blue);
}

static void APIENTRY logColor3usv(const GLushort *v) {
// unknown type: "const GLushort *" name: "v"
	fprintf( tr.logFile, "glColor3usv 'const GLushort * v'\n" );
	dllColor3usv(v);
}

static void APIENTRY logColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha) {
	fprintf( tr.logFile, "glColor4b %d %d %d %d\n", red, green, blue, alpha );
	dllColor4b(red, green, blue, alpha);
}

static void APIENTRY logColor4bv(const GLbyte *v) {
// unknown type: "const GLbyte *" name: "v"
	fprintf( tr.logFile, "glColor4bv 'const GLbyte * v'\n" );
	dllColor4bv(v);
}

static void APIENTRY logColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha) {
	fprintf( tr.logFile, "glColor4d %g %g %g %g\n", red, green, blue, alpha );
	dllColor4d(red, green, blue, alpha);
}

static void APIENTRY logColor4dv(const GLdouble *v) {
// unknown type: "const GLdouble *" name: "v"
	fprintf( tr.logFile, "glColor4dv 'const GLdouble * v'\n" );
	dllColor4dv(v);
}

static void APIENTRY logColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
	fprintf( tr.logFile, "glColor4f %g %g %g %g\n", red, green, blue, alpha );
	dllColor4f(red, green, blue, alpha);
}

static void APIENTRY logColor4fv(const GLfloat *v) {
// unknown type: "const GLfloat *" name: "v"
	fprintf( tr.logFile, "glColor4fv 'const GLfloat * v'\n" );
	dllColor4fv(v);
}

static void APIENTRY logColor4i(GLint red, GLint green, GLint blue, GLint alpha) {
	fprintf( tr.logFile, "glColor4i %d %d %d %d\n", red, green, blue, alpha );
	dllColor4i(red, green, blue, alpha);
}

static void APIENTRY logColor4iv(const GLint *v) {
// unknown type: "const GLint *" name: "v"
	fprintf( tr.logFile, "glColor4iv 'const GLint * v'\n" );
	dllColor4iv(v);
}

static void APIENTRY logColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha) {
	fprintf( tr.logFile, "glColor4s %d %d %d %d\n", red, green, blue, alpha );
	dllColor4s(red, green, blue, alpha);
}

static void APIENTRY logColor4sv(const GLshort *v) {
// unknown type: "const GLshort *" name: "v"
	fprintf( tr.logFile, "glColor4sv 'const GLshort * v'\n" );
	dllColor4sv(v);
}

static void APIENTRY logColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha) {
	fprintf( tr.logFile, "glColor4ub %d %d %d %d\n", red, green, blue, alpha );
	dllColor4ub(red, green, blue, alpha);
}

static void APIENTRY logColor4ubv(const GLubyte *v) {
// unknown type: "const GLubyte *" name: "v"
	fprintf( tr.logFile, "glColor4ubv 'const GLubyte * v'\n" );
	dllColor4ubv(v);
}

static void APIENTRY logColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha) {
	fprintf( tr.logFile, "glColor4ui %d %d %d %d\n", red, green, blue, alpha );
	dllColor4ui(red, green, blue, alpha);
}

static void APIENTRY logColor4uiv(const GLuint *v) {
// unknown type: "const GLuint *" name: "v"
	fprintf( tr.logFile, "glColor4uiv 'const GLuint * v'\n" );
	dllColor4uiv(v);
}

static void APIENTRY logColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha) {
	fprintf( tr.logFile, "glColor4us %d %d %d %d\n", red, green, blue, alpha );
	dllColor4us(red, green, blue, alpha);
}

static void APIENTRY logColor4usv(const GLushort *v) {
// unknown type: "const GLushort *" name: "v"
	fprintf( tr.logFile, "glColor4usv 'const GLushort * v'\n" );
	dllColor4usv(v);
}

static void APIENTRY logColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
	fprintf( tr.logFile, "glColorMask %s %s %s %s\n", red ? "Y" : "N", green ? "Y" : "N", blue ? "Y" : "N", alpha ? "Y" : "N" );
	dllColorMask(red, green, blue, alpha);
}

static void APIENTRY logColorMaterial(GLenum face, GLenum mode) {
	fprintf( tr.logFile, "glColorMaterial %s %s\n", EnumString(face), EnumString(mode) );
	dllColorMaterial(face, mode);
}

static void APIENTRY logColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
// unknown type: "const GLvoid *" name: "pointer"
	fprintf( tr.logFile, "glColorPointer %d %s %d 'const GLvoid * pointer'\n", size, EnumString(type), stride );
	dllColorPointer(size, type, stride, pointer);
}

static void APIENTRY logCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type) {
	fprintf( tr.logFile, "glCopyPixels %d %d %d %d %s\n", x, y, width, height, EnumString(type) );
	dllCopyPixels(x, y, width, height, type);
}

static void APIENTRY logCopyTexImage1D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border) {
	fprintf( tr.logFile, "glCopyTexImage1D %s %d %s %d %d %d %d\n", EnumString(target), level, EnumString(internalFormat), x, y, width, border );
	dllCopyTexImage1D(target, level, internalFormat, x, y, width, border);
}

static void APIENTRY logCopyTexImage2D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
	fprintf( tr.logFile, "glCopyTexImage2D %s %d %s %d %d %d %d %d\n", EnumString(target), level, EnumString(internalFormat), x, y, width, height, border );
	dllCopyTexImage2D(target, level, internalFormat, x, y, width, height, border);
}

static void APIENTRY logCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) {
	fprintf( tr.logFile, "glCopyTexSubImage1D %s %d %d %d %d %d\n", EnumString(target), level, xoffset, x, y, width );
	dllCopyTexSubImage1D(target, level, xoffset, x, y, width);
}

static void APIENTRY logCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
	fprintf( tr.logFile, "glCopyTexSubImage2D %s %d %d %d %d %d %d %d\n", EnumString(target), level, xoffset, yoffset, x, y, width, height );
	dllCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

static void APIENTRY logCullFace(GLenum mode) {
	fprintf( tr.logFile, "glCullFace %s\n", EnumString(mode) );
	dllCullFace(mode);
}

static void APIENTRY logDeleteLists(GLuint list, GLsizei range) {
	fprintf( tr.logFile, "glDeleteLists %d %d\n", list, range );
	dllDeleteLists(list, range);
}

static void APIENTRY logDeleteTextures(GLsizei n, const GLuint *textures) {
// unknown type: "const GLuint *" name: "textures"
	fprintf( tr.logFile, "glDeleteTextures %d 'const GLuint * textures'\n", n );
	dllDeleteTextures(n, textures);
}

static void APIENTRY logDepthFunc(GLenum func) {
	fprintf( tr.logFile, "glDepthFunc %s\n", EnumString(func) );
	dllDepthFunc(func);
}

static void APIENTRY logDepthMask(GLboolean flag) {
	fprintf( tr.logFile, "glDepthMask %s\n", flag ? "Y" : "N" );
	dllDepthMask(flag);
}

static void APIENTRY logDepthRange(GLclampd zNear, GLclampd zFar) {
// unknown type: "GLclampd" name: "zNear"
// unknown type: "GLclampd" name: "zFar"
	fprintf( tr.logFile, "glDepthRange 'GLclampd zNear' 'GLclampd zFar'\n" );
	dllDepthRange(zNear, zFar);
}

static void APIENTRY logDisable(GLenum cap) {
	fprintf( tr.logFile, "glDisable %s\n", EnumString(cap) );
	dllDisable(cap);
}

static void APIENTRY logDisableClientState(GLenum array) {
	fprintf( tr.logFile, "glDisableClientState %s\n", EnumString(array) );
	dllDisableClientState(array);
}

static void APIENTRY logDrawArrays(GLenum mode, GLint first, GLsizei count) {
	fprintf( tr.logFile, "glDrawArrays %s %d %d\n", EnumString(mode), first, count );
	dllDrawArrays(mode, first, count);
}

static void APIENTRY logDrawBuffer(GLenum mode) {
	fprintf( tr.logFile, "glDrawBuffer %s\n", EnumString(mode) );
	dllDrawBuffer(mode);
}

static void APIENTRY logDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) {
// unknown type: "const GLvoid *" name: "indices"
	fprintf( tr.logFile, "glDrawElements %s %d %s 'const GLvoid * indices'\n", EnumString(mode), count, EnumString(type) );
	dllDrawElements(mode, count, type, indices);
}

static void APIENTRY logDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels) {
// unknown type: "const GLvoid *" name: "pixels"
	fprintf( tr.logFile, "glDrawPixels %d %d %s %s 'const GLvoid * pixels'\n", width, height, EnumString(format), EnumString(type) );
	dllDrawPixels(width, height, format, type, pixels);
}

static void APIENTRY logEdgeFlag(GLboolean flag) {
	fprintf( tr.logFile, "glEdgeFlag %s\n", flag ? "Y" : "N" );
	dllEdgeFlag(flag);
}

static void APIENTRY logEdgeFlagPointer(GLsizei stride, const GLvoid *pointer) {
// unknown type: "const GLvoid *" name: "pointer"
	fprintf( tr.logFile, "glEdgeFlagPointer %d 'const GLvoid * pointer'\n", stride );
	dllEdgeFlagPointer(stride, pointer);
}

static void APIENTRY logEdgeFlagv(const GLboolean *flag) {
// unknown type: "const GLboolean *" name: "flag"
	fprintf( tr.logFile, "glEdgeFlagv 'const GLboolean * flag'\n" );
	dllEdgeFlagv(flag);
}

static void APIENTRY logEnable(GLenum cap) {
	fprintf( tr.logFile, "glEnable %s\n", EnumString(cap) );
	dllEnable(cap);
}

static void APIENTRY logEnableClientState(GLenum array) {
	fprintf( tr.logFile, "glEnableClientState %s\n", EnumString(array) );
	dllEnableClientState(array);
}

static void APIENTRY logEnd(void) {
	fprintf( tr.logFile, "glEnd\n" );
	dllEnd();
}

static void APIENTRY logEndList(void) {
	fprintf( tr.logFile, "glEndList\n" );
	dllEndList();
}

static void APIENTRY logEvalCoord1d(GLdouble u) {
	fprintf( tr.logFile, "glEvalCoord1d %g\n", u );
	dllEvalCoord1d(u);
}

static void APIENTRY logEvalCoord1dv(const GLdouble *u) {
// unknown type: "const GLdouble *" name: "u"
	fprintf( tr.logFile, "glEvalCoord1dv 'const GLdouble * u'\n" );
	dllEvalCoord1dv(u);
}

static void APIENTRY logEvalCoord1f(GLfloat u) {
	fprintf( tr.logFile, "glEvalCoord1f %g\n", u );
	dllEvalCoord1f(u);
}

static void APIENTRY logEvalCoord1fv(const GLfloat *u) {
// unknown type: "const GLfloat *" name: "u"
	fprintf( tr.logFile, "glEvalCoord1fv 'const GLfloat * u'\n" );
	dllEvalCoord1fv(u);
}

static void APIENTRY logEvalCoord2d(GLdouble u, GLdouble v) {
	fprintf( tr.logFile, "glEvalCoord2d %g %g\n", u, v );
	dllEvalCoord2d(u, v);
}

static void APIENTRY logEvalCoord2dv(const GLdouble *u) {
// unknown type: "const GLdouble *" name: "u"
	fprintf( tr.logFile, "glEvalCoord2dv 'const GLdouble * u'\n" );
	dllEvalCoord2dv(u);
}

static void APIENTRY logEvalCoord2f(GLfloat u, GLfloat v) {
	fprintf( tr.logFile, "glEvalCoord2f %g %g\n", u, v );
	dllEvalCoord2f(u, v);
}

static void APIENTRY logEvalCoord2fv(const GLfloat *u) {
// unknown type: "const GLfloat *" name: "u"
	fprintf( tr.logFile, "glEvalCoord2fv 'const GLfloat * u'\n" );
	dllEvalCoord2fv(u);
}

static void APIENTRY logEvalMesh1(GLenum mode, GLint i1, GLint i2) {
	fprintf( tr.logFile, "glEvalMesh1 %s %d %d\n", EnumString(mode), i1, i2 );
	dllEvalMesh1(mode, i1, i2);
}

static void APIENTRY logEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2) {
	fprintf( tr.logFile, "glEvalMesh2 %s %d %d %d %d\n", EnumString(mode), i1, i2, j1, j2 );
	dllEvalMesh2(mode, i1, i2, j1, j2);
}

static void APIENTRY logEvalPoint1(GLint i) {
	fprintf( tr.logFile, "glEvalPoint1 %d\n", i );
	dllEvalPoint1(i);
}

static void APIENTRY logEvalPoint2(GLint i, GLint j) {
	fprintf( tr.logFile, "glEvalPoint2 %d %d\n", i, j );
	dllEvalPoint2(i, j);
}

static void APIENTRY logFeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer) {
// unknown type: "GLfloat *" name: "buffer"
	fprintf( tr.logFile, "glFeedbackBuffer %d %s 'GLfloat * buffer'\n", size, EnumString(type) );
	dllFeedbackBuffer(size, type, buffer);
}

static void APIENTRY logFinish(void) {
	fprintf( tr.logFile, "glFinish\n" );
	dllFinish();
}

static void APIENTRY logFlush(void) {
	fprintf( tr.logFile, "glFlush\n" );
	dllFlush();
}

static void APIENTRY logFogf(GLenum pname, GLfloat param) {
	fprintf( tr.logFile, "glFogf %s %g\n", EnumString(pname), param );
	dllFogf(pname, param);
}

static void APIENTRY logFogfv(GLenum pname, const GLfloat *params) {
// unknown type: "const GLfloat *" name: "params"
	fprintf( tr.logFile, "glFogfv %s 'const GLfloat * params'\n", EnumString(pname) );
	dllFogfv(pname, params);
}

static void APIENTRY logFogi(GLenum pname, GLint param) {
	fprintf( tr.logFile, "glFogi %s %d\n", EnumString(pname), param );
	dllFogi(pname, param);
}

static void APIENTRY logFogiv(GLenum pname, const GLint *params) {
// unknown type: "const GLint *" name: "params"
	fprintf( tr.logFile, "glFogiv %s 'const GLint * params'\n", EnumString(pname) );
	dllFogiv(pname, params);
}

static void APIENTRY logFrontFace(GLenum mode) {
	fprintf( tr.logFile, "glFrontFace %s\n", EnumString(mode) );
	dllFrontFace(mode);
}

static void APIENTRY logFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) {
	fprintf( tr.logFile, "glFrustum %g %g %g %g %g %g\n", left, right, bottom, top, zNear, zFar );
	dllFrustum(left, right, bottom, top, zNear, zFar);
}

static GLuint APIENTRY logGenLists(GLsizei range) {
	fprintf( tr.logFile, "glGenLists %d\n", range );
	return dllGenLists(range);
}

static void APIENTRY logGenTextures(GLsizei n, GLuint *textures) {
// unknown type: "GLuint *" name: "textures"
	fprintf( tr.logFile, "glGenTextures %d 'GLuint * textures'\n", n );
	dllGenTextures(n, textures);
}

static void APIENTRY logGetBooleanv(GLenum pname, GLboolean *params) {
// unknown type: "GLboolean *" name: "params"
	fprintf( tr.logFile, "glGetBooleanv %s 'GLboolean * params'\n", EnumString(pname) );
	dllGetBooleanv(pname, params);
}

static void APIENTRY logGetClipPlane(GLenum plane, GLdouble *equation) {
// unknown type: "GLdouble *" name: "equation"
	fprintf( tr.logFile, "glGetClipPlane %s 'GLdouble * equation'\n", EnumString(plane) );
	dllGetClipPlane(plane, equation);
}

static void APIENTRY logGetDoublev(GLenum pname, GLdouble *params) {
// unknown type: "GLdouble *" name: "params"
	fprintf( tr.logFile, "glGetDoublev %s 'GLdouble * params'\n", EnumString(pname) );
	dllGetDoublev(pname, params);
}

static GLenum APIENTRY logGetError(void) {
	fprintf( tr.logFile, "glGetError\n" );
	return dllGetError();
}

static void APIENTRY logGetFloatv(GLenum pname, GLfloat *params) {
// unknown type: "GLfloat *" name: "params"
	fprintf( tr.logFile, "glGetFloatv %s 'GLfloat * params'\n", EnumString(pname) );
	dllGetFloatv(pname, params);
}

static void APIENTRY logGetIntegerv(GLenum pname, GLint *params) {
// unknown type: "GLint *" name: "params"
	fprintf( tr.logFile, "glGetIntegerv %s 'GLint * params'\n", EnumString(pname) );
	dllGetIntegerv(pname, params);
}

static void APIENTRY logGetLightfv(GLenum light, GLenum pname, GLfloat *params) {
// unknown type: "GLfloat *" name: "params"
	fprintf( tr.logFile, "glGetLightfv %s %s 'GLfloat * params'\n", EnumString(light), EnumString(pname) );
	dllGetLightfv(light, pname, params);
}

static void APIENTRY logGetLightiv(GLenum light, GLenum pname, GLint *params) {
// unknown type: "GLint *" name: "params"
	fprintf( tr.logFile, "glGetLightiv %s %s 'GLint * params'\n", EnumString(light), EnumString(pname) );
	dllGetLightiv(light, pname, params);
}

static void APIENTRY logGetMapdv(GLenum target, GLenum query, GLdouble *v) {
// unknown type: "GLdouble *" name: "v"
	fprintf( tr.logFile, "glGetMapdv %s %s 'GLdouble * v'\n", EnumString(target), EnumString(query) );
	dllGetMapdv(target, query, v);
}

static void APIENTRY logGetMapfv(GLenum target, GLenum query, GLfloat *v) {
// unknown type: "GLfloat *" name: "v"
	fprintf( tr.logFile, "glGetMapfv %s %s 'GLfloat * v'\n", EnumString(target), EnumString(query) );
	dllGetMapfv(target, query, v);
}

static void APIENTRY logGetMapiv(GLenum target, GLenum query, GLint *v) {
// unknown type: "GLint *" name: "v"
	fprintf( tr.logFile, "glGetMapiv %s %s 'GLint * v'\n", EnumString(target), EnumString(query) );
	dllGetMapiv(target, query, v);
}

static void APIENTRY logGetMaterialfv(GLenum face, GLenum pname, GLfloat *params) {
// unknown type: "GLfloat *" name: "params"
	fprintf( tr.logFile, "glGetMaterialfv %s %s 'GLfloat * params'\n", EnumString(face), EnumString(pname) );
	dllGetMaterialfv(face, pname, params);
}

static void APIENTRY logGetMaterialiv(GLenum face, GLenum pname, GLint *params) {
// unknown type: "GLint *" name: "params"
	fprintf( tr.logFile, "glGetMaterialiv %s %s 'GLint * params'\n", EnumString(face), EnumString(pname) );
	dllGetMaterialiv(face, pname, params);
}

static void APIENTRY logGetPixelMapfv(GLenum map, GLfloat *values) {
// unknown type: "GLfloat *" name: "values"
	fprintf( tr.logFile, "glGetPixelMapfv %s 'GLfloat * values'\n", EnumString(map) );
	dllGetPixelMapfv(map, values);
}

static void APIENTRY logGetPixelMapuiv(GLenum map, GLuint *values) {
// unknown type: "GLuint *" name: "values"
	fprintf( tr.logFile, "glGetPixelMapuiv %s 'GLuint * values'\n", EnumString(map) );
	dllGetPixelMapuiv(map, values);
}

static void APIENTRY logGetPixelMapusv(GLenum map, GLushort *values) {
// unknown type: "GLushort *" name: "values"
	fprintf( tr.logFile, "glGetPixelMapusv %s 'GLushort * values'\n", EnumString(map) );
	dllGetPixelMapusv(map, values);
}

static void APIENTRY logGetPointerv(GLenum pname, GLvoid* *params) {
// unknown type: "GLvoid* *" name: "params"
	fprintf( tr.logFile, "glGetPointerv %s 'GLvoid* * params'\n", EnumString(pname) );
	dllGetPointerv(pname, params);
}

static void APIENTRY logGetPolygonStipple(GLubyte *mask) {
// unknown type: "GLubyte *" name: "mask"
	fprintf( tr.logFile, "glGetPolygonStipple 'GLubyte * mask'\n" );
	dllGetPolygonStipple(mask);
}

static const GLubyte * APIENTRY logGetString(GLenum name) {
	fprintf( tr.logFile, "glGetString %s\n", EnumString(name) );
	return dllGetString(name);
}

static void APIENTRY logGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params) {
// unknown type: "GLfloat *" name: "params"
	fprintf( tr.logFile, "glGetTexEnvfv %s %s 'GLfloat * params'\n", EnumString(target), EnumString(pname) );
	dllGetTexEnvfv(target, pname, params);
}

static void APIENTRY logGetTexEnviv(GLenum target, GLenum pname, GLint *params) {
// unknown type: "GLint *" name: "params"
	fprintf( tr.logFile, "glGetTexEnviv %s %s 'GLint * params'\n", EnumString(target), EnumString(pname) );
	dllGetTexEnviv(target, pname, params);
}

static void APIENTRY logGetTexGendv(GLenum coord, GLenum pname, GLdouble *params) {
// unknown type: "GLdouble *" name: "params"
	fprintf( tr.logFile, "glGetTexGendv %s %s 'GLdouble * params'\n", EnumString(coord), EnumString(pname) );
	dllGetTexGendv(coord, pname, params);
}

static void APIENTRY logGetTexGenfv(GLenum coord, GLenum pname, GLfloat *params) {
// unknown type: "GLfloat *" name: "params"
	fprintf( tr.logFile, "glGetTexGenfv %s %s 'GLfloat * params'\n", EnumString(coord), EnumString(pname) );
	dllGetTexGenfv(coord, pname, params);
}

static void APIENTRY logGetTexGeniv(GLenum coord, GLenum pname, GLint *params) {
// unknown type: "GLint *" name: "params"
	fprintf( tr.logFile, "glGetTexGeniv %s %s 'GLint * params'\n", EnumString(coord), EnumString(pname) );
	dllGetTexGeniv(coord, pname, params);
}

static void APIENTRY logGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels) {
// unknown type: "GLvoid *" name: "pixels"
	fprintf( tr.logFile, "glGetTexImage %s %d %s %s 'GLvoid * pixels'\n", EnumString(target), level, EnumString(format), EnumString(type) );
	dllGetTexImage(target, level, format, type, pixels);
}

static void APIENTRY logGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params) {
// unknown type: "GLfloat *" name: "params"
	fprintf( tr.logFile, "glGetTexLevelParameterfv %s %d %s 'GLfloat * params'\n", EnumString(target), level, EnumString(pname) );
	dllGetTexLevelParameterfv(target, level, pname, params);
}

static void APIENTRY logGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params) {
// unknown type: "GLint *" name: "params"
	fprintf( tr.logFile, "glGetTexLevelParameteriv %s %d %s 'GLint * params'\n", EnumString(target), level, EnumString(pname) );
	dllGetTexLevelParameteriv(target, level, pname, params);
}

static void APIENTRY logGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params) {
// unknown type: "GLfloat *" name: "params"
	fprintf( tr.logFile, "glGetTexParameterfv %s %s 'GLfloat * params'\n", EnumString(target), EnumString(pname) );
	dllGetTexParameterfv(target, pname, params);
}

static void APIENTRY logGetTexParameteriv(GLenum target, GLenum pname, GLint *params) {
// unknown type: "GLint *" name: "params"
	fprintf( tr.logFile, "glGetTexParameteriv %s %s 'GLint * params'\n", EnumString(target), EnumString(pname) );
	dllGetTexParameteriv(target, pname, params);
}

static void APIENTRY logHint(GLenum target, GLenum mode) {
	fprintf( tr.logFile, "glHint %s %s\n", EnumString(target), EnumString(mode) );
	dllHint(target, mode);
}

static void APIENTRY logIndexMask(GLuint mask) {
	fprintf( tr.logFile, "glIndexMask %d\n", mask );
	dllIndexMask(mask);
}

static void APIENTRY logIndexPointer(GLenum type, GLsizei stride, const GLvoid *pointer) {
// unknown type: "const GLvoid *" name: "pointer"
	fprintf( tr.logFile, "glIndexPointer %s %d 'const GLvoid * pointer'\n", EnumString(type), stride );
	dllIndexPointer(type, stride, pointer);
}

static void APIENTRY logIndexd(GLdouble c) {
	fprintf( tr.logFile, "glIndexd %g\n", c );
	dllIndexd(c);
}

static void APIENTRY logIndexdv(const GLdouble *c) {
// unknown type: "const GLdouble *" name: "c"
	fprintf( tr.logFile, "glIndexdv 'const GLdouble * c'\n" );
	dllIndexdv(c);
}

static void APIENTRY logIndexf(GLfloat c) {
	fprintf( tr.logFile, "glIndexf %g\n", c );
	dllIndexf(c);
}

static void APIENTRY logIndexfv(const GLfloat *c) {
// unknown type: "const GLfloat *" name: "c"
	fprintf( tr.logFile, "glIndexfv 'const GLfloat * c'\n" );
	dllIndexfv(c);
}

static void APIENTRY logIndexi(GLint c) {
	fprintf( tr.logFile, "glIndexi %d\n", c );
	dllIndexi(c);
}

static void APIENTRY logIndexiv(const GLint *c) {
// unknown type: "const GLint *" name: "c"
	fprintf( tr.logFile, "glIndexiv 'const GLint * c'\n" );
	dllIndexiv(c);
}

static void APIENTRY logIndexs(GLshort c) {
	fprintf( tr.logFile, "glIndexs %d\n", c );
	dllIndexs(c);
}

static void APIENTRY logIndexsv(const GLshort *c) {
// unknown type: "const GLshort *" name: "c"
	fprintf( tr.logFile, "glIndexsv 'const GLshort * c'\n" );
	dllIndexsv(c);
}

static void APIENTRY logIndexub(GLubyte c) {
	fprintf( tr.logFile, "glIndexub %d\n", c );
	dllIndexub(c);
}

static void APIENTRY logIndexubv(const GLubyte *c) {
// unknown type: "const GLubyte *" name: "c"
	fprintf( tr.logFile, "glIndexubv 'const GLubyte * c'\n" );
	dllIndexubv(c);
}

static void APIENTRY logInitNames(void) {
	fprintf( tr.logFile, "glInitNames\n" );
	dllInitNames();
}

static void APIENTRY logInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer) {
// unknown type: "const GLvoid *" name: "pointer"
	fprintf( tr.logFile, "glInterleavedArrays %s %d 'const GLvoid * pointer'\n", EnumString(format), stride );
	dllInterleavedArrays(format, stride, pointer);
}

static GLboolean APIENTRY logIsEnabled(GLenum cap) {
	fprintf( tr.logFile, "glIsEnabled %s\n", EnumString(cap) );
	return dllIsEnabled(cap);
}

static GLboolean APIENTRY logIsList(GLuint list) {
	fprintf( tr.logFile, "glIsList %d\n", list );
	return dllIsList(list);
}

static GLboolean APIENTRY logIsTexture(GLuint texture) {
	fprintf( tr.logFile, "glIsTexture %d\n", texture );
	return dllIsTexture(texture);
}

static void APIENTRY logLightModelf(GLenum pname, GLfloat param) {
	fprintf( tr.logFile, "glLightModelf %s %g\n", EnumString(pname), param );
	dllLightModelf(pname, param);
}

static void APIENTRY logLightModelfv(GLenum pname, const GLfloat *params) {
// unknown type: "const GLfloat *" name: "params"
	fprintf( tr.logFile, "glLightModelfv %s 'const GLfloat * params'\n", EnumString(pname) );
	dllLightModelfv(pname, params);
}

static void APIENTRY logLightModeli(GLenum pname, GLint param) {
	fprintf( tr.logFile, "glLightModeli %s %d\n", EnumString(pname), param );
	dllLightModeli(pname, param);
}

static void APIENTRY logLightModeliv(GLenum pname, const GLint *params) {
// unknown type: "const GLint *" name: "params"
	fprintf( tr.logFile, "glLightModeliv %s 'const GLint * params'\n", EnumString(pname) );
	dllLightModeliv(pname, params);
}

static void APIENTRY logLightf(GLenum light, GLenum pname, GLfloat param) {
	fprintf( tr.logFile, "glLightf %s %s %g\n", EnumString(light), EnumString(pname), param );
	dllLightf(light, pname, param);
}

static void APIENTRY logLightfv(GLenum light, GLenum pname, const GLfloat *params) {
// unknown type: "const GLfloat *" name: "params"
	fprintf( tr.logFile, "glLightfv %s %s 'const GLfloat * params'\n", EnumString(light), EnumString(pname) );
	dllLightfv(light, pname, params);
}

static void APIENTRY logLighti(GLenum light, GLenum pname, GLint param) {
	fprintf( tr.logFile, "glLighti %s %s %d\n", EnumString(light), EnumString(pname), param );
	dllLighti(light, pname, param);
}

static void APIENTRY logLightiv(GLenum light, GLenum pname, const GLint *params) {
// unknown type: "const GLint *" name: "params"
	fprintf( tr.logFile, "glLightiv %s %s 'const GLint * params'\n", EnumString(light), EnumString(pname) );
	dllLightiv(light, pname, params);
}

static void APIENTRY logLineStipple(GLint factor, GLushort pattern) {
	fprintf( tr.logFile, "glLineStipple %d %d\n", factor, pattern );
	dllLineStipple(factor, pattern);
}

static void APIENTRY logLineWidth(GLfloat width) {
	fprintf( tr.logFile, "glLineWidth %g\n", width );
	dllLineWidth(width);
}

static void APIENTRY logListBase(GLuint base) {
	fprintf( tr.logFile, "glListBase %d\n", base );
	dllListBase(base);
}

static void APIENTRY logLoadIdentity(void) {
	fprintf( tr.logFile, "glLoadIdentity\n" );
	dllLoadIdentity();
}

static void APIENTRY logLoadMatrixd(const GLdouble *m) {
// unknown type: "const GLdouble *" name: "m"
	fprintf( tr.logFile, "glLoadMatrixd 'const GLdouble * m'\n" );
	dllLoadMatrixd(m);
}

static void APIENTRY logLoadMatrixf(const GLfloat *m) {
// unknown type: "const GLfloat *" name: "m"
	fprintf( tr.logFile, "glLoadMatrixf 'const GLfloat * m'\n" );
	dllLoadMatrixf(m);
}

static void APIENTRY logLoadName(GLuint name) {
	fprintf( tr.logFile, "glLoadName %d\n", name );
	dllLoadName(name);
}

static void APIENTRY logLogicOp(GLenum opcode) {
	fprintf( tr.logFile, "glLogicOp %s\n", EnumString(opcode) );
	dllLogicOp(opcode);
}

static void APIENTRY logMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points) {
// unknown type: "const GLdouble *" name: "points"
	fprintf( tr.logFile, "glMap1d %s %g %g %d %d 'const GLdouble * points'\n", EnumString(target), u1, u2, stride, order );
	dllMap1d(target, u1, u2, stride, order, points);
}

static void APIENTRY logMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points) {
// unknown type: "const GLfloat *" name: "points"
	fprintf( tr.logFile, "glMap1f %s %g %g %d %d 'const GLfloat * points'\n", EnumString(target), u1, u2, stride, order );
	dllMap1f(target, u1, u2, stride, order, points);
}

static void APIENTRY logMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points) {
// unknown type: "const GLdouble *" name: "points"
	fprintf( tr.logFile, "glMap2d %s %g %g %d %d %g %g %d %d 'const GLdouble * points'\n", EnumString(target), u1, u2, ustride, uorder, v1, v2, vstride, vorder );
	dllMap2d(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}

static void APIENTRY logMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points) {
// unknown type: "const GLfloat *" name: "points"
	fprintf( tr.logFile, "glMap2f %s %g %g %d %d %g %g %d %d 'const GLfloat * points'\n", EnumString(target), u1, u2, ustride, uorder, v1, v2, vstride, vorder );
	dllMap2f(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}

static void APIENTRY logMapGrid1d(GLint un, GLdouble u1, GLdouble u2) {
	fprintf( tr.logFile, "glMapGrid1d %d %g %g\n", un, u1, u2 );
	dllMapGrid1d(un, u1, u2);
}

static void APIENTRY logMapGrid1f(GLint un, GLfloat u1, GLfloat u2) {
	fprintf( tr.logFile, "glMapGrid1f %d %g %g\n", un, u1, u2 );
	dllMapGrid1f(un, u1, u2);
}

static void APIENTRY logMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2) {
	fprintf( tr.logFile, "glMapGrid2d %d %g %g %d %g %g\n", un, u1, u2, vn, v1, v2 );
	dllMapGrid2d(un, u1, u2, vn, v1, v2);
}

static void APIENTRY logMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2) {
	fprintf( tr.logFile, "glMapGrid2f %d %g %g %d %g %g\n", un, u1, u2, vn, v1, v2 );
	dllMapGrid2f(un, u1, u2, vn, v1, v2);
}

static void APIENTRY logMaterialf(GLenum face, GLenum pname, GLfloat param) {
	fprintf( tr.logFile, "glMaterialf %s %s %g\n", EnumString(face), EnumString(pname), param );
	dllMaterialf(face, pname, param);
}

static void APIENTRY logMaterialfv(GLenum face, GLenum pname, const GLfloat *params) {
// unknown type: "const GLfloat *" name: "params"
	fprintf( tr.logFile, "glMaterialfv %s %s 'const GLfloat * params'\n", EnumString(face), EnumString(pname) );
	dllMaterialfv(face, pname, params);
}

static void APIENTRY logMateriali(GLenum face, GLenum pname, GLint param) {
	fprintf( tr.logFile, "glMateriali %s %s %d\n", EnumString(face), EnumString(pname), param );
	dllMateriali(face, pname, param);
}

static void APIENTRY logMaterialiv(GLenum face, GLenum pname, const GLint *params) {
// unknown type: "const GLint *" name: "params"
	fprintf( tr.logFile, "glMaterialiv %s %s 'const GLint * params'\n", EnumString(face), EnumString(pname) );
	dllMaterialiv(face, pname, params);
}

static void APIENTRY logMatrixMode(GLenum mode) {
	fprintf( tr.logFile, "glMatrixMode %s\n", EnumString(mode) );
	dllMatrixMode(mode);
}

static void APIENTRY logMultMatrixd(const GLdouble *m) {
// unknown type: "const GLdouble *" name: "m"
	fprintf( tr.logFile, "glMultMatrixd 'const GLdouble * m'\n" );
	dllMultMatrixd(m);
}

static void APIENTRY logMultMatrixf(const GLfloat *m) {
// unknown type: "const GLfloat *" name: "m"
	fprintf( tr.logFile, "glMultMatrixf 'const GLfloat * m'\n" );
	dllMultMatrixf(m);
}

static void APIENTRY logNewList(GLuint list, GLenum mode) {
	fprintf( tr.logFile, "glNewList %d %s\n", list, EnumString(mode) );
	dllNewList(list, mode);
}

static void APIENTRY logNormal3b(GLbyte nx, GLbyte ny, GLbyte nz) {
	fprintf( tr.logFile, "glNormal3b %d %d %d\n", nx, ny, nz );
	dllNormal3b(nx, ny, nz);
}

static void APIENTRY logNormal3bv(const GLbyte *v) {
// unknown type: "const GLbyte *" name: "v"
	fprintf( tr.logFile, "glNormal3bv 'const GLbyte * v'\n" );
	dllNormal3bv(v);
}

static void APIENTRY logNormal3d(GLdouble nx, GLdouble ny, GLdouble nz) {
	fprintf( tr.logFile, "glNormal3d %g %g %g\n", nx, ny, nz );
	dllNormal3d(nx, ny, nz);
}

static void APIENTRY logNormal3dv(const GLdouble *v) {
// unknown type: "const GLdouble *" name: "v"
	fprintf( tr.logFile, "glNormal3dv 'const GLdouble * v'\n" );
	dllNormal3dv(v);
}

static void APIENTRY logNormal3f(GLfloat nx, GLfloat ny, GLfloat nz) {
	fprintf( tr.logFile, "glNormal3f %g %g %g\n", nx, ny, nz );
	dllNormal3f(nx, ny, nz);
}

static void APIENTRY logNormal3fv(const GLfloat *v) {
// unknown type: "const GLfloat *" name: "v"
	fprintf( tr.logFile, "glNormal3fv 'const GLfloat * v'\n" );
	dllNormal3fv(v);
}

static void APIENTRY logNormal3i(GLint nx, GLint ny, GLint nz) {
	fprintf( tr.logFile, "glNormal3i %d %d %d\n", nx, ny, nz );
	dllNormal3i(nx, ny, nz);
}

static void APIENTRY logNormal3iv(const GLint *v) {
// unknown type: "const GLint *" name: "v"
	fprintf( tr.logFile, "glNormal3iv 'const GLint * v'\n" );
	dllNormal3iv(v);
}

static void APIENTRY logNormal3s(GLshort nx, GLshort ny, GLshort nz) {
	fprintf( tr.logFile, "glNormal3s %d %d %d\n", nx, ny, nz );
	dllNormal3s(nx, ny, nz);
}

static void APIENTRY logNormal3sv(const GLshort *v) {
// unknown type: "const GLshort *" name: "v"
	fprintf( tr.logFile, "glNormal3sv 'const GLshort * v'\n" );
	dllNormal3sv(v);
}

static void APIENTRY logNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer) {
// unknown type: "const GLvoid *" name: "pointer"
	fprintf( tr.logFile, "glNormalPointer %s %d 'const GLvoid * pointer'\n", EnumString(type), stride );
	dllNormalPointer(type, stride, pointer);
}

static void APIENTRY logOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar) {
	fprintf( tr.logFile, "glOrtho %g %g %g %g %g %g\n", left, right, bottom, top, zNear, zFar );
	dllOrtho(left, right, bottom, top, zNear, zFar);
}

static void APIENTRY logPassThrough(GLfloat token) {
	fprintf( tr.logFile, "glPassThrough %g\n", token );
	dllPassThrough(token);
}

static void APIENTRY logPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat *values) {
// unknown type: "const GLfloat *" name: "values"
	fprintf( tr.logFile, "glPixelMapfv %s %d 'const GLfloat * values'\n", EnumString(map), mapsize );
	dllPixelMapfv(map, mapsize, values);
}

static void APIENTRY logPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint *values) {
// unknown type: "const GLuint *" name: "values"
	fprintf( tr.logFile, "glPixelMapuiv %s %d 'const GLuint * values'\n", EnumString(map), mapsize );
	dllPixelMapuiv(map, mapsize, values);
}

static void APIENTRY logPixelMapusv(GLenum map, GLsizei mapsize, const GLushort *values) {
// unknown type: "const GLushort *" name: "values"
	fprintf( tr.logFile, "glPixelMapusv %s %d 'const GLushort * values'\n", EnumString(map), mapsize );
	dllPixelMapusv(map, mapsize, values);
}

static void APIENTRY logPixelStoref(GLenum pname, GLfloat param) {
	fprintf( tr.logFile, "glPixelStoref %s %g\n", EnumString(pname), param );
	dllPixelStoref(pname, param);
}

static void APIENTRY logPixelStorei(GLenum pname, GLint param) {
	fprintf( tr.logFile, "glPixelStorei %s %d\n", EnumString(pname), param );
	dllPixelStorei(pname, param);
}

static void APIENTRY logPixelTransferf(GLenum pname, GLfloat param) {
	fprintf( tr.logFile, "glPixelTransferf %s %g\n", EnumString(pname), param );
	dllPixelTransferf(pname, param);
}

static void APIENTRY logPixelTransferi(GLenum pname, GLint param) {
	fprintf( tr.logFile, "glPixelTransferi %s %d\n", EnumString(pname), param );
	dllPixelTransferi(pname, param);
}

static void APIENTRY logPixelZoom(GLfloat xfactor, GLfloat yfactor) {
	fprintf( tr.logFile, "glPixelZoom %g %g\n", xfactor, yfactor );
	dllPixelZoom(xfactor, yfactor);
}

static void APIENTRY logPointSize(GLfloat size) {
	fprintf( tr.logFile, "glPointSize %g\n", size );
	dllPointSize(size);
}

static void APIENTRY logPolygonMode(GLenum face, GLenum mode) {
	fprintf( tr.logFile, "glPolygonMode %s %s\n", EnumString(face), EnumString(mode) );
	dllPolygonMode(face, mode);
}

static void APIENTRY logPolygonOffset(GLfloat factor, GLfloat units) {
	fprintf( tr.logFile, "glPolygonOffset %g %g\n", factor, units );
	dllPolygonOffset(factor, units);
}

static void APIENTRY logPolygonStipple(const GLubyte *mask) {
// unknown type: "const GLubyte *" name: "mask"
	fprintf( tr.logFile, "glPolygonStipple 'const GLubyte * mask'\n" );
	dllPolygonStipple(mask);
}

static void APIENTRY logPopAttrib(void) {
	fprintf( tr.logFile, "glPopAttrib\n" );
	dllPopAttrib();
}

static void APIENTRY logPopClientAttrib(void) {
	fprintf( tr.logFile, "glPopClientAttrib\n" );
	dllPopClientAttrib();
}

static void APIENTRY logPopMatrix(void) {
	fprintf( tr.logFile, "glPopMatrix\n" );
	dllPopMatrix();
}

static void APIENTRY logPopName(void) {
	fprintf( tr.logFile, "glPopName\n" );
	dllPopName();
}

static void APIENTRY logPrioritizeTextures(GLsizei n, const GLuint *textures, const GLclampf *priorities) {
// unknown type: "const GLuint *" name: "textures"
// unknown type: "const GLclampf *" name: "priorities"
	fprintf( tr.logFile, "glPrioritizeTextures %d 'const GLuint * textures' 'const GLclampf * priorities'\n", n );
	dllPrioritizeTextures(n, textures, priorities);
}

static void APIENTRY logPushAttrib(GLbitfield mask) {
// unknown type: "GLbitfield" name: "mask"
	fprintf( tr.logFile, "glPushAttrib 'GLbitfield mask'\n" );
	dllPushAttrib(mask);
}

static void APIENTRY logPushClientAttrib(GLbitfield mask) {
// unknown type: "GLbitfield" name: "mask"
	fprintf( tr.logFile, "glPushClientAttrib 'GLbitfield mask'\n" );
	dllPushClientAttrib(mask);
}

static void APIENTRY logPushMatrix(void) {
	fprintf( tr.logFile, "glPushMatrix\n" );
	dllPushMatrix();
}

static void APIENTRY logPushName(GLuint name) {
	fprintf( tr.logFile, "glPushName %d\n", name );
	dllPushName(name);
}

static void APIENTRY logRasterPos2d(GLdouble x, GLdouble y) {
	fprintf( tr.logFile, "glRasterPos2d %g %g\n", x, y );
	dllRasterPos2d(x, y);
}

static void APIENTRY logRasterPos2dv(const GLdouble *v) {
// unknown type: "const GLdouble *" name: "v"
	fprintf( tr.logFile, "glRasterPos2dv 'const GLdouble * v'\n" );
	dllRasterPos2dv(v);
}

static void APIENTRY logRasterPos2f(GLfloat x, GLfloat y) {
	fprintf( tr.logFile, "glRasterPos2f %g %g\n", x, y );
	dllRasterPos2f(x, y);
}

static void APIENTRY logRasterPos2fv(const GLfloat *v) {
// unknown type: "const GLfloat *" name: "v"
	fprintf( tr.logFile, "glRasterPos2fv 'const GLfloat * v'\n" );
	dllRasterPos2fv(v);
}

static void APIENTRY logRasterPos2i(GLint x, GLint y) {
	fprintf( tr.logFile, "glRasterPos2i %d %d\n", x, y );
	dllRasterPos2i(x, y);
}

static void APIENTRY logRasterPos2iv(const GLint *v) {
// unknown type: "const GLint *" name: "v"
	fprintf( tr.logFile, "glRasterPos2iv 'const GLint * v'\n" );
	dllRasterPos2iv(v);
}

static void APIENTRY logRasterPos2s(GLshort x, GLshort y) {
	fprintf( tr.logFile, "glRasterPos2s %d %d\n", x, y );
	dllRasterPos2s(x, y);
}

static void APIENTRY logRasterPos2sv(const GLshort *v) {
// unknown type: "const GLshort *" name: "v"
	fprintf( tr.logFile, "glRasterPos2sv 'const GLshort * v'\n" );
	dllRasterPos2sv(v);
}

static void APIENTRY logRasterPos3d(GLdouble x, GLdouble y, GLdouble z) {
	fprintf( tr.logFile, "glRasterPos3d %g %g %g\n", x, y, z );
	dllRasterPos3d(x, y, z);
}

static void APIENTRY logRasterPos3dv(const GLdouble *v) {
// unknown type: "const GLdouble *" name: "v"
	fprintf( tr.logFile, "glRasterPos3dv 'const GLdouble * v'\n" );
	dllRasterPos3dv(v);
}

static void APIENTRY logRasterPos3f(GLfloat x, GLfloat y, GLfloat z) {
	fprintf( tr.logFile, "glRasterPos3f %g %g %g\n", x, y, z );
	dllRasterPos3f(x, y, z);
}

static void APIENTRY logRasterPos3fv(const GLfloat *v) {
// unknown type: "const GLfloat *" name: "v"
	fprintf( tr.logFile, "glRasterPos3fv 'const GLfloat * v'\n" );
	dllRasterPos3fv(v);
}

static void APIENTRY logRasterPos3i(GLint x, GLint y, GLint z) {
	fprintf( tr.logFile, "glRasterPos3i %d %d %d\n", x, y, z );
	dllRasterPos3i(x, y, z);
}

static void APIENTRY logRasterPos3iv(const GLint *v) {
// unknown type: "const GLint *" name: "v"
	fprintf( tr.logFile, "glRasterPos3iv 'const GLint * v'\n" );
	dllRasterPos3iv(v);
}

static void APIENTRY logRasterPos3s(GLshort x, GLshort y, GLshort z) {
	fprintf( tr.logFile, "glRasterPos3s %d %d %d\n", x, y, z );
	dllRasterPos3s(x, y, z);
}

static void APIENTRY logRasterPos3sv(const GLshort *v) {
// unknown type: "const GLshort *" name: "v"
	fprintf( tr.logFile, "glRasterPos3sv 'const GLshort * v'\n" );
	dllRasterPos3sv(v);
}

static void APIENTRY logRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
	fprintf( tr.logFile, "glRasterPos4d %g %g %g %g\n", x, y, z, w );
	dllRasterPos4d(x, y, z, w);
}

static void APIENTRY logRasterPos4dv(const GLdouble *v) {
// unknown type: "const GLdouble *" name: "v"
	fprintf( tr.logFile, "glRasterPos4dv 'const GLdouble * v'\n" );
	dllRasterPos4dv(v);
}

static void APIENTRY logRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	fprintf( tr.logFile, "glRasterPos4f %g %g %g %g\n", x, y, z, w );
	dllRasterPos4f(x, y, z, w);
}

static void APIENTRY logRasterPos4fv(const GLfloat *v) {
// unknown type: "const GLfloat *" name: "v"
	fprintf( tr.logFile, "glRasterPos4fv 'const GLfloat * v'\n" );
	dllRasterPos4fv(v);
}

static void APIENTRY logRasterPos4i(GLint x, GLint y, GLint z, GLint w) {
	fprintf( tr.logFile, "glRasterPos4i %d %d %d %d\n", x, y, z, w );
	dllRasterPos4i(x, y, z, w);
}

static void APIENTRY logRasterPos4iv(const GLint *v) {
// unknown type: "const GLint *" name: "v"
	fprintf( tr.logFile, "glRasterPos4iv 'const GLint * v'\n" );
	dllRasterPos4iv(v);
}

static void APIENTRY logRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w) {
	fprintf( tr.logFile, "glRasterPos4s %d %d %d %d\n", x, y, z, w );
	dllRasterPos4s(x, y, z, w);
}

static void APIENTRY logRasterPos4sv(const GLshort *v) {
// unknown type: "const GLshort *" name: "v"
	fprintf( tr.logFile, "glRasterPos4sv 'const GLshort * v'\n" );
	dllRasterPos4sv(v);
}

static void APIENTRY logReadBuffer(GLenum mode) {
	fprintf( tr.logFile, "glReadBuffer %s\n", EnumString(mode) );
	dllReadBuffer(mode);
}

static void APIENTRY logReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels) {
// unknown type: "GLvoid *" name: "pixels"
	fprintf( tr.logFile, "glReadPixels %d %d %d %d %s %s 'GLvoid * pixels'\n", x, y, width, height, EnumString(format), EnumString(type) );
	dllReadPixels(x, y, width, height, format, type, pixels);
}

static void APIENTRY logRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2) {
	fprintf( tr.logFile, "glRectd %g %g %g %g\n", x1, y1, x2, y2 );
	dllRectd(x1, y1, x2, y2);
}

static void APIENTRY logRectdv(const GLdouble *v1, const GLdouble *v2) {
// unknown type: "const GLdouble *" name: "v1"
// unknown type: "const GLdouble *" name: "v2"
	fprintf( tr.logFile, "glRectdv 'const GLdouble * v1' 'const GLdouble * v2'\n" );
	dllRectdv(v1, v2);
}

static void APIENTRY logRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
	fprintf( tr.logFile, "glRectf %g %g %g %g\n", x1, y1, x2, y2 );
	dllRectf(x1, y1, x2, y2);
}

static void APIENTRY logRectfv(const GLfloat *v1, const GLfloat *v2) {
// unknown type: "const GLfloat *" name: "v1"
// unknown type: "const GLfloat *" name: "v2"
	fprintf( tr.logFile, "glRectfv 'const GLfloat * v1' 'const GLfloat * v2'\n" );
	dllRectfv(v1, v2);
}

static void APIENTRY logRecti(GLint x1, GLint y1, GLint x2, GLint y2) {
	fprintf( tr.logFile, "glRecti %d %d %d %d\n", x1, y1, x2, y2 );
	dllRecti(x1, y1, x2, y2);
}

static void APIENTRY logRectiv(const GLint *v1, const GLint *v2) {
// unknown type: "const GLint *" name: "v1"
// unknown type: "const GLint *" name: "v2"
	fprintf( tr.logFile, "glRectiv 'const GLint * v1' 'const GLint * v2'\n" );
	dllRectiv(v1, v2);
}

static void APIENTRY logRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2) {
	fprintf( tr.logFile, "glRects %d %d %d %d\n", x1, y1, x2, y2 );
	dllRects(x1, y1, x2, y2);
}

static void APIENTRY logRectsv(const GLshort *v1, const GLshort *v2) {
// unknown type: "const GLshort *" name: "v1"
// unknown type: "const GLshort *" name: "v2"
	fprintf( tr.logFile, "glRectsv 'const GLshort * v1' 'const GLshort * v2'\n" );
	dllRectsv(v1, v2);
}

static GLint APIENTRY logRenderMode(GLenum mode) {
	fprintf( tr.logFile, "glRenderMode %s\n", EnumString(mode) );
	return dllRenderMode(mode);
}

static void APIENTRY logRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z) {
	fprintf( tr.logFile, "glRotated %g %g %g %g\n", angle, x, y, z );
	dllRotated(angle, x, y, z);
}

static void APIENTRY logRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
	fprintf( tr.logFile, "glRotatef %g %g %g %g\n", angle, x, y, z );
	dllRotatef(angle, x, y, z);
}

static void APIENTRY logScaled(GLdouble x, GLdouble y, GLdouble z) {
	fprintf( tr.logFile, "glScaled %g %g %g\n", x, y, z );
	dllScaled(x, y, z);
}

static void APIENTRY logScalef(GLfloat x, GLfloat y, GLfloat z) {
	fprintf( tr.logFile, "glScalef %g %g %g\n", x, y, z );
	dllScalef(x, y, z);
}

static void APIENTRY logScissor(GLint x, GLint y, GLsizei width, GLsizei height) {
	fprintf( tr.logFile, "glScissor %d %d %d %d\n", x, y, width, height );
	dllScissor(x, y, width, height);
}

static void APIENTRY logSelectBuffer(GLsizei size, GLuint *buffer) {
// unknown type: "GLuint *" name: "buffer"
	fprintf( tr.logFile, "glSelectBuffer %d 'GLuint * buffer'\n", size );
	dllSelectBuffer(size, buffer);
}

static void APIENTRY logShadeModel(GLenum mode) {
	fprintf( tr.logFile, "glShadeModel %s\n", EnumString(mode) );
	dllShadeModel(mode);
}

static void APIENTRY logStencilFunc(GLenum func, GLint ref, GLuint mask) {
	fprintf( tr.logFile, "glStencilFunc %s %d %d\n", EnumString(func), ref, mask );
	dllStencilFunc(func, ref, mask);
}

static void APIENTRY logStencilMask(GLuint mask) {
	fprintf( tr.logFile, "glStencilMask %d\n", mask );
	dllStencilMask(mask);
}

static void APIENTRY logStencilOp(GLenum fail, GLenum zfail, GLenum zpass) {
	fprintf( tr.logFile, "glStencilOp %s %s %s\n", EnumString(fail), EnumString(zfail), EnumString(zpass) );
	dllStencilOp(fail, zfail, zpass);
}

static void APIENTRY logTexCoord1d(GLdouble s) {
	fprintf( tr.logFile, "glTexCoord1d %g\n", s );
	dllTexCoord1d(s);
}

static void APIENTRY logTexCoord1dv(const GLdouble *v) {
// unknown type: "const GLdouble *" name: "v"
	fprintf( tr.logFile, "glTexCoord1dv 'const GLdouble * v'\n" );
	dllTexCoord1dv(v);
}

static void APIENTRY logTexCoord1f(GLfloat s) {
	fprintf( tr.logFile, "glTexCoord1f %g\n", s );
	dllTexCoord1f(s);
}

static void APIENTRY logTexCoord1fv(const GLfloat *v) {
// unknown type: "const GLfloat *" name: "v"
	fprintf( tr.logFile, "glTexCoord1fv 'const GLfloat * v'\n" );
	dllTexCoord1fv(v);
}

static void APIENTRY logTexCoord1i(GLint s) {
	fprintf( tr.logFile, "glTexCoord1i %d\n", s );
	dllTexCoord1i(s);
}

static void APIENTRY logTexCoord1iv(const GLint *v) {
// unknown type: "const GLint *" name: "v"
	fprintf( tr.logFile, "glTexCoord1iv 'const GLint * v'\n" );
	dllTexCoord1iv(v);
}

static void APIENTRY logTexCoord1s(GLshort s) {
	fprintf( tr.logFile, "glTexCoord1s %d\n", s );
	dllTexCoord1s(s);
}

static void APIENTRY logTexCoord1sv(const GLshort *v) {
// unknown type: "const GLshort *" name: "v"
	fprintf( tr.logFile, "glTexCoord1sv 'const GLshort * v'\n" );
	dllTexCoord1sv(v);
}

static void APIENTRY logTexCoord2d(GLdouble s, GLdouble t) {
	fprintf( tr.logFile, "glTexCoord2d %g %g\n", s, t );
	dllTexCoord2d(s, t);
}

static void APIENTRY logTexCoord2dv(const GLdouble *v) {
// unknown type: "const GLdouble *" name: "v"
	fprintf( tr.logFile, "glTexCoord2dv 'const GLdouble * v'\n" );
	dllTexCoord2dv(v);
}

static void APIENTRY logTexCoord2f(GLfloat s, GLfloat t) {
	fprintf( tr.logFile, "glTexCoord2f %g %g\n", s, t );
	dllTexCoord2f(s, t);
}

static void APIENTRY logTexCoord2fv(const GLfloat *v) {
// unknown type: "const GLfloat *" name: "v"
	fprintf( tr.logFile, "glTexCoord2fv 'const GLfloat * v'\n" );
	dllTexCoord2fv(v);
}

static void APIENTRY logTexCoord2i(GLint s, GLint t) {
	fprintf( tr.logFile, "glTexCoord2i %d %d\n", s, t );
	dllTexCoord2i(s, t);
}

static void APIENTRY logTexCoord2iv(const GLint *v) {
// unknown type: "const GLint *" name: "v"
	fprintf( tr.logFile, "glTexCoord2iv 'const GLint * v'\n" );
	dllTexCoord2iv(v);
}

static void APIENTRY logTexCoord2s(GLshort s, GLshort t) {
	fprintf( tr.logFile, "glTexCoord2s %d %d\n", s, t );
	dllTexCoord2s(s, t);
}

static void APIENTRY logTexCoord2sv(const GLshort *v) {
// unknown type: "const GLshort *" name: "v"
	fprintf( tr.logFile, "glTexCoord2sv 'const GLshort * v'\n" );
	dllTexCoord2sv(v);
}

static void APIENTRY logTexCoord3d(GLdouble s, GLdouble t, GLdouble r) {
	fprintf( tr.logFile, "glTexCoord3d %g %g %g\n", s, t, r );
	dllTexCoord3d(s, t, r);
}

static void APIENTRY logTexCoord3dv(const GLdouble *v) {
// unknown type: "const GLdouble *" name: "v"
	fprintf( tr.logFile, "glTexCoord3dv 'const GLdouble * v'\n" );
	dllTexCoord3dv(v);
}

static void APIENTRY logTexCoord3f(GLfloat s, GLfloat t, GLfloat r) {
	fprintf( tr.logFile, "glTexCoord3f %g %g %g\n", s, t, r );
	dllTexCoord3f(s, t, r);
}

static void APIENTRY logTexCoord3fv(const GLfloat *v) {
// unknown type: "const GLfloat *" name: "v"
	fprintf( tr.logFile, "glTexCoord3fv 'const GLfloat * v'\n" );
	dllTexCoord3fv(v);
}

static void APIENTRY logTexCoord3i(GLint s, GLint t, GLint r) {
	fprintf( tr.logFile, "glTexCoord3i %d %d %d\n", s, t, r );
	dllTexCoord3i(s, t, r);
}

static void APIENTRY logTexCoord3iv(const GLint *v) {
// unknown type: "const GLint *" name: "v"
	fprintf( tr.logFile, "glTexCoord3iv 'const GLint * v'\n" );
	dllTexCoord3iv(v);
}

static void APIENTRY logTexCoord3s(GLshort s, GLshort t, GLshort r) {
	fprintf( tr.logFile, "glTexCoord3s %d %d %d\n", s, t, r );
	dllTexCoord3s(s, t, r);
}

static void APIENTRY logTexCoord3sv(const GLshort *v) {
// unknown type: "const GLshort *" name: "v"
	fprintf( tr.logFile, "glTexCoord3sv 'const GLshort * v'\n" );
	dllTexCoord3sv(v);
}

static void APIENTRY logTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q) {
	fprintf( tr.logFile, "glTexCoord4d %g %g %g %g\n", s, t, r, q );
	dllTexCoord4d(s, t, r, q);
}

static void APIENTRY logTexCoord4dv(const GLdouble *v) {
// unknown type: "const GLdouble *" name: "v"
	fprintf( tr.logFile, "glTexCoord4dv 'const GLdouble * v'\n" );
	dllTexCoord4dv(v);
}

static void APIENTRY logTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
	fprintf( tr.logFile, "glTexCoord4f %g %g %g %g\n", s, t, r, q );
	dllTexCoord4f(s, t, r, q);
}

static void APIENTRY logTexCoord4fv(const GLfloat *v) {
// unknown type: "const GLfloat *" name: "v"
	fprintf( tr.logFile, "glTexCoord4fv 'const GLfloat * v'\n" );
	dllTexCoord4fv(v);
}

static void APIENTRY logTexCoord4i(GLint s, GLint t, GLint r, GLint q) {
	fprintf( tr.logFile, "glTexCoord4i %d %d %d %d\n", s, t, r, q );
	dllTexCoord4i(s, t, r, q);
}

static void APIENTRY logTexCoord4iv(const GLint *v) {
// unknown type: "const GLint *" name: "v"
	fprintf( tr.logFile, "glTexCoord4iv 'const GLint * v'\n" );
	dllTexCoord4iv(v);
}

static void APIENTRY logTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q) {
	fprintf( tr.logFile, "glTexCoord4s %d %d %d %d\n", s, t, r, q );
	dllTexCoord4s(s, t, r, q);
}

static void APIENTRY logTexCoord4sv(const GLshort *v) {
// unknown type: "const GLshort *" name: "v"
	fprintf( tr.logFile, "glTexCoord4sv 'const GLshort * v'\n" );
	dllTexCoord4sv(v);
}

static void APIENTRY logTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
// unknown type: "const GLvoid *" name: "pointer"
	fprintf( tr.logFile, "glTexCoordPointer %d %s %d 'const GLvoid * pointer'\n", size, EnumString(type), stride );
	dllTexCoordPointer(size, type, stride, pointer);
}

static void APIENTRY logTexEnvf(GLenum target, GLenum pname, GLfloat param) {
	fprintf( tr.logFile, "glTexEnvf %s %s %g\n", EnumString(target), EnumString(pname), param );
	dllTexEnvf(target, pname, param);
}

static void APIENTRY logTexEnvfv(GLenum target, GLenum pname, const GLfloat *params) {
// unknown type: "const GLfloat *" name: "params"
	fprintf( tr.logFile, "glTexEnvfv %s %s 'const GLfloat * params'\n", EnumString(target), EnumString(pname) );
	dllTexEnvfv(target, pname, params);
}

static void APIENTRY logTexEnvi(GLenum target, GLenum pname, GLint param) {
	fprintf( tr.logFile, "glTexEnvi %s %s %d\n", EnumString(target), EnumString(pname), param );
	dllTexEnvi(target, pname, param);
}

static void APIENTRY logTexEnviv(GLenum target, GLenum pname, const GLint *params) {
// unknown type: "const GLint *" name: "params"
	fprintf( tr.logFile, "glTexEnviv %s %s 'const GLint * params'\n", EnumString(target), EnumString(pname) );
	dllTexEnviv(target, pname, params);
}

static void APIENTRY logTexGend(GLenum coord, GLenum pname, GLdouble param) {
	fprintf( tr.logFile, "glTexGend %s %s %g\n", EnumString(coord), EnumString(pname), param );
	dllTexGend(coord, pname, param);
}

static void APIENTRY logTexGendv(GLenum coord, GLenum pname, const GLdouble *params) {
// unknown type: "const GLdouble *" name: "params"
	fprintf( tr.logFile, "glTexGendv %s %s 'const GLdouble * params'\n", EnumString(coord), EnumString(pname) );
	dllTexGendv(coord, pname, params);
}

static void APIENTRY logTexGenf(GLenum coord, GLenum pname, GLfloat param) {
	fprintf( tr.logFile, "glTexGenf %s %s %g\n", EnumString(coord), EnumString(pname), param );
	dllTexGenf(coord, pname, param);
}

static void APIENTRY logTexGenfv(GLenum coord, GLenum pname, const GLfloat *params) {
// unknown type: "const GLfloat *" name: "params"
	fprintf( tr.logFile, "glTexGenfv %s %s 'const GLfloat * params'\n", EnumString(coord), EnumString(pname) );
	dllTexGenfv(coord, pname, params);
}

static void APIENTRY logTexGeni(GLenum coord, GLenum pname, GLint param) {
	fprintf( tr.logFile, "glTexGeni %s %s %d\n", EnumString(coord), EnumString(pname), param );
	dllTexGeni(coord, pname, param);
}

static void APIENTRY logTexGeniv(GLenum coord, GLenum pname, const GLint *params) {
// unknown type: "const GLint *" name: "params"
	fprintf( tr.logFile, "glTexGeniv %s %s 'const GLint * params'\n", EnumString(coord), EnumString(pname) );
	dllTexGeniv(coord, pname, params);
}

static void APIENTRY logTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels) {
// unknown type: "const GLvoid *" name: "pixels"
	fprintf( tr.logFile, "glTexImage1D %s %d %d %d %d %s %s 'const GLvoid * pixels'\n", EnumString(target), level, internalformat, width, border, EnumString(format), EnumString(type) );
	dllTexImage1D(target, level, internalformat, width, border, format, type, pixels);
}

static void APIENTRY logTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) {
// unknown type: "const GLvoid *" name: "pixels"
	fprintf( tr.logFile, "glTexImage2D %s %d %d %d %d %d %s %s 'const GLvoid * pixels'\n", EnumString(target), level, internalformat, width, height, border, EnumString(format), EnumString(type) );
	dllTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

static void APIENTRY logTexParameterf(GLenum target, GLenum pname, GLfloat param) {
	fprintf( tr.logFile, "glTexParameterf %s %s %g\n", EnumString(target), EnumString(pname), param );
	dllTexParameterf(target, pname, param);
}

static void APIENTRY logTexParameterfv(GLenum target, GLenum pname, const GLfloat *params) {
// unknown type: "const GLfloat *" name: "params"
	fprintf( tr.logFile, "glTexParameterfv %s %s 'const GLfloat * params'\n", EnumString(target), EnumString(pname) );
	dllTexParameterfv(target, pname, params);
}

static void APIENTRY logTexParameteri(GLenum target, GLenum pname, GLint param) {
	fprintf( tr.logFile, "glTexParameteri %s %s %d\n", EnumString(target), EnumString(pname), param );
	dllTexParameteri(target, pname, param);
}

static void APIENTRY logTexParameteriv(GLenum target, GLenum pname, const GLint *params) {
// unknown type: "const GLint *" name: "params"
	fprintf( tr.logFile, "glTexParameteriv %s %s 'const GLint * params'\n", EnumString(target), EnumString(pname) );
	dllTexParameteriv(target, pname, params);
}

static void APIENTRY logTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels) {
// unknown type: "const GLvoid *" name: "pixels"
	fprintf( tr.logFile, "glTexSubImage1D %s %d %d %d %s %s 'const GLvoid * pixels'\n", EnumString(target), level, xoffset, width, EnumString(format), EnumString(type) );
	dllTexSubImage1D(target, level, xoffset, width, format, type, pixels);
}

static void APIENTRY logTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels) {
// unknown type: "const GLvoid *" name: "pixels"
	fprintf( tr.logFile, "glTexSubImage2D %s %d %d %d %d %d %s %s 'const GLvoid * pixels'\n", EnumString(target), level, xoffset, yoffset, width, height, EnumString(format), EnumString(type) );
	dllTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

static void APIENTRY logTranslated(GLdouble x, GLdouble y, GLdouble z) {
	fprintf( tr.logFile, "glTranslated %g %g %g\n", x, y, z );
	dllTranslated(x, y, z);
}

static void APIENTRY logTranslatef(GLfloat x, GLfloat y, GLfloat z) {
	fprintf( tr.logFile, "glTranslatef %g %g %g\n", x, y, z );
	dllTranslatef(x, y, z);
}

static void APIENTRY logVertex2d(GLdouble x, GLdouble y) {
	fprintf( tr.logFile, "glVertex2d %g %g\n", x, y );
	dllVertex2d(x, y);
}

static void APIENTRY logVertex2dv(const GLdouble *v) {
// unknown type: "const GLdouble *" name: "v"
	fprintf( tr.logFile, "glVertex2dv 'const GLdouble * v'\n" );
	dllVertex2dv(v);
}

static void APIENTRY logVertex2f(GLfloat x, GLfloat y) {
	fprintf( tr.logFile, "glVertex2f %g %g\n", x, y );
	dllVertex2f(x, y);
}

static void APIENTRY logVertex2fv(const GLfloat *v) {
// unknown type: "const GLfloat *" name: "v"
	fprintf( tr.logFile, "glVertex2fv 'const GLfloat * v'\n" );
	dllVertex2fv(v);
}

static void APIENTRY logVertex2i(GLint x, GLint y) {
	fprintf( tr.logFile, "glVertex2i %d %d\n", x, y );
	dllVertex2i(x, y);
}

static void APIENTRY logVertex2iv(const GLint *v) {
// unknown type: "const GLint *" name: "v"
	fprintf( tr.logFile, "glVertex2iv 'const GLint * v'\n" );
	dllVertex2iv(v);
}

static void APIENTRY logVertex2s(GLshort x, GLshort y) {
	fprintf( tr.logFile, "glVertex2s %d %d\n", x, y );
	dllVertex2s(x, y);
}

static void APIENTRY logVertex2sv(const GLshort *v) {
// unknown type: "const GLshort *" name: "v"
	fprintf( tr.logFile, "glVertex2sv 'const GLshort * v'\n" );
	dllVertex2sv(v);
}

static void APIENTRY logVertex3d(GLdouble x, GLdouble y, GLdouble z) {
	fprintf( tr.logFile, "glVertex3d %g %g %g\n", x, y, z );
	dllVertex3d(x, y, z);
}

static void APIENTRY logVertex3dv(const GLdouble *v) {
// unknown type: "const GLdouble *" name: "v"
	fprintf( tr.logFile, "glVertex3dv 'const GLdouble * v'\n" );
	dllVertex3dv(v);
}

static void APIENTRY logVertex3f(GLfloat x, GLfloat y, GLfloat z) {
	fprintf( tr.logFile, "glVertex3f %g %g %g\n", x, y, z );
	dllVertex3f(x, y, z);
}

static void APIENTRY logVertex3fv(const GLfloat *v) {
// unknown type: "const GLfloat *" name: "v"
	fprintf( tr.logFile, "glVertex3fv 'const GLfloat * v'\n" );
	dllVertex3fv(v);
}

static void APIENTRY logVertex3i(GLint x, GLint y, GLint z) {
	fprintf( tr.logFile, "glVertex3i %d %d %d\n", x, y, z );
	dllVertex3i(x, y, z);
}

static void APIENTRY logVertex3iv(const GLint *v) {
// unknown type: "const GLint *" name: "v"
	fprintf( tr.logFile, "glVertex3iv 'const GLint * v'\n" );
	dllVertex3iv(v);
}

static void APIENTRY logVertex3s(GLshort x, GLshort y, GLshort z) {
	fprintf( tr.logFile, "glVertex3s %d %d %d\n", x, y, z );
	dllVertex3s(x, y, z);
}

static void APIENTRY logVertex3sv(const GLshort *v) {
// unknown type: "const GLshort *" name: "v"
	fprintf( tr.logFile, "glVertex3sv 'const GLshort * v'\n" );
	dllVertex3sv(v);
}

static void APIENTRY logVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
	fprintf( tr.logFile, "glVertex4d %g %g %g %g\n", x, y, z, w );
	dllVertex4d(x, y, z, w);
}

static void APIENTRY logVertex4dv(const GLdouble *v) {
// unknown type: "const GLdouble *" name: "v"
	fprintf( tr.logFile, "glVertex4dv 'const GLdouble * v'\n" );
	dllVertex4dv(v);
}

static void APIENTRY logVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
	fprintf( tr.logFile, "glVertex4f %g %g %g %g\n", x, y, z, w );
	dllVertex4f(x, y, z, w);
}

static void APIENTRY logVertex4fv(const GLfloat *v) {
// unknown type: "const GLfloat *" name: "v"
	fprintf( tr.logFile, "glVertex4fv 'const GLfloat * v'\n" );
	dllVertex4fv(v);
}

static void APIENTRY logVertex4i(GLint x, GLint y, GLint z, GLint w) {
	fprintf( tr.logFile, "glVertex4i %d %d %d %d\n", x, y, z, w );
	dllVertex4i(x, y, z, w);
}

static void APIENTRY logVertex4iv(const GLint *v) {
// unknown type: "const GLint *" name: "v"
	fprintf( tr.logFile, "glVertex4iv 'const GLint * v'\n" );
	dllVertex4iv(v);
}

static void APIENTRY logVertex4s(GLshort x, GLshort y, GLshort z, GLshort w) {
	fprintf( tr.logFile, "glVertex4s %d %d %d %d\n", x, y, z, w );
	dllVertex4s(x, y, z, w);
}

static void APIENTRY logVertex4sv(const GLshort *v) {
// unknown type: "const GLshort *" name: "v"
	fprintf( tr.logFile, "glVertex4sv 'const GLshort * v'\n" );
	dllVertex4sv(v);
}

static void APIENTRY logVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
// unknown type: "const GLvoid *" name: "pointer"
	fprintf( tr.logFile, "glVertexPointer %d %s %d 'const GLvoid * pointer'\n", size, EnumString(type), stride );
	dllVertexPointer(size, type, stride, pointer);
}

static void APIENTRY logViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
	fprintf( tr.logFile, "glViewport %d %d %d %d\n", x, y, width, height );
	dllViewport(x, y, width, height);
}


#ifdef __linux__

static XVisualInfo * APIENTRY logChooseVisual(Display *dpy, int screen, int *attribList) {
// unknown type: "Display *" name: "dpy"
// unknown type: "int" name: "screen"
// unknown type: "int *" name: "attribList"
	fprintf( tr.logFile, "glXChooseVisual 'Display * dpy' 'int screen' 'int * attribList'\n" );
	return dllChooseVisual(dpy, screen, attribList);
}

static GLXContext APIENTRY logCreateContext(Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct) {
// unknown type: "Display *" name: "dpy"
// unknown type: "XVisualInfo *" name: "vis"
// unknown type: "GLXContext" name: "shareList"
// unknown type: "Bool" name: "direct"
	fprintf( tr.logFile, "glXCreateContext 'Display * dpy' 'XVisualInfo * vis' 'GLXContext shareList' 'Bool direct'\n" );
	return dllCreateContext(dpy, vis, shareList, direct);
}

static void APIENTRY logDestroyContext(Display *dpy, GLXContext ctx) {
// unknown type: "Display *" name: "dpy"
// unknown type: "GLXContext" name: "ctx"
	fprintf( tr.logFile, "glXDestroyContext 'Display * dpy' 'GLXContext ctx'\n" );
	dllDestroyContext(dpy, ctx);
}

static Bool APIENTRY logMakeCurrent(Display *dpy, GLXDrawable drawable, GLXContext ctx) {
// unknown type: "Display *" name: "dpy"
// unknown type: "GLXDrawable" name: "drawable"
// unknown type: "GLXContext" name: "ctx"
	fprintf( tr.logFile, "glXMakeCurrent 'Display * dpy' 'GLXDrawable drawable' 'GLXContext ctx'\n" );
	return dllMakeCurrent(dpy, drawable, ctx);
}

static void APIENTRY logSwapBuffers(Display *dpy, GLXDrawable drawable) {
// unknown type: "Display *" name: "dpy"
// unknown type: "GLXDrawable" name: "drawable"
	fprintf( tr.logFile, "glXSwapBuffers 'Display * dpy' 'GLXDrawable drawable'\n" );
	dllSwapBuffers(dpy, drawable);
}


#endif


#ifdef WIN32


#endif

