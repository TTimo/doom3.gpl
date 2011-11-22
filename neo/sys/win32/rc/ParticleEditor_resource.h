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

#define IDD_DIALOG_PARTICLE_EDITOR               7000

#define IDC_BUTTON_UPDATE                        7200
#define IDC_BUTTON_SAVE_PARTICLE                 7201
#define IDC_COMBO_PARTICLES                      7202
#define IDC_BUTTON_NEW                           7203
#define IDC_LIST_STAGES                          7204
#define IDC_BUTTON_ADDSTAGE                      7205
#define IDC_BUTTON_REMOVESTAGE                   7206
#define IDC_EDIT_MATERIAL                        7207
#define IDC_BUTTON_BROWSEMATERIAL                7208
#define IDC_EDIT_DEPTHHACK                       7209
#define IDC_CHECK_ONESHOT                        7210
#define IDC_EDIT_COUNT                           7211
#define IDC_SLIDER_COUNT                         7212
#define IDC_CHECK_WORLDGRAVITY                   7213
#define IDC_EDIT_TIME                            7214
#define IDC_SLIDER_TIME                          7215
#define IDC_EDIT_TIMEOFFSET                      7216
#define IDC_EDIT_BUNCHING                        7217
#define IDC_EDIT_DEADTIME                        7218
#define IDC_RADIO_RECT                           7219
#define IDC_RADIO_SPHERE                         7220
#define IDC_RADIO_CYLINDER                       7221
#define IDC_RADIO_SPHERE2                        7222
#define IDC_CHECK_WORLDGRAVITY2                  7223
#define IDC_CHECK_ENTITYCOLOR                    7224
#define IDC_EDIT_XSIZE                           7225
#define IDC_EDIT_ZSIZE                           7226
#define IDC_EDIT_YSIZE                           7227
#define IDC_EDIT_RINGOFFSET                      7228
#define IDC_RADIO_CONE                           7229
#define IDC_RADIO_OUTWARD                        7230
#define IDC_EDIT_DIRECTIONPARM                   7231
#define IDC_RADIO_VIEW                           7232
#define IDC_RADIO_AIMED                          7233
#define IDC_RADIO_X                              7234
#define IDC_RADIO_Y                              7235
#define IDC_RADIO_Z                              7236
#define IDC_EDIT_CUSTOMPATH                      7237
#define IDC_CHECK_RANDOMANGLE2                   7238
#define IDC_CHECK_RANDOMDISTRIBUTION             7239
#define IDC_EDIT_CYCLES                          7240
#define IDC_EDIT_COLOR                           7241
#define IDC_BUTTON_BROWSECOLOR                   7242
#define IDC_EDIT_SPEEDFROM                       7243
#define IDC_EDIT_SPEEDTO                         7244
#define IDC_EDIT_ROTATIONFROM                    7245
#define IDC_EDIT_ROTATIONTO                      7246
#define IDC_EDIT_SIZEFROM                        7247
#define IDC_EDIT_SIZETO                          7248
#define IDC_EDIT_ASPECTFROM                      7249
#define IDC_EDIT_ASPECTTO                        7250
#define IDC_EDIT_FADECOLOR                       7251
#define IDC_BUTTON_BROWSEFADECOLOR               7252
#define IDC_EDIT_FADEIN                          7253
#define IDC_EDIT_FADEOUT                         7254
#define IDC_EDIT_ANIMFRAMES                      7255
#define IDC_EDIT_ANIMRATE                        7256
#define IDC_EDIT_GRAVITY                         7257
#define IDC_EDIT_OFFSET                          7258
#define IDC_STATIC_DIRPARM                       7259
#define IDC_BUTTON_HIDESTAGE                     7260
#define IDC_RADIO_VIEWORIGIN                     7261
#define IDC_EDIT_CUSTOMPARMS                     7262
#define IDC_CHECK_EDITPARTICLEMODE               7263
#define IDC_RADIO_VIEWPROJECTILE                 7264
#define IDC_EDIT_FADEFRACTION                    7265
#define IDC_BUTTON_SAVE_PARTICLE_AS              7266
#define IDC_RADIO_VIEWIMPACT                     7267
#define IDC_BUTTON_BROWSECOLOR_ENTITY            7268
#define IDC_BUTTON_SAVE_ONENTITY                 7269
#define IDC_BUTTON_SHOWSTAGE                     7270
#define IDC_RADIO_VIEWEXPLOSION                  7271
#define IDC_BUTTON_SAVE_PARTICLEENTITIES         7272
#define IDC_EDIT_ORIENTATIONPARM1                7273
#define IDC_EDIT_ORIENTATIONPARM2                7274
#define IDC_SLIDER_BUNCHING                      7275
#define IDC_SLIDER_SPEEDFROM                     7276
#define IDC_SLIDER_SPEEDTO                       7277
#define IDC_SLIDER_SIZEFROM                      7278
#define IDC_SLIDER_SIZETO                        7279
#define IDC_SLIDER_ROTATIONFROM                  7280
#define IDC_SLIDER_ROTATIONTO                    7281
#define IDC_SLIDER_ASPECTFROM                    7282
#define IDC_SLIDER_ASPECTTO                      7283
#define IDC_SLIDER_GRAVITY                       7284
#define IDC_SLIDER_FADEIN                        7285
#define IDC_SLIDER_FADEOUT                       7286
#define IDC_COMBO_CUSTOMPATH                     7287
#define IDC_STATIC_INFILE                        7288
#define IDC_BUTTON_TESTMODEL                     7289
#define IDC_BUTTON_IMPACT                        7290
#define IDC_BUTTON_MUZZLE                        7291
#define IDC_BUTTON_FLIGHT                        7292
#define IDC_BUTTON_SELECTED                      7293
#define IDC_BUTTON_DOOM                          7294
#define IDC_BUTTON_DROPEMITTER                   7295
#define IDC_BUTTON1                              7296
#define IDC_BUTTON_VECTOR                        7297
#define IDC_SLIDER_FADEFRACTION                  7298
#define IDC_EDIT_BOUNDSEXPANSION                 7299
#define IDC_STATIC_DESC                          7300
#define IDC_EDIT_INITIALANGLE                    7301
#define IDC_BUTTON_XDN                           7302
#define IDC_BUTTON_XUP                           7303
#define IDC_BUTTON_YUP                           7304
#define IDC_BUTTON_YDN                           7305
#define IDC_BUTTON_ZUP                           7306
#define IDC_BUTTON_ZDN                           7307


// Next default values for new objects
// 
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_3D_CONTROLS                1
#define _APS_NEXT_RESOURCE_VALUE        7001
#define _APS_NEXT_COMMAND_VALUE         27000
#define _APS_NEXT_CONTROL_VALUE         7308
#define _APS_NEXT_SYMED_VALUE           7001
#endif
#endif
