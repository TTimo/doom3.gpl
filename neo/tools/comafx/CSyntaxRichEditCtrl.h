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

#ifndef __CSYNTAXRICHEDITCTR_H__
#define __CSYNTAXRICHEDITCTR_H__

/*
===============================================================================

	Rich Edit Control with:

	- syntax highlighting
	- braced section highlighting
	- braced section auto-indentation
	- multi-line tabs
	- keyword auto-completion
	- object member auto-completion
	- keyword tool tip
	- function parameter tool tip

===============================================================================
*/

// use #import on Vista to generate .tlh header to copy from intermediate compile directory to local directory for subsequent builds
//     rename: avoids warning C4278: 'FindText': identifier in type library 'riched20.dll' is already a macro; use the 'rename' qualifier
//     no_auto_exclude: avoids warnings
//     no_namespace: no longer using this option, which avoids variable redifinition compile errors on Vista
//#define GENERATE_TLH
#ifdef GENERATE_TLH
#	import "riched20.dll" raw_interfaces_only, raw_native_types, named_guids, no_auto_exclude, no_implementation, rename( "FindText", "FindShit" ) 
#else
#	include "riched20.tlh"
#endif
 
static const char *		FONT_NAME				= "Courier";
static const int		FONT_HEIGHT				= 10;
static const int		FONT_WIDTH				= 8;
static const int		TAB_SIZE				= 4;

static const COLORREF	SRE_COLOR_BLACK			= RGB(   0,   0,   0 );
static const COLORREF	SRE_COLOR_WHITE			= RGB( 255, 255, 255 );
static const COLORREF	SRE_COLOR_RED			= RGB( 255,   0,   0 );
static const COLORREF	SRE_COLOR_GREEN			= RGB(   0, 255,   0 );
static const COLORREF	SRE_COLOR_BLUE			= RGB(   0,   0, 255 );
static const COLORREF	SRE_COLOR_YELLOW		= RGB( 255, 255,   0 );
static const COLORREF	SRE_COLOR_MAGENTA		= RGB( 255,   0, 255 );
static const COLORREF	SRE_COLOR_CYAN			= RGB(   0, 255, 255 );
static const COLORREF	SRE_COLOR_ORANGE		= RGB( 255, 128,   0 );
static const COLORREF	SRE_COLOR_PURPLE		= RGB( 150,   0, 150 );
static const COLORREF	SRE_COLOR_PINK			= RGB( 186, 102, 123 );
static const COLORREF	SRE_COLOR_GREY			= RGB(  85,  85,  85 );
static const COLORREF	SRE_COLOR_BROWN			= RGB( 100,  90,  20 );
static const COLORREF	SRE_COLOR_LIGHT_GREY	= RGB( 170, 170, 170 );
static const COLORREF	SRE_COLOR_LIGHT_BROWN	= RGB( 170, 150,  20 );
static const COLORREF	SRE_COLOR_DARK_GREEN	= RGB(   0, 128,   0 );
static const COLORREF	SRE_COLOR_DARK_CYAN		= RGB(   0, 150, 150 );
static const COLORREF	SRE_COLOR_DARK_YELLOW	= RGB( 220, 200,  20 );

typedef struct {
	const char *		keyWord;
	COLORREF			color;
	const char *		description;
} keyWord_t;

typedef bool (*objectMemberCallback_t)( const char *objectName, CListBox &listBox );
typedef bool (*toolTipCallback_t)( const char *name, CString &string );


class CSyntaxRichEditCtrl : public CRichEditCtrl {
public:
							CSyntaxRichEditCtrl( void );
							~CSyntaxRichEditCtrl( void );

	void					Init( void );

	void					SetCaseSensitive( bool caseSensitive );
	void					AllowPathNames( bool allow );
	void					EnableKeyWordAutoCompletion( bool enable );
	void					SetKeyWords( const keyWord_t kws[] );
	bool					LoadKeyWordsFromFile( const char *fileName );
	void					SetObjectMemberCallback( objectMemberCallback_t callback );
	void					SetFunctionParmCallback( toolTipCallback_t callback );
	void					SetToolTipCallback( toolTipCallback_t callback );

	void					SetDefaultColor( const COLORREF color );
	void					SetCommentColor( const COLORREF color );
	void					SetStringColor( const COLORREF color, const COLORREF altColor = -1 );
	void					SetLiteralColor( const COLORREF color );

	COLORREF				GetForeColor( int charIndex ) const;
	COLORREF				GetBackColor( int charIndex ) const;

