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

#include "ConsoleView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define EDIT_HEIGHT 25


IMPLEMENT_DYNCREATE(ConsoleView, CFormView)

BEGIN_MESSAGE_MAP(ConsoleView, CFormView)
	ON_WM_SIZE()
END_MESSAGE_MAP()

/**
* Constructor for ConsoleView.
*/
ConsoleView::ConsoleView()
: CFormView(ConsoleView::IDD) {
}

/**
* Destructor for ConsoleView.
*/
ConsoleView::~ConsoleView() {
}

/**
* Adds text to the end of the console output window.
* @param msg The text to append.
* \todo: BMatt Nerve: Fix scroll code so the output window will scroll as text
* is added if the cursor is at the end of the window.
*/
void ConsoleView::AddText( const char *msg ) {

	if(!editConsole.GetSafeHwnd())
		return;

	idStr work;
	CString work2;

	work = msg;
	work.RemoveColors();
	work = TranslateString( work.c_str() );

	editConsole.GetWindowText( work2 );
	int len = work2.GetLength();
	if ( len + work.Length() > (int)editConsole.GetLimitText() ) {
		work2 = work2.Right( editConsole.GetLimitText() * .75f );
		len = work2.GetLength();
		editConsole.SetWindowText(work2);
	}
	editConsole.SetSel( len, len );
	editConsole.ReplaceSel( work );
	
	//Hack: scrolls down a bit
	editConsole.LineScroll(100);
}

/**
* Replaces the text in the console window with the specified text.
* @param text The text to place in the console window.
*/
void ConsoleView::SetConsoleText ( const idStr& text ) {
	editInput.Clear ();
	editInput.SetWindowText ( text.c_str() );
}

/**
* Executes the specified console command. If the command is passed
* as a parameter then it is executed otherwise the command in the
* input box is executed.
* @param cmd The text to execute. If this string is empty then the
* input edit box text is used.
*/
void ConsoleView::ExecuteCommand ( const idStr& cmd ) {

	CString str;
	if ( cmd.Length() > 0 ) {
		str = cmd;
	}
	else {
		editInput.GetWindowText(str);
	}

	if ( str != "" ) {			
		editInput.SetWindowText("");
		common->Printf("%s\n", str.GetBuffer(0));		

		//avoid adding multiple identical commands in a row
		int index = consoleHistory.Num ();

		if ( index == 0 || str.GetBuffer(0) != consoleHistory[index-1]) {					
			//keep the history to 16 commands, removing the oldest command
			if ( consoleHistory.Num () > 16 ) {
				consoleHistory.RemoveIndex ( 0 );
			}
			currentHistoryPosition = consoleHistory.Append ( str.GetBuffer (0) );    
		}
		else {
			currentHistoryPosition = consoleHistory.Num () - 1;
		}

		currentCommand.Clear ();

		bool propogateCommand = true;

		//process some of our own special commands
		if ( str.CompareNoCase ( "clear" ) == 0) {
			editConsole.SetSel ( 0 , -1 );
			editConsole.Clear ();
		}
		else if ( str.CompareNoCase ( "edit" ) == 0) {
			propogateCommand = false;
		}
		if ( propogateCommand ) {
			cmdSystem->BufferCommandText(CMD_EXEC_NOW, str);
		}
	}
}

/**
* Handles keyboard input to process the "Enter" key to execute 
* commands and command history.
*/
BOOL ConsoleView::PreTranslateMessage(MSG* pMsg) {

	if (pMsg->hwnd == editInput.GetSafeHwnd()) {

		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN ) {
			this->ExecuteCommand();
			return TRUE;
		}

		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_UP ) {     
			//save off the current in-progress command so we can get back to it
			if ( saveCurrentCommand == true ) {
				CString str;
				editInput.GetWindowText ( str );
				currentCommand = str.GetBuffer ( 0 );
				saveCurrentCommand = false;
			}

			if ( consoleHistory.Num () > 0 ) {
				editInput.SetWindowText ( consoleHistory[currentHistoryPosition] );

				int selLocation = consoleHistory[currentHistoryPosition].Length ();
				editInput.SetSel ( selLocation , selLocation + 1);
			}

			if ( currentHistoryPosition > 0) {
				--currentHistoryPosition;
			}

			return TRUE;
		}

		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_DOWN ) {  
			int selLocation = 0;
			if ( currentHistoryPosition < consoleHistory.Num () - 1 ) {
				++currentHistoryPosition;
				editInput.SetWindowText ( consoleHistory[currentHistoryPosition] );
				selLocation = consoleHistory[currentHistoryPosition].Length ();
			}
			else {
				editInput.SetWindowText ( currentCommand );
				selLocation = currentCommand.Length ();
				currentCommand.Clear ();
				saveCurrentCommand = true;
			}

			editInput.SetSel ( selLocation , selLocation + 1);

			return TRUE;
		}
		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB ) {  
			common->Printf ( "Command History\n----------------\n" );
			for ( int i = 0 ; i < consoleHistory.Num ();i++ )
			{
				common->Printf ( "[cmd %d]:  %s\n" , i , consoleHistory[i].c_str() );
			}
			common->Printf ( "----------------\n" );
		}
		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_NEXT) {  
			editConsole.LineScroll ( 10 );	
		}

		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_PRIOR ) {  
			editConsole.LineScroll ( -10 );	
		}

		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_HOME ) {  
			editConsole.LineScroll ( -editConsole.GetLineCount() );	
		}

		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_END ) {  
			editConsole.LineScroll ( editConsole.GetLineCount() );	
		}
	}

	return CFormView::PreTranslateMessage(pMsg);
}

/**
* Transfers data to and from the controls in the console.
*/
void ConsoleView::DoDataExchange(CDataExchange* pDX) {
	CFormView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_CONSOLE_OUTPUT, editConsole);
	DDX_Control(pDX, IDC_CONSOLE_EDIT, editInput);
}

/**
* Transfers data to and from the controls in the console.
*/
void ConsoleView::OnInitialUpdate() {
	
	CFormView::OnInitialUpdate();

	CRect rect;
	GetWindowRect(rect);

	if(editConsole.m_hWnd)
		editConsole.MoveWindow(0, 0, rect.Width(), rect.Height()-EDIT_HEIGHT);

	if(editInput.m_hWnd)
		editInput.MoveWindow(0, rect.Height()-EDIT_HEIGHT, rect.Width(), EDIT_HEIGHT);
}

/**
* Windows message called when the window is resized.
*/
void ConsoleView::OnSize(UINT nType, int cx, int cy) {
	CFormView::OnSize(nType, cx, cy);

	//Move the edit windows around
	if(editConsole.GetSafeHwnd())
		editConsole.MoveWindow(0, 0, cx, cy-EDIT_HEIGHT);

	if(editInput.GetSafeHwnd())
		editInput.MoveWindow(0, cy-EDIT_HEIGHT, cx, EDIT_HEIGHT);
}

/**
* Replaces \\n with \\r\\n for carriage returns in an edit control.
*/
const char *ConsoleView::TranslateString(const char *buf) {
	static char buf2[32768];
	int			i, l;
	char		*out;

	l = strlen(buf);
	out = buf2;
	for (i = 0; i < l; i++) {
		if (buf[i] == '\n') {
			*out++ = '\r';
			*out++ = '\n';
		}
		else {
			*out++ = buf[i];
		}
	}

	*out++ = 0;

	return buf2;
}
