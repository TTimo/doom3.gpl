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

#define IDD_GUIED_ABOUT                          5000
#define IDD_GUIED_OPTIONS_GRID                   5001
#define IDD_GUIED_OPTIONS_GENERAL                5002
#define IDD_GUIED_ITEMPROPS_GENERAL              5003
#define IDD_GUIED_ITEMPROPS_ADVANCED             5004
#define IDD_GUIED_ITEMKEY                        5005
#define IDD_GUIED_ALPHA                          5006
#define IDD_GUIED_SCRIPTS                        5007
#define IDD_GUIED_ITEMPROPS_KEYS                 5008
#define IDD_GUIED_ITEM_VARIABLES                 5009
#define IDD_GUIED_ITEMPROPS_TEXT                 5010
#define IDD_GUIED_ITEMPROPS_IMAGE                5011
#define IDD_GUIED_TRANSFORMER                    5012
#define IDD_GUIED_CHECKIN                        5013
#define IDR_GUIED_MAIN                           5014
#define IDR_GUIED_ACCELERATORS                   5015
#define IDR_GUIED_ITEM_POPUP                     5016
#define IDR_GUIED_VIEWERTOOLBAR                  5017
#define IDI_GUIED                                5018
#define IDI_GUIED_NAV_VISIBLE                    5019
#define IDI_GUIED_NAV_SCRIPTS                    5020
#define IDI_GUIED_NAV_VISIBLEDISABLED            5021
#define IDI_GUIED_NAV_SCRIPTSHI                  5022
#define IDI_GUIED_NAV_COLLAPSE                   5023
#define IDI_GUIED_NAV_EXPAND                     5024

#define IDC_GUIED_OPTIONSTAB                     5200
#define IDC_GUIED_GRIDCOLOR                      5201
#define IDC_GUIED_SPACINGWIDTH                   5202
#define IDC_GUIED_SPACINGHEIGHT                  5203
#define IDC_GUIED_GRIDVISIBLE                    5204
#define IDC_GUIED_GRIDSNAP                       5205
#define IDC_GUIED_WINDOWTREE                     5206
#define IDC_GUIED_ITEMNAME                       5207
#define IDC_GUIED_ITEMBACKGROUND                 5208
#define IDC_GUIED_TYPE                           5209
#define IDC_GUIED_USEMATERIAL                    5210
#define IDC_GUIED_ITEMBORDERMATERIAL             5211
#define IDC_GUIED_ITEMBACKCOLOR                  5212
#define IDC_GUIED_ITEMMATCOLOR                   5213
#define IDC_GUIED_ITEMFORECOLOR                  5214
#define IDC_GUIED_ITEMBORDERCOLOR                5215
#define IDC_GUIED_ITEMBACKCOLORALPHA             5216
#define IDC_GUIED_ITEMTEXT                       5217
#define IDC_GUIED_ITEMBORDERCOLORALPHA           5218
#define IDC_GUIED_ITEMTEXTSCALE                  5219
#define IDC_GUIED_ITEMBORDERSIZE                 5220
#define IDC_GUIED_ITEMMATCOLORALPHA              5221
#define IDC_GUIED_ITEMTEXTALIGNX                 5222
#define IDC_GUIED_ITEMTEXTALIGNY                 5223
#define IDC_GUIED_ITEMMATSCALEX                  5224
#define IDC_GUIED_ITEMMATSCALEY                  5225
#define IDC_GUIED_USEBACKCOLOR                   5226
#define IDC_GUIED_ITEMMATCOLORSTATIC             5227
#define IDC_GUIED_ITEMKEYS                       5228
#define IDC_GUIED_ADDKEY                         5229
#define IDC_GUIED_EDITKEY                        5230
#define IDC_GUIED_DELETEKEY                      5231
#define IDC_GUIED_ITEMKEY                        5232
#define IDC_GUIED_ITEMVALUE                      5233
#define IDC_GUIED_ITEMVISIBLE                    5234
#define IDC_GUIED_SELECTIONCOLOR                 5235
#define IDC_GUIED_ALPHA                          5236
#define IDC_GUIED_ALPHASLIDER                    5237
#define IDC_GUIED_SCRIPT                         5238
#define IDC_GUIED_ADDVARIABLE                    5239
#define IDC_GUIED_EDITVARIABLE                   5240
#define IDC_GUIED_DELETEVARIABLE                 5241
#define IDC_GUIED_ITEMVARIABLES                  5242
#define IDC_GUIED_USETEXT                        5243
#define IDC_GUIED_ITEMTEXTALIGN                  5244
#define IDC_GUIED_STATIC_ALIGNMENT               5245
#define IDC_GUIED_STATIC_X                       5246
#define IDC_GUIED_STATIC_Y                       5247
#define IDC_GUIED_ITEMFORECOLORALPHA             5248
#define IDC_GUIED_STATIC_SCALE                   5249
#define IDC_GUIED_STATIC_COLOR                   5250
#define IDC_GUIED_USEBORDERCOLOR                 5251
#define IDC_GUIED_STATIC_FONT                    5252
#define IDC_GUIED_USEBORDERMATERIAL              5253
#define IDC_GUIED_ITEMTEXTFONT                   5254
#define IDC_GUIED_STATIC_FONTSIZE                5255
#define IDC_GUIED_ITEMVARIABLEBACKGROUND         5256
#define IDC_GUIED_ITEMTEXTFONTSIZE               5257
#define IDC_GUIED_XSCALE_STATIC                  5258
#define IDC_GUIED_YSCALE_STATIC                  5259
#define IDC_GUIED_ITEMNOEVENTS                   5260
#define IDC_GUIED_ITEMNOCLIP                     5261
#define IDC_GUIED_ITEMNOCLIP2                    5262
#define IDC_GUIED_ITEMNOCURSOR                   5263
#define IDC_GUIED_ITEMNOTIME                     5264
#define IDC_GUIED_ITEMRECTX                      5265
#define IDC_GUIED_ITEMRECTW                      5266
#define IDC_GUIED_IGNOREDESKTOP                  5267
#define IDC_GUIED_ITEMRECTH                      5268
#define IDC_GUIED_ITEMTEXTNOWRAP                 5269
#define IDC_GUIED_ITEMRECTY                      5270
#define IDC_GUIED_FILENAME                       5271
#define IDC_GUIED_COMMENT                        5272
#define IDC_GUIED_ITEMTEXTSCALE_SPIN             5273
#define IDC_GUIED_HAND                           5274

