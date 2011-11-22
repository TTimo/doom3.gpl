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

#include "../../sys/win32/win_local.h"

#include "MaterialEditor.h"
#include "MEMainFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

MEMainFrame* meMainFrame = NULL;

CFont* materialEditorFont = NULL;

/**
* Initializes the material editor tool.
*/
void MaterialEditorInit( void ) {

	InitPropTree(win32.hInstance);

	com_editors = EDITOR_MATERIAL;

	Sys_GrabMouseCursor( false );

	InitAfx();

	InitCommonControls();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		return;
	}
	AfxEnableControlContainer();

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);

	LOGFONT lf;
	memset(&lf, 0, sizeof (LOGFONT));

	CWindowDC dc(NULL);
	lf.lfCharSet = (BYTE)GetTextCharsetInfo(dc.GetSafeHdc(), NULL, 0);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	// check if we should use system font
	_tcscpy(lf.lfFaceName, info.lfMenuFont.lfFaceName);

	materialEditorFont = new CFont;
	materialEditorFont->CreateFontIndirect(&lf);


	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object
	meMainFrame = new MEMainFrame;
	
	// create and load the frame with its resources
	meMainFrame->LoadFrame(IDR_ME_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL);


	// hide the doom window by default
	::ShowWindow ( win32.hWnd, SW_HIDE );
	
	// The one and only window has been initialized, so show and update it
	meMainFrame->ShowWindow(SW_SHOW);
	meMainFrame->UpdateWindow();
}

/**
* Called every frame by the doom engine to allow the material editor to process messages.
*/
void MaterialEditorRun( void ) {

	MSG *msg = AfxGetCurrentMessage();
	
	while( ::PeekMessage(msg, NULL, NULL, NULL, PM_NOREMOVE) ) {
		// pump message
		if ( !AfxGetApp()->PumpMessage() ) {
		}
	}
}

/**
* Called by the doom engine when the material editor needs to be destroyed.
*/
void MaterialEditorShutdown( void ) {
	
	delete meMainFrame;

	delete materialEditorFont;

	meMainFrame = NULL;
}
 
/**
* Allows the doom engine to reflect console output to the material editors console.
*/
void MaterialEditorPrintConsole( const char *msg ) {
	if(com_editors & EDITOR_MATERIAL)
		meMainFrame->PrintConsoleMessage(msg);
}

/**
* Returns the handle to the main Material Editor Window
*/
HWND GetMaterialEditorWindow() {
	return meMainFrame->GetSafeHwnd();
}

/**
* Simple about box for the material editor.
*/
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	enum { IDD = IDD_ME_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

/**
* Constructor for the about box.
*/
CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD) {
}

/**
* Called by the MFC framework to exchange data with the window controls.
*/
void CAboutDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

