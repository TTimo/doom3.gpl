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

#include "../../sys/win32/rc/guied_resource.h"

#include "GEApp.h"
#include "GEProperties.h"
#include "GEWindowWrapper.h"
#include "GEStateModifier.h"

/*
================
rvGEProperties::rvGEProperties

constructor
================
*/
rvGEProperties::rvGEProperties( void )
{
	mWrapper   = NULL;
	mWnd	   = NULL;
	mWorkspace = NULL;
}

/*
================
rvGEProperties::Create

Create the property window as a child of the given window
================
*/
bool rvGEProperties::Create ( HWND parent, bool visible )
{
	WNDCLASSEX wndClass;
	memset ( &wndClass, 0, sizeof(wndClass) );
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpszClassName = "GUIEDITOR_PROPERTIES_CLASS";
	wndClass.lpfnWndProc   = WndProc;
	wndClass.hbrBackground = (HBRUSH)GetSysColorBrush ( COLOR_3DFACE ); 
	wndClass.hCursor       = LoadCursor((HINSTANCE) NULL, IDC_ARROW); 
	wndClass.lpszMenuName  = NULL;
	wndClass.hInstance     = win32.hInstance; 
	RegisterClassEx ( &wndClass );

	mWnd = CreateWindowEx ( WS_EX_TOOLWINDOW, 
							"GUIEDITOR_PROPERTIES_CLASS", 
							"Properties", 
							WS_SYSMENU|WS_THICKFRAME|WS_CAPTION|WS_POPUP|WS_OVERLAPPED|WS_BORDER|WS_CLIPSIBLINGS|WS_CHILD, 
							0, 0, 200,300,
							parent, 
							NULL, 
							win32.hInstance, 
							this );
							
	if ( !mWnd )
	{
		return false;
	}

	if ( !gApp.GetOptions().GetWindowPlacement ( "properties", mWnd ) )
	{
		RECT rParent;
		RECT rClient;
		
		GetWindowRect ( parent, &rParent );
		GetWindowRect ( mWnd, &rClient );
		SetWindowPos ( mWnd, NULL,
					rParent.right - 10 - (rClient.right-rClient.left),
					rParent.bottom - 10 - (rClient.bottom-rClient.top),
					0,0,
					SWP_NOZORDER|SWP_NOSIZE );
	}
				 	
	Show ( visible );
	
	return true;	
}

/*
================
rvGEProperties::Show

Show/Hide the properties window
================
*/
void rvGEProperties::Show ( bool visible )
{
	gApp.GetOptions().SetPropertiesVisible ( visible );
	ShowWindow ( mWnd, visible?SW_SHOW:SW_HIDE );
}

/*
================
rvGEProperties::Update

Update the properties in the window
================
*/
void rvGEProperties::Update ( void )
{
	int i;

	if ( mWorkspace && mWorkspace->GetSelectionMgr ( ).Num ( ) == 1 )
	{
		mWrapper = rvGEWindowWrapper::GetWrapper ( mWorkspace->GetSelectionMgr()[0] );	
	}
	else
	{
		mWrapper = NULL;
	}

	ShowWindow ( mGrid.GetWindow ( ), mWrapper?SW_SHOW:SW_HIDE );
		
	mGrid.RemoveAllItems ( );

	if ( mWrapper )
	{
		for ( i = 0; i < (int)mWrapper->GetStateDict().GetNumKeyVals ( ); i ++ )
		{
			const idKeyValue* kv = mWrapper->GetStateDict().GetKeyVal ( i );
			idStr temp;
			temp = kv->GetValue();
			temp.StripQuotes ( );
			mGrid.AddItem ( kv->GetKey(), temp );
		}
	}	
}