	void					GetCursorPos( int &line, int &column, int &character ) const;
	CHARRANGE				GetVisibleRange( void ) const;

	void					GetText( idStr &text ) const;
	void					GetText( idStr &text, int startCharIndex, int endCharIndex ) const;
	void					SetText( const char *text );

	void					GoToLine( int line );
	bool					FindNext( const char *find, bool matchCase, bool matchWholeWords, bool searchForward );
	int						ReplaceAll( const char *find, const char *replace, bool matchCase, bool matchWholeWords );
	void					ReplaceText( int startCharIndex, int endCharIndex, const char *replace );

protected:
	virtual int				OnToolHitTest( CPoint point, TOOLINFO* pTI ) const;
	afx_msg BOOL			OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg UINT			OnGetDlgCode();
	afx_msg void			OnChar( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg void			OnKeyDown( UINT nKey, UINT nRepCnt, UINT nFlags );
	afx_msg void			OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg BOOL			OnMouseWheel( UINT nFlags, short zDelta, CPoint pt );
	afx_msg void			OnMouseMove( UINT nFlags, CPoint point );
	afx_msg void			OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	afx_msg void			OnSize( UINT nType, int cx, int cy );
	afx_msg void			OnProtected( NMHDR *pNMHDR, LRESULT *pResult );
	afx_msg void			OnChange();
	afx_msg void			OnAutoCompleteListBoxChange();
	afx_msg void			OnAutoCompleteListBoxDblClk();

	DECLARE_MESSAGE_MAP()

	// settings
	CHARFORMAT2				defaultCharFormat;
	COLORREF				defaultColor;
	COLORREF				singleLineCommentColor;
	COLORREF				multiLineCommentColor;
	COLORREF				stringColor[2];
	COLORREF				literalColor;
	COLORREF				braceHighlightColor;
	
	typedef enum {
		CT_WHITESPACE,
		CT_COMMENT,
		CT_STRING,
		CT_LITERAL,
		CT_NUMBER,
		CT_NAME,
		CT_PUNCTUATION
	} charType_t;

	int						charType[256];

	idList<keyWord_t>		keyWordsFromFile;
	const keyWord_t *		keyWords;
	int *					keyWordLengths;
	COLORREF *				keyWordColors;
	idHashIndex				keyWordHash;

	bool					caseSensitive;
	bool					allowPathNames;
	bool					keyWordAutoCompletion;

	objectMemberCallback_t	GetObjectMembers;
	toolTipCallback_t		GetFunctionParms;
	toolTipCallback_t		GetToolTip;

	// run-time variables
	tom::ITextDocument *	m_TextDoc;
	tom::ITextFont *		m_DefaultFont;

	CHARRANGE				updateRange;
	bool					updateSyntaxHighlighting;
	int						stringColorIndex;
	int						stringColorLine;

	int						autoCompleteStart;
	CListBox				autoCompleteListBox;

	int						funcParmToolTipStart;
	CEdit					funcParmToolTip;

	int						bracedSection[2];

	CPoint					mousePoint;
	CToolTipCtrl *			keyWordToolTip;
	TCHAR *					m_pchTip;
	WCHAR *					m_pwchTip;

protected:
	void					InitFont( void );
	void					InitSyntaxHighlighting( void );
	void					SetCharType( int first, int last, int type );
	void					SetDefaultFont( int startCharIndex, int endCharIndex );
	void					SetColor( int startCharIndex, int endCharIndex, COLORREF foreColor, COLORREF backColor, bool bold );

	void					FreeKeyWordsFromFile( void );
	int						FindKeyWord( const char *keyWord, int length ) const;

	void					HighlightSyntax( int startCharIndex, int endCharIndex );
	void					UpdateVisibleRange( void );

	bool					GetNameBeforeCurrentSelection( CString &name, int &charIndex ) const;
	bool					GetNameForMousePosition( idStr &name ) const;

	void					AutoCompleteInsertText( void );
	void					AutoCompleteUpdate( void );
	void					AutoCompleteShow( int charIndex );
	void					AutoCompleteHide( void );

	void					ToolTipShow( int charIndex, const char *string );
	void					ToolTipHide( void );

	bool					BracedSectionStart( char braceStartChar, char braceEndChar );
	bool					BracedSectionEnd( char braceStartChar, char braceEndChar );
	void					BracedSectionAdjustEndTabs( void );
	void					BracedSectionShow( void );
	void					BracedSectionHide( void );
};

#endif /* !__CSYNTAXRICHEDITCTR_H__ */
