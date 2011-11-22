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
#pragma hdrstop

#include "../../renderer/tr_local.h"

void glAccum(GLenum op, GLfloat value){};
void glAlphaFunc(GLenum func, GLclampf ref){};
GLboolean glAreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences){};
void glArrayElement(GLint i){};
void glBegin(GLenum mode){};
void glBindTexture(GLenum target, GLuint texture){};
void glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap){};
void glBlendFunc(GLenum sfactor, GLenum dfactor){};
void glCallList(GLuint list){};
void glCallLists(GLsizei n, GLenum type, const GLvoid *lists){};
void glClear(GLbitfield mask){};
void glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha){};
void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha){};
void glClearDepth(GLclampd depth){};
void glClearIndex(GLfloat c){};
void glClearStencil(GLint s){};
void glClipPlane(GLenum plane, const GLdouble *equation){};
void glColor3b(GLbyte red, GLbyte green, GLbyte blue){};
void glColor3bv(const GLbyte *v){};
void glColor3d(GLdouble red, GLdouble green, GLdouble blue){};
void glColor3dv(const GLdouble *v){};
void glColor3f(GLfloat red, GLfloat green, GLfloat blue){};
void glColor3fv(const GLfloat *v){};
void glColor3i(GLint red, GLint green, GLint blue){};
void glColor3iv(const GLint *v){};
void glColor3s(GLshort red, GLshort green, GLshort blue){};
void glColor3sv(const GLshort *v){};
void glColor3ub(GLubyte red, GLubyte green, GLubyte blue){};
void glColor3ubv(const GLubyte *v){};
void glColor3ui(GLuint red, GLuint green, GLuint blue){};
void glColor3uiv(const GLuint *v){};
void glColor3us(GLushort red, GLushort green, GLushort blue){};
void glColor3usv(const GLushort *v){};
void glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha){};
void glColor4bv(const GLbyte *v){};
void glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha){};
void glColor4dv(const GLdouble *v){};
void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha){};
void glColor4fv(const GLfloat *v){};
void glColor4i(GLint red, GLint green, GLint blue, GLint alpha){};
void glColor4iv(const GLint *v){};
void glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha){};
void glColor4sv(const GLshort *v){};
void glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha){};
void glColor4ubv(const GLubyte *v){};
void glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha){};
void glColor4uiv(const GLuint *v){};
void glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha){};
void glColor4usv(const GLushort *v){};
void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha){};
void glColorMaterial(GLenum face, GLenum mode){};
void glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer){};
void glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type){};
void glCopyTexImage1D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border){};
void glCopyTexImage2D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border){};
void glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width){};
void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height){};
void glCullFace(GLenum mode){};
void glDeleteLists(GLuint list, GLsizei range){};
void glDeleteTextures(GLsizei n, const GLuint *textures){};
void glDepthFunc(GLenum func){};
void glDepthMask(GLboolean flag){};
void glDepthRange(GLclampd zNear, GLclampd zFar){};
void glDisable(GLenum cap){};
void glDisableClientState(GLenum array){};
void glDrawArrays(GLenum mode, GLint first, GLsizei count){};
void glDrawBuffer(GLenum mode){};
void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices){};
void glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels){};
void glEdgeFlag(GLboolean flag){};
void glEdgeFlagPointer(GLsizei stride, const GLvoid *pointer){};
void glEdgeFlagv(const GLboolean *flag){};
void glEnable(GLenum cap){};
void glEnableClientState(GLenum array){};
void glEnd(void){};
void glEndList(void){};
void glEvalCoord1d(GLdouble u){};
void glEvalCoord1dv(const GLdouble *u){};
void glEvalCoord1f(GLfloat u){};
void glEvalCoord1fv(const GLfloat *u){};
void glEvalCoord2d(GLdouble u, GLdouble v){};
void glEvalCoord2dv(const GLdouble *u){};
void glEvalCoord2f(GLfloat u, GLfloat v){};
void glEvalCoord2fv(const GLfloat *u){};
void glEvalMesh1(GLenum mode, GLint i1, GLint i2){};
void glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2){};
void glEvalPoint1(GLint i){};
void glEvalPoint2(GLint i, GLint j){};
void glFeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer){};
void glFinish(void){};
void glFlush(void){};
void glFogf(GLenum pname, GLfloat param){};
void glFogfv(GLenum pname, const GLfloat *params){};
void glFogi(GLenum pname, GLint param){};
void glFogiv(GLenum pname, const GLint *params){};
void glFrontFace(GLenum mode){};
void glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar){};
GLuint glGenLists(GLsizei range){return 0;};
void glGenTextures(GLsizei n, GLuint *textures){};
void glGetBooleanv(GLenum pname, GLboolean *params){};
void glGetClipPlane(GLenum plane, GLdouble *equation){};
void glGetDoublev(GLenum pname, GLdouble *params){};
GLenum glGetError(void){return 0;};
void glGetFloatv(GLenum pname, GLfloat *params){};
void glGetIntegerv(GLenum pname, GLint *params){
	switch( pname ) {
		case GL_MAX_TEXTURE_SIZE: *params = 1024; break;
		case GL_MAX_TEXTURE_UNITS_ARB: *params = 2; break;
		default: *params = 0; break;
	}
};
void glGetLightfv(GLenum light, GLenum pname, GLfloat *params){};
void glGetLightiv(GLenum light, GLenum pname, GLint *params){};
void glGetMapdv(GLenum target, GLenum query, GLdouble *v){};
void glGetMapfv(GLenum target, GLenum query, GLfloat *v){};
void glGetMapiv(GLenum target, GLenum query, GLint *v){};
void glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params){};
void glGetMaterialiv(GLenum face, GLenum pname, GLint *params){};
void glGetPixelMapfv(GLenum map, GLfloat *values){};
void glGetPixelMapuiv(GLenum map, GLuint *values){};
void glGetPixelMapusv(GLenum map, GLushort *values){};
void glGetPointerv(GLenum pname, GLvoid* *params){};
void glGetPolygonStipple(GLubyte *mask){};
const GLubyte * glGetString(GLenum name){
	switch( name ) {
		case GL_EXTENSIONS: return (GLubyte *)"GL_ARB_multitexture GL_ARB_texture_env_combine GL_ARB_texture_cube_map GL_ARB_texture_env_dot3";
	}
	return (const GLubyte *)"";
};
void glGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params){};
void glGetTexEnviv(GLenum target, GLenum pname, GLint *params){};
void glGetTexGendv(GLenum coord, GLenum pname, GLdouble *params){};
void glGetTexGenfv(GLenum coord, GLenum pname, GLfloat *params){};
void glGetTexGeniv(GLenum coord, GLenum pname, GLint *params){};
void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels){};
void glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params){};
void glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params){};
void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params){};
void glGetTexParameteriv(GLenum target, GLenum pname, GLint *params){};
void glHint(GLenum target, GLenum mode){};
void glIndexMask(GLuint mask){};
void glIndexPointer(GLenum type, GLsizei stride, const GLvoid *pointer){};
void glIndexd(GLdouble c){};
void glIndexdv(const GLdouble *c){};
void glIndexf(GLfloat c){};
void glIndexfv(const GLfloat *c){};
void glIndexi(GLint c){};
void glIndexiv(const GLint *c){};
void glIndexs(GLshort c){};
void glIndexsv(const GLshort *c){};
void glIndexub(GLubyte c){};
void glIndexubv(const GLubyte *c){};
void glInitNames(void){};
void glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer){};
GLboolean glIsEnabled(GLenum cap){};
GLboolean glIsList(GLuint list){};
GLboolean glIsTexture(GLuint texture){};
void glLightModelf(GLenum pname, GLfloat param){};
void glLightModelfv(GLenum pname, const GLfloat *params){};
void glLightModeli(GLenum pname, GLint param){};
void glLightModeliv(GLenum pname, const GLint *params){};
void glLightf(GLenum light, GLenum pname, GLfloat param){};
void glLightfv(GLenum light, GLenum pname, const GLfloat *params){};
void glLighti(GLenum light, GLenum pname, GLint param){};
void glLightiv(GLenum light, GLenum pname, const GLint *params){};
void glLineStipple(GLint factor, GLushort pattern){};
void glLineWidth(GLfloat width){};
void glListBase(GLuint base){};
void glLoadIdentity(void){};
void glLoadMatrixd(const GLdouble *m){};
void glLoadMatrixf(const GLfloat *m){};
void glLoadName(GLuint name){};
void glLogicOp(GLenum opcode){};
void glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points){};
void glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points){};
void glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points){};
void glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points){};
void glMapGrid1d(GLint un, GLdouble u1, GLdouble u2){};
void glMapGrid1f(GLint un, GLfloat u1, GLfloat u2){};
void glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2){};
void glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2){};
void glMaterialf(GLenum face, GLenum pname, GLfloat param){};
void glMaterialfv(GLenum face, GLenum pname, const GLfloat *params){};
void glMateriali(GLenum face, GLenum pname, GLint param){};
void glMaterialiv(GLenum face, GLenum pname, const GLint *params){};
void glMatrixMode(GLenum mode){};
void glMultMatrixd(const GLdouble *m){};
void glMultMatrixf(const GLfloat *m){};
void glNewList(GLuint list, GLenum mode){};
void glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz){};
void glNormal3bv(const GLbyte *v){};
void glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz){};
void glNormal3dv(const GLdouble *v){};
void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz){};
void glNormal3fv(const GLfloat *v){};
void glNormal3i(GLint nx, GLint ny, GLint nz){};
void glNormal3iv(const GLint *v){};
void glNormal3s(GLshort nx, GLshort ny, GLshort nz){};
void glNormal3sv(const GLshort *v){};
void glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer){};
void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar){};
void glPassThrough(GLfloat token){};
void glPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat *values){};
void glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint *values){};
void glPixelMapusv(GLenum map, GLsizei mapsize, const GLushort *values){};
void glPixelStoref(GLenum pname, GLfloat param){};
void glPixelStorei(GLenum pname, GLint param){};
void glPixelTransferf(GLenum pname, GLfloat param){};
void glPixelTransferi(GLenum pname, GLint param){};
void glPixelZoom(GLfloat xfactor, GLfloat yfactor){};
void glPointSize(GLfloat size){};
void glPolygonMode(GLenum face, GLenum mode){};
void glPolygonOffset(GLfloat factor, GLfloat units){};
void glPolygonStipple(const GLubyte *mask){};
void glPopAttrib(void){};
void glPopClientAttrib(void){};
void glPopMatrix(void){};
void glPopName(void){};
void glPrioritizeTextures(GLsizei n, const GLuint *textures, const GLclampf *priorities){};
void glPushAttrib(GLbitfield mask){};
void glPushClientAttrib(GLbitfield mask){};
void glPushMatrix(void){};
void glPushName(GLuint name){};
void glRasterPos2d(GLdouble x, GLdouble y){};
void glRasterPos2dv(const GLdouble *v){};
void glRasterPos2f(GLfloat x, GLfloat y){};
void glRasterPos2fv(const GLfloat *v){};
void glRasterPos2i(GLint x, GLint y){};
void glRasterPos2iv(const GLint *v){};
void glRasterPos2s(GLshort x, GLshort y){};
void glRasterPos2sv(const GLshort *v){};
void glRasterPos3d(GLdouble x, GLdouble y, GLdouble z){};
void glRasterPos3dv(const GLdouble *v){};
void glRasterPos3f(GLfloat x, GLfloat y, GLfloat z){};
void glRasterPos3fv(const GLfloat *v){};
void glRasterPos3i(GLint x, GLint y, GLint z){};
void glRasterPos3iv(const GLint *v){};
void glRasterPos3s(GLshort x, GLshort y, GLshort z){};
void glRasterPos3sv(const GLshort *v){};
void glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w){};
void glRasterPos4dv(const GLdouble *v){};
void glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w){};
void glRasterPos4fv(const GLfloat *v){};
void glRasterPos4i(GLint x, GLint y, GLint z, GLint w){};
void glRasterPos4iv(const GLint *v){};
void glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w){};
void glRasterPos4sv(const GLshort *v){};
void glReadBuffer(GLenum mode){};
void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels){};
void glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2){};
void glRectdv(const GLdouble *v1, const GLdouble *v2){};
void glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2){};
void glRectfv(const GLfloat *v1, const GLfloat *v2){};
void glRecti(GLint x1, GLint y1, GLint x2, GLint y2){};
void glRectiv(const GLint *v1, const GLint *v2){};
void glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2){};
void glRectsv(const GLshort *v1, const GLshort *v2){};
GLint glRenderMode(GLenum mode){};
void glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z){};
void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z){};
void glScaled(GLdouble x, GLdouble y, GLdouble z){};
void glScalef(GLfloat x, GLfloat y, GLfloat z){};
void glScissor(GLint x, GLint y, GLsizei width, GLsizei height){};
void glSelectBuffer(GLsizei size, GLuint *buffer){};
void glShadeModel(GLenum mode){};
void glStencilFunc(GLenum func, GLint ref, GLuint mask){};
void glStencilMask(GLuint mask){};
void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass){};
void glTexCoord1d(GLdouble s){};
void glTexCoord1dv(const GLdouble *v){};
void glTexCoord1f(GLfloat s){};
void glTexCoord1fv(const GLfloat *v){};
void glTexCoord1i(GLint s){};
void glTexCoord1iv(const GLint *v){};
void glTexCoord1s(GLshort s){};
void glTexCoord1sv(const GLshort *v){};
void glTexCoord2d(GLdouble s, GLdouble t){};
void glTexCoord2dv(const GLdouble *v){};
void glTexCoord2f(GLfloat s, GLfloat t){};
void glTexCoord2fv(const GLfloat *v){};
void glTexCoord2i(GLint s, GLint t){};
void glTexCoord2iv(const GLint *v){};
void glTexCoord2s(GLshort s, GLshort t){};
void glTexCoord2sv(const GLshort *v){};
void glTexCoord3d(GLdouble s, GLdouble t, GLdouble r){};
void glTexCoord3dv(const GLdouble *v){};
void glTexCoord3f(GLfloat s, GLfloat t, GLfloat r){};
void glTexCoord3fv(const GLfloat *v){};
void glTexCoord3i(GLint s, GLint t, GLint r){};
void glTexCoord3iv(const GLint *v){};
void glTexCoord3s(GLshort s, GLshort t, GLshort r){};
void glTexCoord3sv(const GLshort *v){};
void glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q){};
void glTexCoord4dv(const GLdouble *v){};
void glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q){};
void glTexCoord4fv(const GLfloat *v){};
void glTexCoord4i(GLint s, GLint t, GLint r, GLint q){};
void glTexCoord4iv(const GLint *v){};
void glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q){};
void glTexCoord4sv(const GLshort *v){};
void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer){};
void glTexEnvf(GLenum target, GLenum pname, GLfloat param){};
void glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params){};
void glTexEnvi(GLenum target, GLenum pname, GLint param){};
void glTexEnviv(GLenum target, GLenum pname, const GLint *params){};
void glTexGend(GLenum coord, GLenum pname, GLdouble param){};
void glTexGendv(GLenum coord, GLenum pname, const GLdouble *params){};
void glTexGenf(GLenum coord, GLenum pname, GLfloat param){};
void glTexGenfv(GLenum coord, GLenum pname, const GLfloat *params){};
void glTexGeni(GLenum coord, GLenum pname, GLint param){};
void glTexGeniv(GLenum coord, GLenum pname, const GLint *params){};
void glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels){};
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) {};
void glTexParameterf(GLenum target, GLenum pname, GLfloat param){};
void glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params){};
void glTexParameteri(GLenum target, GLenum pname, GLint param){};
void glTexParameteriv(GLenum target, GLenum pname, const GLint *params){};
void glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels){};
void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels){};
void glTranslated(GLdouble x, GLdouble y, GLdouble z){};
void glTranslatef(GLfloat x, GLfloat y, GLfloat z){};
void glVertex2d(GLdouble x, GLdouble y){};
void glVertex2dv(const GLdouble *v){};
void glVertex2f(GLfloat x, GLfloat y){};
void glVertex2fv(const GLfloat *v){};
void glVertex2i(GLint x, GLint y){};
void glVertex2iv(const GLint *v){};
void glVertex2s(GLshort x, GLshort y){};
void glVertex2sv(const GLshort *v){};
void glVertex3d(GLdouble x, GLdouble y, GLdouble z){};
void glVertex3dv(const GLdouble *v){};
void glVertex3f(GLfloat x, GLfloat y, GLfloat z){};
void glVertex3fv(const GLfloat *v){};
void glVertex3i(GLint x, GLint y, GLint z){};
void glVertex3iv(const GLint *v){};
void glVertex3s(GLshort x, GLshort y, GLshort z){};
void glVertex3sv(const GLshort *v){};
void glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w){};
void glVertex4dv(const GLdouble *v){};
void glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w){};
void glVertex4fv(const GLfloat *v){};
void glVertex4i(GLint x, GLint y, GLint z, GLint w){};
void glVertex4iv(const GLint *v){};
void glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w){};
void glVertex4sv(const GLshort *v){};
void glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer){};
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height){};

void GLimp_WakeBackEnd(void*a) {};
void GLimp_EnableLogging(bool) {};
void GLimp_FrontEndSleep() {};
void GLimp_ActivateContext() {};
void GLimp_DeactivateContext() {};
bool GLimp_SpawnRenderThread(void (*a)()) {return false;};

static void StubFunction( void ) {};
GLExtension_t GLimp_ExtensionPointer( const char *a) { return StubFunction; };

bool GLimp_Init(glimpParms_t a) {return true;};
void GLimp_SetGamma(unsigned short*a, unsigned short*b, unsigned short*c) {};
void GLimp_Shutdown() {};
void GLimp_SwapBuffers() {};
void *GLimp_BackEndSleep() {return 0;};