/*
================
rvGEProperties::AddModifier

Add a state modifier for the given key / value pair
================
*/
bool rvGEProperties::AddModifier ( const char* name, const char* value )
{
	idDict tempstate;	
	idStr  tempvalue;
	
	tempvalue = value;
	if ( !mWrapper->VerfiyStateKey ( name, tempvalue ) )
	{
		tempvalue = "\"";
		tempvalue += value;
		tempvalue += "\"";
		if ( !mWrapper->VerfiyStateKey ( name, tempvalue ) )
		{
			gApp.MessageBox ( va("Invalid property value '%s' for property '%s'", value, name), MB_OK );
			return false;
		}
	}
	
	tempstate = mWrapper->GetStateDict ( );
	
	tempstate.Set ( name, tempvalue );
	
	mWorkspace->GetModifierStack().Append ( new rvGEStateModifier ( "Property Change", mWrapper->GetWindow(), tempstate ) );		
	mWorkspace->SetModified ( true );
	gApp.GetNavigator().Update ( );	
	
	return true;	
}

/*
================
rvGEProperties::WndProc

Window Procedure for the properties window
================
*/
LRESULT CALLBACK rvGEProperties::WndProc ( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	rvGEProperties* kv = (rvGEProperties*) GetWindowLong ( hWnd, GWL_USERDATA );

	if ( kv && kv->mGrid.ReflectMessage ( hWnd, msg, wParam, lParam ) )
	{
		return 0;
	}

	switch ( msg )
	{				
		case WM_ACTIVATE:
			common->ActivateTool( LOWORD( wParam ) != WA_INACTIVE );
			break;

		case WM_NOTIFY:
		{
			NMHDR* hdr;
			hdr = (NMHDR*)lParam;
			if ( hdr->idFrom == 999 )
			{
				NMPROPGRID* nmpg = (NMPROPGRID*)hdr;
				switch ( hdr->code )
				{
					case PGN_ITEMCHANGED:
						return (int)kv->AddModifier ( nmpg->mName, nmpg->mValue );
/*						
					case NM_KEYDOWN:
					{
						NMKEY* nmkey = (NMKEY*)hdr;
						if ( nmkey->nVKey == VK_DELETE )
						{
							int sel = kv->mGrid.GetCurSel ( );
							if ( sel != -1 )
							{
								const char* prop;
								
								prop = kv->mGrid.GetItemName(sel);
								if ( !idStr::Icmp ( prop, "rect" )		|| 
									 !idStr::Icmp ( prop, "visible" )	|| 
									 !idStr::Icmp ( prop, "name" ) )
								{
									MessageBeep ( MB_ICONASTERISK );
								}
								else
								{
									idDict tempstate;
									tempstate = kv->mWrapper->GetStateDict ( );
									tempstate.Delete ( prop );
									kv->mWorkspace->GetModifierStack().Append ( new rvGEStateModifier ( "Property Change", kv->mWrapper->GetWindow(), tempstate ) );
									kv->mWorkspace->SetModified ( true );
									kv->mGrid.RemoveItem ( sel );
								}
							}
						}
						else
						{
							SendMessage ( gApp.GetMDIFrame(), WM_KEYDOWN, nmkey->nVKey, nmkey->uFlags );
						}
						break;
					}
*/
				}
			}
			break;
		}
	
		case WM_CREATE:
		{
			LPCREATESTRUCT	cs;

			// Attach the class to the window first
			cs = (LPCREATESTRUCT) lParam;
			kv = (rvGEProperties*) cs->lpCreateParams;
			SetWindowLong ( hWnd, GWL_USERDATA, (LONG)kv );

			kv->mGrid.Create ( hWnd, 999, PGS_ALLOWINSERT );					
			
			kv->SetWorkspace ( NULL );
			kv->Update ( );
			
			break;
		}

		case WM_ERASEBKGND:
			if ( kv->mWrapper )
			{
				return FALSE;
			}
			break;

		case WM_SIZE:
			kv->mGrid.Move ( 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE );
			break;

		case WM_CLOSE:		
			gApp.GetOptions().SetPropertiesVisible ( false );
			kv->Show ( false );
			return 0;
			
		case WM_NCACTIVATE:
			return gApp.ToolWindowActivate ( hWnd, msg, wParam, lParam );

		case WM_DESTROY:
			gApp.GetOptions().SetWindowPlacement ( "properties", hWnd );
			break;
	}
		
	return DefWindowProc ( hWnd, msg, wParam, lParam );
}