#define ID_GUIED_NEW_HTMLDEF                     25000
#define ID_GUIED_ITEM_NEWSLIDERDEF               25001
#define ID_GUIED_FILE_EXIT                       25002
#define ID_GUIED_HELP_ABOUT                      25003
#define ID_GUIED_WINDOW_CLOSEALL                 25004
#define ID_GUIED_FILE_NEW                        25005
#define ID_GUIED_FILE_OPEN                       25006
#define ID_GUIED_FILE_CLOSE                      25007
#define ID_GUIED_VIEW_ZOOMIN                     25008
#define ID_GUIED_VIEW_ZOOMOUT                    25009
#define ID_GUIED_FILE_SAVE                       25010
#define ID_GUIED_FILE_SAVEAS                     25011
#define ID_GUIED_VIEW_SHOWGRID                   25012
#define ID_GUIED_VIEW_SNAPTOGRID                 25013
#define ID_GUIED_VIEW_GRIDSETTINGS               25014
#define ID_GUIED_EDIT_CUT                        25015
#define ID_GUIED_EDIT_COPY                       25016
#define ID_GUIED_EDIT_PASTE                      25017
#define ID_GUIED_VIEW_OPTIONS                    25018
#define ID_GUIED_EDIT_REDO                       25019
#define ID_GUIED_VIEW_HIDESELECTED               25020
#define ID_GUIED_VIEW_SHOWHIDDEN                 25021
#define ID_GUIED_EDIT_UNDO                       25022
#define ID_GUIED_EDIT_DELETE                     25023
#define ID_GUIED_WINDOW_TILE                     25024
#define ID_GUIED_WINDOW_SHOWNAVIGATOR            25025
#define ID_GUIED_FILE_MRU                        25026
#define ID_GUIED_ITEM_NEWEDITDEF                 25027
#define ID_GUIED_ITEM_NEWSCRIPTDEF               25028
#define ID_GUIED_ITEM_PROPERTIES                 25029
#define ID_GUIED_ITEM_ARRANGEBRINGFORWARD        25030
#define ID_GUIED_ITEM_ARRANGESENDTOBACK          25031
#define ID_GUIED_ITEM_ARRANGESENDBACKWARD        25032
#define ID_GUIED_ITEM_ARRANGEBRINGTOFRONT        25033
#define ID_GUIED_ITEM_ALIGNLEFTS                 25034
#define ID_GUIED_ITEM_ALIGNCENTERS               25035
#define ID_GUIED_ITEM_ALIGNRIGHTS                25036
#define ID_GUIED_ITEM_ALIGNTOPS                  25037
#define ID_GUIED_ITEM_ALIGNMIDDLES               25038
#define ID_GUIED_ITEM_ALIGNBOTTOMS               25039
#define ID_GUIED_ITEM_MAKESAMESIZEWIDTH          25040
#define ID_GUIED_ITEM_MAKESAMESIZEHEIGHT         25041
#define ID_GUIED_ITEM_MAKESAMESIZEBOTH           25042
#define ID_GUIED_ITEM_SCRIPTS                    25043
#define ID_GUIED_ITEM_ARRANGEMAKECHILD           25044
#define ID_GUIED_TOOLS_VIEWER                    25045
#define ID_GUIED_WINDOW_CASCADE                  25046
#define ID_GUIED_ITEM_NEWWINDOWDEF               25047
#define ID_GUIED_VIEW_UNHIDESELECTED             25048
#define ID_GUIED_WINDOW_SHOWTRANSFORMER          25049
#define ID_GUIED_TOOLS_RELOADMATERIALS           25050
#define ID_GUIED_SOURCECONTROL_CHECKIN           25051
#define ID_GUIED_SOURCECONTROL_CHECKOUT          25052
#define ID_GUIED_SOURCECONTROL_UNDOCHECKOUT      25053
#define ID_GUIED_SOURCECONTROL_GETLATESTVERSION  25054
#define ID_GUIED_ITEM_NEWHTMLDEF                 25055
#define ID_GUIED_VIEW_STATUSBAR                  25056
#define ID_GUIED_ITEM_NEWCHOICEDEF               25057
#define ID_GUIED_ITEM_NEWBINDDEF                 25058
#define ID_GUIED_ITEM_NEWLISTDEF                 25059
#define ID_GUIED_WINDOW_SHOWPROPERTIES           25060
#define ID_GUIED_ITEM_NEWRENDERDEF               25061
#define ID_GUIED_VIEWER_PAUSE                    25062
#define ID_GUIED_VIEWER_PLAY                     25063
#define ID_GUIED_VIEWER_START                    25064

// Next default values for new objects
// 
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_3D_CONTROLS                1
#define _APS_NEXT_RESOURCE_VALUE        5025
#define _APS_NEXT_COMMAND_VALUE         25065
#define _APS_NEXT_CONTROL_VALUE         5275
#define _APS_NEXT_SYMED_VALUE           5025
#endif
#endif
