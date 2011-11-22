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

#include "CSyntaxRichEditCtrl.h"

#ifdef ID_DEBUG_MEMORY
#undef new
#undef DEBUG_NEW
#define DEBUG_NEW new
#endif

// NOTE:	known bug, if you directly jump to a not yet highligted page with the first line starting
//			inside a multi-line comment then the multi-line comment is not picked up and highlighted

const int AUTOCOMPLETE_WIDTH			= 200;
const int AUTOCOMPLETE_HEIGHT			= 180;
const int AUTOCOMPLETE_OFFSET			= 16;

const int FUNCPARMTOOLTIP_WIDTH			= 16;
const int FUNCPARMTOOLTIP_HEIGHT		= 20;
const int FUNCPARMTOOLTIP_OFFSET		= 16;

const COLORREF DEFAULT_BACK_COLOR				= SRE_COLOR_WHITE;
const COLORREF INVALID_BACK_COLOR				= SRE_COLOR_WHITE - 2;
const COLORREF MULTILINE_COMMENT_BACK_COLOR		= SRE_COLOR_WHITE - 1;

#define IDC_LISTBOX_AUTOCOMPLETE		700
#define IDC_EDITBOX_FUNCPARMS			701

static keyWord_t defaultKeyWords[] = {
	{ NULL, SRE_COLOR_BLACK, "" }
};

BEGIN_MESSAGE_MAP(CSyntaxRichEditCtrl, CRichEditCtrl)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
	ON_WM_GETDLGCODE()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_NOTIFY_REFLECT(EN_PROTECTED, OnProtected)
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
	ON_LBN_SELCANCEL(IDC_LISTBOX_AUTOCOMPLETE, OnAutoCompleteListBoxChange)
	ON_LBN_SELCHANGE(IDC_LISTBOX_AUTOCOMPLETE, OnAutoCompleteListBoxChange)
	ON_LBN_DBLCLK(IDC_LISTBOX_AUTOCOMPLETE, OnAutoCompleteListBoxDblClk)
END_MESSAGE_MAP()


/*
================
CSyntaxRichEditCtrl::CSyntaxRichEditCtrl
================
*/
CSyntaxRichEditCtrl::CSyntaxRichEditCtrl( void ) {
	m_TextDoc = NULL;
	keyWords = defaultKeyWords;
	keyWordColors = NULL;
	keyWordLengths = NULL;
	caseSensitive = false;
	allowPathNames = true;
	keyWordAutoCompletion = true;
	updateRange.cpMin = 0;
	updateRange.cpMax = 0;
	updateSyntaxHighlighting = true;
	stringColorIndex = 0;
	stringColorLine = -1;
	autoCompleteStart = -1;
	funcParmToolTipStart = -1;
	bracedSection[0] = -1;
	bracedSection[1] = -1;
	GetObjectMembers = NULL;
	GetFunctionParms = NULL;
	GetToolTip = NULL;
	mousePoint.x = 0;
	mousePoint.y = 0;
	keyWordToolTip = NULL;
	m_pchTip = NULL;
	m_pwchTip = NULL;
}

/*
================
CSyntaxRichEditCtrl::~CSyntaxRichEditCtrl
================
*/
CSyntaxRichEditCtrl::~CSyntaxRichEditCtrl( void ) {
	FreeKeyWordsFromFile();
	delete m_pchTip;
	delete m_pwchTip;
	m_DefaultFont->Release();
}

/*
================
CSyntaxRichEditCtrl::InitFont
================
*/
void CSyntaxRichEditCtrl::InitFont( void ) {
	LOGFONT lf;
	CFont font;
	PARAFORMAT pf;
	int logx, tabSize;

	// set the font
	memset( &lf, 0, sizeof( lf ) );
	lf.lfHeight = FONT_HEIGHT * 10;
	lf.lfWidth = FONT_WIDTH * 10;
	lf.lfCharSet = ANSI_CHARSET;
	lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
	strcpy( lf.lfFaceName, FONT_NAME );
	font.CreatePointFontIndirect( &lf );

	SetFont( &font );

	// get the tab size in twips
	logx = ::GetDeviceCaps( GetDC()->GetSafeHdc(), LOGPIXELSX );
	tabSize = TAB_SIZE * FONT_WIDTH * 1440 / logx;

	// set the tabs
	memset( &pf, 0, sizeof( PARAFORMAT ) );
	pf.cbSize = sizeof( PARAFORMAT );
	pf.dwMask = PFM_TABSTOPS;
	for ( pf.cTabCount = 0; pf.cTabCount < MAX_TAB_STOPS; pf.cTabCount++ ) {
		pf.rgxTabs[pf.cTabCount] = pf.cTabCount * tabSize;
	}

	SetParaFormat( pf );

	memset( &defaultCharFormat, 0, sizeof( defaultCharFormat ) );
	defaultCharFormat.dwMask = CFM_CHARSET | CFM_FACE | CFM_SIZE | CFM_BOLD | CFM_COLOR | CFM_PROTECTED | CFM_BACKCOLOR;
	defaultCharFormat.yHeight = FONT_HEIGHT * 20;
	defaultCharFormat.bCharSet = ANSI_CHARSET;
	defaultCharFormat.bPitchAndFamily = FIXED_PITCH | FF_MODERN;
	defaultCharFormat.crTextColor = SRE_COLOR_BLACK;
	defaultCharFormat.crBackColor = DEFAULT_BACK_COLOR;
	defaultCharFormat.dwEffects = CFE_PROTECTED;
	strcpy( defaultCharFormat.szFaceName, FONT_NAME );
	defaultCharFormat.cbSize = sizeof( defaultCharFormat );

	SetDefaultCharFormat( defaultCharFormat );

	defaultColor = SRE_COLOR_BLACK;
	singleLineCommentColor = SRE_COLOR_DARK_GREEN;
	multiLineCommentColor = SRE_COLOR_DARK_GREEN;
	stringColor[0] = stringColor[1] = SRE_COLOR_DARK_CYAN;
	literalColor = SRE_COLOR_GREY;
	braceHighlightColor = SRE_COLOR_RED;

	// get the default tom::ITextFont
	tom::ITextRange *irange;
	tom::ITextFont *ifont;

	m_TextDoc->Range( 0, 0, &irange );
	irange->get_Font( &ifont );

	ifont->get_Duplicate( &m_DefaultFont );

	ifont->Release();
	irange->Release();
}

/*
================
CSyntaxRichEditCtrl::SetCharType
================
*/
void CSyntaxRichEditCtrl::SetCharType( int first, int last, int type ) {
	for ( int i = first; i <= last; i++ ) {
		charType[i] = type;
	}
}

/*
================
CSyntaxRichEditCtrl::InitSyntaxHighlighting
================
*/
void CSyntaxRichEditCtrl::InitSyntaxHighlighting( void ) {
	SetCharType( 0x00, 0xFF, CT_PUNCTUATION );
	SetCharType( '\0', ' ', CT_WHITESPACE );
	SetCharType( '/', '/', CT_COMMENT );
	SetCharType( '\"', '\"', CT_STRING );
	SetCharType( '\'', '\'', CT_LITERAL );
	SetCharType( 'a', 'z', CT_NAME );
	SetCharType( 'A', 'Z', CT_NAME );
	SetCharType( '_', '_', CT_NAME );
	SetCharType( '#', '#', CT_NAME );
	SetCharType( '0', '9', CT_NUMBER );
}

/*
================
CSyntaxRichEditCtrl::Init
================
*/
void CSyntaxRichEditCtrl::Init( void ) {

	// get the Rich Edit ITextDocument to use the wonky TOM interface
	IRichEditOle *ire = GetIRichEditOle();
	IUnknown *iu = (IUnknown *)ire;
	if ( iu == NULL || iu->QueryInterface( tom::IID_ITextDocument, (void**) &m_TextDoc ) != S_OK ) {
		m_TextDoc = NULL;
	}

	InitFont();

	InitSyntaxHighlighting();

	SetEventMask( GetEventMask() | ENM_CHANGE | ENM_KEYEVENTS | ENM_MOUSEEVENTS | ENM_PROTECTED );	// ENM_SCROLLEVENTS

	EnableToolTips( TRUE );

	// create auto complete list box
	CRect rect( 0, 0, AUTOCOMPLETE_WIDTH, AUTOCOMPLETE_HEIGHT );
	autoCompleteListBox.Create( WS_DLGFRAME | WS_VISIBLE | WS_VSCROLL | LBS_SORT | LBS_NOTIFY, rect, this, IDC_LISTBOX_AUTOCOMPLETE );
	autoCompleteListBox.SetFont( GetParent()->GetFont() );
	autoCompleteListBox.ShowWindow( FALSE );

	// create function parameter tool tip
	funcParmToolTip.Create( WS_VISIBLE | WS_BORDER, rect, this, IDC_EDITBOX_FUNCPARMS );
	funcParmToolTip.SetFont( GetParent()->GetFont() );
	funcParmToolTip.ShowWindow( FALSE );
}

/*
================
CSyntaxRichEditCtrl::FindKeyWord
================
*/
ID_INLINE int CSyntaxRichEditCtrl::FindKeyWord( const char *keyWord, int length ) const {
	int i, hash;

	if ( caseSensitive ) {
		hash = idStr::Hash( keyWord, length );
	} else {
		hash = idStr::IHash( keyWord, length );
	}
	for ( i = keyWordHash.First( hash ); i != -1; i = keyWordHash.Next( i ) ) {
		if ( length != keyWordLengths[i] ) {
			continue;
		}
		if ( caseSensitive ) {
			if ( idStr::Cmpn( keyWords[i].keyWord, keyWord, length ) != 0 ) {
				continue;
			}
		} else {
			if ( idStr::Icmpn( keyWords[i].keyWord, keyWord, length ) != 0 ) {
				continue;
			}
		}
		return i;
	}
	return -1;
}

/*
================
CSyntaxRichEditCtrl::SetKeyWords
================
*/
void CSyntaxRichEditCtrl::SetKeyWords( const keyWord_t kws[] ) {
	int i, numKeyWords, hash;

	keyWords = kws;

	for ( numKeyWords = 0; keyWords[numKeyWords].keyWord; numKeyWords++ ) {
	}

	delete keyWordColors;
	keyWordColors = new COLORREF[numKeyWords];

	for ( i = 0; i < numKeyWords; i++ ) {
		keyWordColors[i] = keyWords[i].color;
	}

	delete keyWordLengths;
	keyWordLengths = new int[numKeyWords];

	for ( i = 0; i < numKeyWords; i++ ) {
		keyWordLengths[i] = idStr::Length( keyWords[i].keyWord );
	}

	keyWordHash.Clear( 1024, 1024 );
	for ( i = 0; i < numKeyWords; i++ ) {
		if ( caseSensitive ) {
			hash = idStr::Hash( keyWords[i].keyWord, keyWordLengths[i] );
		} else {
			hash = idStr::IHash( keyWords[i].keyWord, keyWordLengths[i] );
		}
		keyWordHash.Add( hash, i );
	}
}

/*
================
CSyntaxRichEditCtrl::LoadKeyWordsFromFile
================
*/
bool CSyntaxRichEditCtrl::LoadKeyWordsFromFile( const char *fileName ) {
	idParser src;
	idToken token, name, description;
	byte red, green, blue;
	keyWord_t keyword;

	if ( !src.LoadFile( fileName ) ) {
		return false;
	}

	FreeKeyWordsFromFile();

	while( src.ReadToken( &token ) ) {
		if ( token.Icmp( "keywords" ) == 0 ) {
			src.ExpectTokenString( "{" );
			while( src.ReadToken( &token ) ) {
				if ( token == "}" ) {
					break;
				}
				if ( token == "{" ) {

					// parse name
					src.ExpectTokenType( TT_STRING, 0, &name );
					src.ExpectTokenString( "," );

					// parse color
					src.ExpectTokenString( "(" );
					src.ExpectTokenType( TT_NUMBER, TT_INTEGER, &token );
					red = token.GetIntValue();
					src.ExpectTokenString( "," );
					src.ExpectTokenType( TT_NUMBER, TT_INTEGER, &token );
					green = token.GetIntValue();
					src.ExpectTokenString( "," );
					src.ExpectTokenType( TT_NUMBER, TT_INTEGER, &token );
					blue = token.GetIntValue();
					src.ExpectTokenString( ")" );
					src.ExpectTokenString( "," );

					// parse description
					src.ExpectTokenType( TT_STRING, 0, &description );
					src.ExpectTokenString( "}" );

					keyword.keyWord = Mem_CopyString( name );
					keyword.color = RGB( red, green, blue );
					keyword.description = Mem_CopyString( description );

					keyWordsFromFile.Append( keyword );
				}
			}
		} else {
			src.SkipBracedSection();
		}
	}

	keyword.keyWord = NULL;
	keyword.color = RGB( 255, 255, 255 );
	keyword.description = NULL;
	keyWordsFromFile.Append( keyword );

	SetKeyWords( keyWordsFromFile.Ptr() );

	return true;
}

/*
================
CSyntaxRichEditCtrl::FreeKeyWordsFromFile
================
*/
void CSyntaxRichEditCtrl::FreeKeyWordsFromFile( void ) {
	for ( int i = 0; i < keyWordsFromFile.Num(); i++ ) {
		Mem_Free( const_cast<char *>( keyWordsFromFile[i].keyWord ) );
		Mem_Free( const_cast<char *>( keyWordsFromFile[i].description ) );
	}
	keyWordsFromFile.Clear();
}

/*
================
CSyntaxRichEditCtrl::SetDefaultColor
================
*/
void CSyntaxRichEditCtrl::SetDefaultColor( const COLORREF color ) {
	defaultColor = color;
}

/*
================
CSyntaxRichEditCtrl::SetCommentColor
================
*/
void CSyntaxRichEditCtrl::SetCommentColor( const COLORREF color ) {
	singleLineCommentColor = color;
	multiLineCommentColor = color;
}

/*
================
CSyntaxRichEditCtrl::SetStringColor
================
*/
void CSyntaxRichEditCtrl::SetStringColor( const COLORREF color, const COLORREF altColor ) {
	stringColor[0] = color;
	if ( altColor == -1 ) {
		stringColor[1] = color;
	} else {
		stringColor[1] = altColor;
	}
}

/*
================
CSyntaxRichEditCtrl::SetLiteralColor
================
*/
void CSyntaxRichEditCtrl::SetLiteralColor( const COLORREF color ) {
	literalColor = color;
}

/*
================
CSyntaxRichEditCtrl::SetObjectMemberCallback
================
*/
void CSyntaxRichEditCtrl::SetObjectMemberCallback( objectMemberCallback_t callback ) {
	GetObjectMembers = callback;
}

/*
================
CSyntaxRichEditCtrl::SetFunctionParmCallback
================
*/
void CSyntaxRichEditCtrl::SetFunctionParmCallback( toolTipCallback_t callback ) {
	GetFunctionParms = callback;
}

/*
================
CSyntaxRichEditCtrl::SetToolTipCallback
================
*/
void CSyntaxRichEditCtrl::SetToolTipCallback( toolTipCallback_t callback ) {
	GetToolTip = callback;
}

/*
================
CSyntaxRichEditCtrl::SetCaseSensitive
================
*/
void CSyntaxRichEditCtrl::SetCaseSensitive( bool caseSensitive ) {
	this->caseSensitive = caseSensitive;
}

/*
================
CSyntaxRichEditCtrl::AllowPathNames
================
*/
void CSyntaxRichEditCtrl::AllowPathNames( bool allow ) {
	allowPathNames = allow;
}

/*
================
CSyntaxRichEditCtrl::EnableKeyWordAutoCompletion
================
*/
void CSyntaxRichEditCtrl::EnableKeyWordAutoCompletion( bool enable ) {
	keyWordAutoCompletion = enable;
}

/*
================
CSyntaxRichEditCtrl::GetVisibleRange
================
*/
CHARRANGE CSyntaxRichEditCtrl::GetVisibleRange( void ) const {
	RECT rectArea;
	int firstLine, lastLine;
	CHARRANGE range;

	firstLine = GetFirstVisibleLine();
	GetClientRect( &rectArea );
	lastLine = firstLine + ( rectArea.bottom / ( defaultCharFormat.yHeight / 20 ) );
	
	if ( lastLine >= GetLineCount() ) {
		lastLine = GetLineCount() - 1;
	}
	range.cpMin = LineIndex( firstLine );
	if ( range.cpMin < 0 ) {
		range.cpMin = 0;
	}
	range.cpMax = LineIndex( lastLine );
	if ( range.cpMax == -1 ) {
		range.cpMax = range.cpMin + LineLength( range.cpMin );
	} else {
		range.cpMax += LineLength( range.cpMax );
	}
	if ( range.cpMax >= GetTextLength() ) {
		range.cpMax = GetTextLength() - 1;
	}
	return range;
}

/*
================
CSyntaxRichEditCtrl::SetDefaultFont
================
*/
void CSyntaxRichEditCtrl::SetDefaultFont( int startCharIndex, int endCharIndex ) {
	tom::ITextRange *range;

	updateSyntaxHighlighting = false;

	m_TextDoc->Range( startCharIndex, endCharIndex, &range );

	m_TextDoc->Undo( tom::tomSuspend, NULL );
	range->put_Font( m_DefaultFont );
	m_TextDoc->Undo( tom::tomResume, NULL );

	range->Release();

	updateSyntaxHighlighting = true;
}

/*
================
CSyntaxRichEditCtrl::SetColor
================
*/
void CSyntaxRichEditCtrl::SetColor( int startCharIndex, int endCharIndex, COLORREF foreColor, COLORREF backColor, bool bold ) {
	tom::ITextRange *range;
	tom::ITextFont *font;
	long prop;

	updateSyntaxHighlighting = false;

	m_TextDoc->Range( startCharIndex, endCharIndex, &range );
	range->get_Font( &font );

	m_TextDoc->Undo( tom::tomSuspend, &prop );
	font->put_ForeColor( foreColor );
	m_TextDoc->Undo( tom::tomResume, &prop );

	m_TextDoc->Undo( tom::tomSuspend, &prop );
	font->put_BackColor( backColor );
	m_TextDoc->Undo( tom::tomResume, &prop );

	m_TextDoc->Undo( tom::tomSuspend, &prop );
	font->put_Bold( bold ? tom::tomTrue : tom::tomFalse );
	m_TextDoc->Undo( tom::tomResume, &prop );

	font->Release();
	range->Release();

	updateSyntaxHighlighting = true;
}

/*
================
CSyntaxRichEditCtrl::GetForeColor
================
*/
COLORREF CSyntaxRichEditCtrl::GetForeColor( int charIndex ) const {
	tom::ITextRange *range;
	tom::ITextFont *font;
	long foreColor;

	m_TextDoc->Range( charIndex, charIndex, &range );
	range->get_Font( &font );

	font->get_BackColor( &foreColor );

	font->Release();
	range->Release();

	return foreColor;
}

/*
================
CSyntaxRichEditCtrl::GetBackColor
================
*/
COLORREF CSyntaxRichEditCtrl::GetBackColor( int charIndex ) const {
	tom::ITextRange *range;
	tom::ITextFont *font;
	long backColor;

	m_TextDoc->Range( charIndex, charIndex, &range );
	range->get_Font( &font );

	font->get_BackColor( &backColor );

	font->Release();
	range->Release();

	return backColor;
}

/*
================
CSyntaxRichEditCtrl::HighlightSyntax

  Update the syntax highlighting for the given character range.
================
*/
void CSyntaxRichEditCtrl::HighlightSyntax( int startCharIndex, int endCharIndex ) {
	int c, t, line, charIndex, textLength, syntaxStart, keyWordLength, keyWordIndex;
	const char *keyWord;
	CHARRANGE visRange;
	CString text;

	// get text length
	GetTextRange( 0, GetTextLength(), text );
	textLength = text.GetLength();

	// make sure the indexes are within bounds
	if ( startCharIndex < 0 ) {
		startCharIndex = 0;
	}
	if ( endCharIndex < 0 ) {
		endCharIndex = textLength - 1;
	} else if ( endCharIndex >= textLength ) {
		endCharIndex = textLength - 1;
	}

	// move the start index to the beginning of the line
	for ( ; startCharIndex > 0; startCharIndex-- ) {
		if ( idStr::CharIsNewLine( text[startCharIndex-1] ) ) {
			break;
		}
	}

	// move the end index to the end of the line
	for ( ; endCharIndex < textLength - 1; endCharIndex++ ) {
		if ( idStr::CharIsNewLine( text[endCharIndex+1] ) ) {
			break;
		}
	}

	// get the visible char range
	visRange = GetVisibleRange();

	// never update beyond the visible range
	if ( startCharIndex < visRange.cpMin ) {
		SetColor( startCharIndex, visRange.cpMin - 1, SRE_COLOR_BLACK, INVALID_BACK_COLOR, false );
		startCharIndex = visRange.cpMin;
	}
	if ( visRange.cpMax < endCharIndex ) {
		SetColor( visRange.cpMax, endCharIndex, SRE_COLOR_BLACK, INVALID_BACK_COLOR, false );
		endCharIndex = visRange.cpMax;
		if ( endCharIndex >= textLength ) {
			endCharIndex = textLength - 1;
		}
	}

	// test if the start index is inside a multi-line comment
	if ( startCharIndex > 0 ) {
		// multi-line comments have a slightly different background color
		if ( GetBackColor( startCharIndex-1 ) == MULTILINE_COMMENT_BACK_COLOR ) {
			for( ; startCharIndex > 0; startCharIndex-- ) {
				if ( text[startCharIndex] == '/' && text[startCharIndex+1] == '*' ) {
					break;
				}
			}
		}
	}

	// test if the end index is inside a multi-line comment
	if ( endCharIndex < textLength - 1 ) {
		// multi-line comments have a slightly different background color
		if ( GetBackColor( endCharIndex+1 ) == MULTILINE_COMMENT_BACK_COLOR ) {
			for( endCharIndex++; endCharIndex < textLength - 1; endCharIndex++ ) {
				if ( text[endCharIndex-1] == '*' && text[endCharIndex] == '/' ) {
					break;
				}
			}
		}
	}

	line = 0;
	stringColorLine = -1;
	stringColorIndex = 0;

	// set the default color
	SetDefaultFont( startCharIndex, endCharIndex + 1 );

	// syntax based colors
	for( charIndex = startCharIndex; charIndex <= endCharIndex; charIndex++ ) {

		t = charType[text[charIndex]];
		switch( t ) {
			case CT_WHITESPACE: {
				if ( idStr::CharIsNewLine( text[charIndex] ) ) {
					line++;
				}
				break;
			}
			case CT_COMMENT: {
				c = text[charIndex+1];
				if ( c == '/' ) {
					// single line comment
					syntaxStart = charIndex;
					for ( charIndex += 2; charIndex < textLength; charIndex++ ) {
						if ( idStr::CharIsNewLine( text[charIndex] ) ) {
							break;
						}
					}
					SetColor( syntaxStart, charIndex + 1, singleLineCommentColor, DEFAULT_BACK_COLOR, false );
				} else if ( c == '*' ) {
					// multi-line comment
					syntaxStart = charIndex;
					for ( charIndex += 2; charIndex < textLength; charIndex++ ) {
						if ( text[charIndex] == '*' && text[charIndex+1] == '/' ) {
							break;
						}
					}
					charIndex++;
					SetColor( syntaxStart, charIndex + 1, multiLineCommentColor, MULTILINE_COMMENT_BACK_COLOR, false );
				}
				break;
			}
			case CT_STRING: {
				if ( line != stringColorLine ) {
					stringColorLine = line;
					stringColorIndex = 0;
				}
				syntaxStart = charIndex;
				for ( charIndex++; charIndex < textLength; charIndex++ ) {
					c = text[charIndex];
					if ( charType[c] == CT_STRING && text[charIndex-1] != '\\' ) {
						break;
					}
					if ( idStr::CharIsNewLine( c ) ) {
						line++;
						break;
					}
				}
				SetColor( syntaxStart, charIndex + 1, stringColor[stringColorIndex], DEFAULT_BACK_COLOR, false );
				stringColorIndex ^= 1;
				break;
			}
			case CT_LITERAL: {
				syntaxStart = charIndex;
				for ( charIndex++; charIndex < textLength; charIndex++ ) {
					c = text[charIndex];
					if ( charType[c] == CT_LITERAL && text[charIndex-1] != '\\' ) {
						break;
					}
					if ( idStr::CharIsNewLine( c ) ) {
						line++;
						break;
					}
				}
				SetColor( syntaxStart, charIndex + 1, literalColor, DEFAULT_BACK_COLOR, false );
				break;
			}
			case CT_NUMBER: {
				break;
			}
			case CT_NAME: {
				syntaxStart = charIndex;
				keyWord = ((const char *)text) + charIndex;
				for ( charIndex++; charIndex < textLength; charIndex++ ) {
					c = text[charIndex];
					t = charType[c];
					if ( t != CT_NAME && t != CT_NUMBER ) {
						// allow path names
						if ( !allowPathNames || ( c != '/' && c != '\\' && c != '.' ) ) {
							break;
						}
					}
				}
				keyWordLength = charIndex - syntaxStart;
				keyWordIndex = FindKeyWord( keyWord, keyWordLength );
				if ( keyWordIndex != -1 ) {
					SetColor( syntaxStart, syntaxStart + keyWordLength, keyWordColors[keyWordIndex], DEFAULT_BACK_COLOR, false );
				}
				break;
			}
			case CT_PUNCTUATION: {
				break;
			}
		}
	}

	// show braced section
	BracedSectionShow();
}

/*
================
CSyntaxRichEditCtrl::UpdateVisibleRange

  Updates the visible character range if it is not yet properly highlighted.
================
*/
void CSyntaxRichEditCtrl::UpdateVisibleRange( void ) {
	CHARRANGE visRange;
	tom::ITextRange *range;
	tom::ITextFont *font;
	long backColor;
	bool update = false;

	if ( !updateSyntaxHighlighting ) {
		return;
	}

	visRange = GetVisibleRange();

	m_TextDoc->Range( visRange.cpMin, visRange.cpMax, &range );
	range->get_End( &visRange.cpMax );

	range->get_Font( &font );

	range->SetRange( visRange.cpMin, visRange.cpMin );
	while( 1 ) {
		range->get_Start( &visRange.cpMin );
		if ( visRange.cpMin >= visRange.cpMax ) {
			break;
		}
		font->get_BackColor( &backColor );
		if ( backColor == INVALID_BACK_COLOR ) {
			update = true;
			break;
		}
		if ( range->Move( tom::tomCharFormat, 1, NULL ) != S_OK ) {
			break;
		}
	}

	font->Release();
	range->Release();

	if ( update ) {
		HighlightSyntax( visRange.cpMin, visRange.cpMax - 1 );
	}
}

/*
================
CSyntaxRichEditCtrl::GetCursorPos
================
*/
void CSyntaxRichEditCtrl::GetCursorPos( int &line, int &column, int &character ) const {
	long start, end;
	char buffer[MAX_STRING_CHARS];

	GetSel( start, end );
	line = LineFromChar( start );
	start -= LineIndex( line );
	GetLine( line, buffer, sizeof( buffer ) );
	for ( column = 1, character = 0; character < start; character++ ) {
		if ( idStr::CharIsTab( buffer[character] ) ) {
			column += TAB_SIZE;
			column -= column % TAB_SIZE;
		} else {
			column++;
		}
	}
	character++;
}

/*
================
CSyntaxRichEditCtrl::GetText
================
*/
void CSyntaxRichEditCtrl::GetText( idStr &text ) const {
	GetText( text, 0, GetTextLength() );
}

/*
================
CSyntaxRichEditCtrl::GetText
================
*/
void CSyntaxRichEditCtrl::GetText( idStr &text, int startCharIndex, int endCharIndex ) const {
	tom::ITextRange *range;
	BSTR bstr;
	USES_CONVERSION;

	m_TextDoc->Range( startCharIndex, endCharIndex, &range );
	range->get_Text( &bstr );
	text = W2A( bstr );
	range->Release();
	text.StripTrailingOnce( "\r" );		// remove last carriage return which is always added to a tom::ITextRange
}

/*
================
CSyntaxRichEditCtrl::SetText
================
*/
void CSyntaxRichEditCtrl::SetText( const char *text ) {
	SetSel( 0, -1 );
	ReplaceSel( text, FALSE );
	SetSel( 0, 0 );
}

/*
================
CSyntaxRichEditCtrl::FindNext
================
*/
bool CSyntaxRichEditCtrl::FindNext( const char *find, bool matchCase, bool matchWholeWords, bool searchForward ) {
	long selStart, selEnd, flags, search, length, start;
	tom::ITextRange *range;

	if ( find[0] == '\0' ) {
		return false;
	}

	GetSel( selStart, selEnd );

	flags = 0;
	flags |= matchCase ? tom::tomMatchCase : 0;
	flags |= matchWholeWords ? tom::tomMatchWord : 0;

	if ( searchForward ) {
		m_TextDoc->Range( selEnd, GetTextLength(), &range );
		search = GetTextLength() - selEnd;
	} else {
		m_TextDoc->Range( 0, selStart, &range );
		search = -selStart;
	}

	if ( range->FindShit( A2BSTR(find), search, flags, &length ) == S_OK ) {

		m_TextDoc->Freeze( NULL );

		range->get_Start( &start );
		range->Release();

		SetSel( start, start + length );

		int line = Max( (int) LineFromChar( start ) - 5, 0 );
		LineScroll( line - GetFirstVisibleLine(), 0 );

		UpdateVisibleRange();

		m_TextDoc->Unfreeze( NULL );
		return true;
	} else {
		range->Release();
		return false;
	}
}

/*
================
CSyntaxRichEditCtrl::ReplaceAll
================
*/
int CSyntaxRichEditCtrl::ReplaceAll( const char *find, const char *replace, bool matchCase, bool matchWholeWords ) {
	long selStart, selEnd, flags, search, length, start;
	int numReplaced;
	tom::ITextRange *range;
	CComBSTR bstr( find );

	if ( find[0] == '\0' ) {
		return 0;
	}

	m_TextDoc->Freeze( NULL );

	GetSel( selStart, selEnd );

	flags = 0;
	flags |= matchCase ? tom::tomMatchCase : 0;
	flags |= matchWholeWords ? tom::tomMatchWord : 0;

	m_TextDoc->Range( 0, GetTextLength(), &range );
	search = GetTextLength();

	numReplaced = 0;
	while( range->FindShit( bstr, search, flags, &length ) == S_OK ) {
		range->get_Start( &start );
		ReplaceText( start, start + length, replace );
		numReplaced++;
	}

	range->Release();

	m_TextDoc->Unfreeze( NULL );

	return numReplaced;
}

/*
================
CSyntaxRichEditCtrl::ReplaceText
================
*/
void CSyntaxRichEditCtrl::ReplaceText( int startCharIndex, int endCharIndex, const char *replace ) {
	tom::ITextRange *range;
	CComBSTR bstr( replace );

	m_TextDoc->Range( startCharIndex, endCharIndex, &range );
	range->put_Text( bstr );
	range->Release();
}

/*
================
CSyntaxRichEditCtrl::AutoCompleteInsertText
================
*/
void CSyntaxRichEditCtrl::AutoCompleteInsertText( void ) {
	long selStart, selEnd;
	int index;

	index = autoCompleteListBox.GetCurSel();
	if ( index >= 0 ) {
		CString text;
		autoCompleteListBox.GetText( index, text );
		GetSel( selStart, selEnd );
		selStart = autoCompleteStart;
		SetSel( selStart, selEnd );
		ReplaceSel( text, TRUE );
	}
}

/*
================
CSyntaxRichEditCtrl::AutoCompleteUpdate
================
*/
void CSyntaxRichEditCtrl::AutoCompleteUpdate( void ) {
	long selStart, selEnd;
	int index;
	idStr text;

	GetSel( selStart, selEnd );
	GetText( text, autoCompleteStart, selStart );
	index = autoCompleteListBox.FindString( -1, text );
	if ( index >= 0 && index < autoCompleteListBox.GetCount() ) {
		autoCompleteListBox.SetCurSel( index );
	}
}

/*
================
CSyntaxRichEditCtrl::AutoCompleteShow
================
*/
void CSyntaxRichEditCtrl::AutoCompleteShow( int charIndex ) {
	CPoint point;
	CRect rect;

	autoCompleteStart = charIndex;
	point = PosFromChar( charIndex );
	GetClientRect( rect );
	if ( point.y < rect.bottom - AUTOCOMPLETE_OFFSET - AUTOCOMPLETE_HEIGHT ) {
		rect.top = point.y + AUTOCOMPLETE_OFFSET;
		rect.bottom = point.y + AUTOCOMPLETE_OFFSET + AUTOCOMPLETE_HEIGHT;
	} else {
		rect.top = point.y - AUTOCOMPLETE_HEIGHT;
		rect.bottom = point.y;
	}
	rect.left = point.x;
	rect.right = point.x + AUTOCOMPLETE_WIDTH;
	autoCompleteListBox.MoveWindow( &rect );
	autoCompleteListBox.ShowWindow( TRUE );
	AutoCompleteUpdate();
}

/*
================
CSyntaxRichEditCtrl::AutoCompleteHide
================
*/
void CSyntaxRichEditCtrl::AutoCompleteHide( void ) {
	autoCompleteStart = -1;
	autoCompleteListBox.ShowWindow( FALSE );
}

/*
================
CSyntaxRichEditCtrl::ToolTipShow
================
*/
void CSyntaxRichEditCtrl::ToolTipShow( int charIndex, const char *string ) {
	CPoint point, p1, p2;
	CRect rect;

	funcParmToolTipStart = charIndex;
	funcParmToolTip.SetWindowText( string );
	p1 = funcParmToolTip.PosFromChar( 0 );
	p2 = funcParmToolTip.PosFromChar( strlen( string ) - 1 );
	point = PosFromChar( charIndex );
	GetClientRect( rect );
	if ( point.y < rect.bottom - FUNCPARMTOOLTIP_OFFSET - FUNCPARMTOOLTIP_HEIGHT ) {
		rect.top = point.y + FUNCPARMTOOLTIP_OFFSET;
		rect.bottom = point.y + FUNCPARMTOOLTIP_OFFSET + FUNCPARMTOOLTIP_HEIGHT;
	} else {
		rect.top = point.y - FUNCPARMTOOLTIP_HEIGHT;
		rect.bottom = point.y;
	}
	rect.left = point.x;
	rect.right = point.x + FUNCPARMTOOLTIP_WIDTH + p2.x - p1.x;
	funcParmToolTip.MoveWindow( &rect );
	funcParmToolTip.ShowWindow( TRUE );
}

/*
================
CSyntaxRichEditCtrl::ToolTipHide
================
*/
void CSyntaxRichEditCtrl::ToolTipHide( void ) {
	funcParmToolTipStart = -1;
	funcParmToolTip.ShowWindow( FALSE );
}

/*
================
CSyntaxRichEditCtrl::BracedSectionStart
================
*/
bool CSyntaxRichEditCtrl::BracedSectionStart( char braceStartChar, char braceEndChar ) {
	long selStart, selEnd;
	int brace, i;
	idStr text;

	GetSel( selStart, selEnd );
	GetText( text, 0, GetTextLength() );

	for ( brace = 1, i = selStart; i < text.Length(); i++ ) {
		if ( text[i] == braceStartChar ) {
			brace++;
		} else if ( text[i] == braceEndChar ) {
			brace--;
			if ( brace == 0 ) {
				break;
			}
		}
	}
	if ( brace == 0 ) {
		bracedSection[0] = selStart - 1;
		bracedSection[1] = i;
		BracedSectionShow();
	}

	return ( brace == 0 );
}

/*
================
CSyntaxRichEditCtrl::BracedSectionEnd
================
*/
bool CSyntaxRichEditCtrl::BracedSectionEnd( char braceStartChar, char braceEndChar ) {
	long selStart, selEnd;
	int brace, i;
	idStr text;

	GetSel( selStart, selEnd );
	GetText( text, 0, GetTextLength() );

	for ( brace = 1, i = Min( selStart-2, (long)text.Length()-1 ); i >= 0; i-- ) {
		if ( text[i] == braceStartChar ) {
			brace--;
			if ( brace == 0 ) {
				break;
			}
		} else if ( text[i] == braceEndChar ) {
			brace++;
		}
	}

	if ( brace == 0 ) {
		bracedSection[0] = i;
		bracedSection[1] = selStart - 1;
		BracedSectionAdjustEndTabs();
		BracedSectionShow();
	}

	return ( brace == 0 );
}

/*
================
CSyntaxRichEditCtrl::BracedSectionAdjustEndTabs
================
*/
void CSyntaxRichEditCtrl::BracedSectionAdjustEndTabs( void ) {
	int line, lineIndex, length, column, numTabs, i;
	char buffer[1024];
	idStr text;

	line = LineFromChar( bracedSection[0] );
	length = GetLine( line, buffer, sizeof( buffer ) );
	for ( numTabs = 0; numTabs < length; numTabs++ ) {
		if ( !idStr::CharIsTab( buffer[numTabs] ) ) {
			break;
		}
		text.Append( '\t' );
	}

	line = LineFromChar( bracedSection[1] );
	lineIndex = LineIndex( line );
	length = GetLine( line, buffer, sizeof( buffer ) );
	column = bracedSection[1] - lineIndex;
	for ( i = 0; i < column; i++ ) {
		if ( charType[buffer[i]] != CT_WHITESPACE ) {
			return;
		}
	}

	ReplaceText( lineIndex, lineIndex + column, text );

	bracedSection[1] += numTabs - column;
	SetSel( bracedSection[1]+1, bracedSection[1]+1 );
}

/*
================
CSyntaxRichEditCtrl::BracedSectionShow
================
*/
void CSyntaxRichEditCtrl::BracedSectionShow( void ) {
	for ( int i = 0; i < 2; i++ ) {
		if ( bracedSection[i] >= 0 ) {
			SetColor( bracedSection[i], bracedSection[i] + 1, braceHighlightColor, DEFAULT_BACK_COLOR, true );
		}
	}
}

/*
================
CSyntaxRichEditCtrl::BracedSectionHide
================
*/
void CSyntaxRichEditCtrl::BracedSectionHide( void ) {
	for ( int i = 0; i < 2; i++ ) {
		if ( bracedSection[i] >= 0 ) {
			SetColor( bracedSection[i], bracedSection[i] + 1, defaultColor, DEFAULT_BACK_COLOR, false );
			bracedSection[i] = -1;
		}
	}
}

/*
================
CSyntaxRichEditCtrl::GetNameBeforeCurrentSelection
================
*/
bool CSyntaxRichEditCtrl::GetNameBeforeCurrentSelection( CString &name, int &charIndex ) const {
	long selStart, selEnd;
	int line, column, length;
	char buffer[1024];

	GetSel( selStart, selEnd );
	charIndex = selStart;
	line = LineFromChar( selStart );
	length = GetLine( line, buffer, sizeof( buffer ) );
	column = selStart - LineIndex( line ) - 1;
	do {
		buffer[column--] = '\0';
	} while( charType[buffer[column]] == CT_WHITESPACE );
	for ( length = 0; length < column; length++ ) {
		if ( charType[buffer[column-length-1]] != CT_NAME ) {
			break;
		}
	}
	if ( length > 0 ) {
		name = buffer + column - length;
		return true;
	}
	return false;
}

/*
================
CSyntaxRichEditCtrl::GetNameForMousePosition
================
*/
bool CSyntaxRichEditCtrl::GetNameForMousePosition( idStr &name ) const {
	int charIndex, startCharIndex, endCharIndex, type;
	idStr text;

	charIndex = CharFromPos( mousePoint );

	for ( startCharIndex = charIndex; startCharIndex > 0; startCharIndex-- ) {
		GetText( text, startCharIndex - 1, startCharIndex );
		type = charType[text[0]];
		if ( type != CT_NAME && type != CT_NUMBER ) {
			break;
		}
	}

	for ( endCharIndex = charIndex; endCharIndex < GetTextLength(); endCharIndex++ ) {
		GetText( text, endCharIndex, endCharIndex + 1 );
		type = charType[text[0]];
		if ( type != CT_NAME && type != CT_NUMBER ) {
			break;
		}
	}

	GetText( name, startCharIndex, endCharIndex );

	return ( endCharIndex > startCharIndex );
}

/*
================
CSyntaxRichEditCtrl::GoToLine
================
*/
void CSyntaxRichEditCtrl::GoToLine( int line ) {

	int index = LineIndex( line );

	m_TextDoc->Freeze( NULL );

	SetSel( index, index );

	m_TextDoc->Unfreeze( NULL );

	UpdateVisibleRange();

	RedrawWindow();
}

/*
================
CSyntaxRichEditCtrl::OnToolHitTest
================
*/
int CSyntaxRichEditCtrl::OnToolHitTest( CPoint point, TOOLINFO* pTI ) const {
	CRichEditCtrl::OnToolHitTest( point, pTI );

    pTI->hwnd = GetSafeHwnd();
    pTI->uId = (UINT_PTR)GetSafeHwnd();
	pTI->uFlags |= TTF_IDISHWND;
    pTI->lpszText = LPSTR_TEXTCALLBACK;
    pTI->rect = CRect( point, point );
	pTI->rect.right += 100;
	pTI->rect.bottom += 20;
    return pTI->uId;
}

/*
================
CSyntaxRichEditCtrl::OnToolTipNotify
================
*/
BOOL CSyntaxRichEditCtrl::OnToolTipNotify( UINT id, NMHDR *pNMHDR, LRESULT *pResult ) {
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;

	*pResult = 0;

	idStr name;

	if ( GetNameForMousePosition( name ) ) {
		CString toolTip;

		if ( GetToolTip == NULL || !GetToolTip( name, toolTip ) ) {

			int keyWordIndex = FindKeyWord( name, name.Length() );

			if ( keyWordIndex != -1 && keyWords[keyWordIndex].description[0] != '\0' ) {
				toolTip = keyWords[keyWordIndex].description;
			} else {
				toolTip = name.c_str();
			}
		}

		AFX_MODULE_THREAD_STATE *state = AfxGetModuleThreadState();

		// set max tool tip width to enable multi-line tool tips using "\r\n" for line breaks
		state->m_pToolTip->SetMaxTipWidth( 500 );

		// set the number of milliseconds after which the tool tip automatically disappears
		state->m_pToolTip->SetDelayTime( TTDT_AUTOPOP, 5000 + toolTip.GetLength() * 50 );

#ifndef _UNICODE
		if( pNMHDR->code == TTN_NEEDTEXTA ) {
			delete m_pchTip;
			m_pchTip = new TCHAR[toolTip.GetLength() + 2];
			lstrcpyn( m_pchTip, toolTip, toolTip.GetLength() + 1 );
			pTTTW->lpszText = (WCHAR*)m_pchTip;
		} else {
			delete m_pwchTip;
			m_pwchTip = new WCHAR[toolTip.GetLength() + 2];
			_mbstowcsz( m_pwchTip, toolTip, toolTip.GetLength() + 1 );
			pTTTW->lpszText = (WCHAR*)m_pwchTip;
		}
#else
		if( pNMHDR->code == TTN_NEEDTEXTA ) {
			delete m_pchTip;
			m_pchTip = new TCHAR[toolTip.GetLength() + 2];
			_wcstombsz( m_pchTip, toolTip, toolTip.GetLength() + 1 );
			pTTTA->lpszText = (LPTSTR)m_pchTip;
		} else {
			delete m_pwchTip;
			m_pwchTip = new WCHAR[toolTip.GetLength() + 2];
			lstrcpyn( m_pwchTip, toolTip, toolTip.GetLength() + 1 );
			pTTTA->lpszText = (LPTSTR) m_pwchTip;
		}
#endif

		return TRUE;
	}
	return FALSE;
}

/*
================
CSyntaxRichEditCtrl::OnGetDlgCode
================
*/
UINT CSyntaxRichEditCtrl::OnGetDlgCode() {
	// get all keys, including tabs
	return DLGC_WANTALLKEYS | DLGC_WANTARROWS | DLGC_WANTCHARS | DLGC_WANTMESSAGE | DLGC_WANTTAB;
}

/*
================
CSyntaxRichEditCtrl::OnKeyDown
================
*/
void CSyntaxRichEditCtrl::OnKeyDown( UINT nKey, UINT nRepCnt, UINT nFlags ) {

	if ( m_TextDoc == NULL ) {
		return;
	}

	if ( autoCompleteStart >= 0 ) {
		int sel;

		switch( nKey ) {
			case VK_UP: {		// up arrow
				sel = Max( 0, autoCompleteListBox.GetCurSel() - 1 );
				autoCompleteListBox.SetCurSel( sel );
				return;
			}
			case VK_DOWN: {		// down arrow
				sel = Min( autoCompleteListBox.GetCount() - 1, autoCompleteListBox.GetCurSel() + 1 );
				autoCompleteListBox.SetCurSel( sel );
				return;
			}
			case VK_PRIOR: {	// page up key
				sel = Max( 0, autoCompleteListBox.GetCurSel() - 10 );
				autoCompleteListBox.SetCurSel( sel );
				return;
			}
			case VK_NEXT: {		// page down key
				sel = Min( autoCompleteListBox.GetCount() - 1, autoCompleteListBox.GetCurSel() + 10 );
				autoCompleteListBox.SetCurSel( sel );
				return;
			}
			case VK_HOME: {		// home key
				autoCompleteListBox.SetCurSel( 0 );
				return;
			}
			case VK_END: {
				autoCompleteListBox.SetCurSel( autoCompleteListBox.GetCount() - 1 );
				return;
			}
			case VK_RETURN:		// enter key
			case VK_TAB: {		// tab key
				AutoCompleteInsertText();
				AutoCompleteHide();
				return;
			}
			case VK_LEFT:		// left arrow
			case VK_RIGHT:		// right arrow
			case VK_INSERT:		// insert key
			case VK_DELETE: {	// delete key
				return;
			}
		}
	}

	BracedSectionHide();

	switch( nKey ) {
		case VK_TAB: {		// multi-line tabs
			long selStart, selEnd;

			GetSel( selStart, selEnd );

			// if multiple lines are selected add tabs to, or remove tabs from all of them
			if ( selEnd > selStart ) {
				CString text;

				text = GetSelText();

				if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) {
					if ( idStr::CharIsTab( text[0] ) ) {
						text.Delete( 0, 1 );
					}
					for ( int i = 0; i < text.GetLength() - 2; i++ ) {
						if ( idStr::CharIsNewLine( text[i] ) ) {
							do {
								i++;
							} while( idStr::CharIsNewLine( text[i] ) );
							if ( idStr::CharIsTab( text[i] ) ) {
								text.Delete( i, 1 );
							}
						}
					}
				} else {
					text.Insert( 0, '\t' );
					for ( int i = 0; i < text.GetLength() - 1; i++ ) {
						if ( idStr::CharIsNewLine( text[i] ) ) {
							do {
								i++;
							} while( idStr::CharIsNewLine( text[i] ) );
							text.Insert( i, '\t' );
						}
					}
				}

				ReplaceSel( text, TRUE );
				SetSel( selStart, selStart + text.GetLength() );
			} else {
				ReplaceSel( "\t", TRUE );
			}
			return;
		}
		case VK_RETURN: {	// auto-indentation
			long selStart, selEnd;
			int line, length, numTabs, i;
			char buffer[1024];
			idStr text;

			GetSel( selStart, selEnd );
			line = LineFromChar( selStart );
			length = GetLine( line, buffer, sizeof( buffer ) );
			for ( numTabs = 0; numTabs < length; numTabs++ ) {
				if ( !idStr::CharIsTab( buffer[numTabs] ) ) {
					break;
				}
			}
			bool first = true;
			for ( i = numTabs; i < length; i++ ) {
				if ( buffer[i] == '{' ) {
					numTabs++;
					first = false;
				} else if ( buffer[i] == '}' && !first ) {
					numTabs--;
				}
			}
			text = "\r\n";
			for ( i = 0; i < numTabs; i++ ) {
				text.Append( '\t' );
			}
			ReplaceSel( text, TRUE );
			return;
		}
	}

	m_TextDoc->Freeze( NULL );

	CRichEditCtrl::OnKeyDown( nKey, nRepCnt, nFlags );

	UpdateVisibleRange();

	m_TextDoc->Unfreeze( NULL );
}

/*
================
CSyntaxRichEditCtrl::OnChar
================
*/
void CSyntaxRichEditCtrl::OnChar( UINT nChar, UINT nRepCnt, UINT nFlags ) {

	if ( nChar == VK_TAB ) {
		return;	// tab is handle in OnKeyDown
	}

	CRichEditCtrl::OnChar( nChar, nRepCnt, nFlags );

	// if the auto-complete list box is up
	if ( autoCompleteStart >= 0 ) {
		long selStart, selEnd;

		if ( charType[nChar] == CT_NAME ) {
			AutoCompleteUpdate();
			return;
		} else if ( nChar == VK_BACK ) {
			GetSel( selStart, selEnd );
			if ( selStart > autoCompleteStart ) {
				AutoCompleteUpdate();
			} else {
				AutoCompleteHide();
			}
			return;
		} else {
			AutoCompleteHide();
		}
	}

	// if the function parameter tool tip is up
	if ( funcParmToolTipStart >= 0 ) {
		long selStart, selEnd;

		if ( nChar == ')' || nChar == VK_ESCAPE ) {
			ToolTipHide();
		} else if ( nChar == VK_BACK ) {
			GetSel( selStart, selEnd );
			if ( selStart < funcParmToolTipStart ) {
				ToolTipHide();
			}
		}
	}

	// show keyword auto-completion
	if ( keyWordAutoCompletion && charType[nChar] == CT_NAME && funcParmToolTipStart < 0 ) {
		long selStart, selEnd;
		int line, column, length, i;
		char buffer[1024];

		GetSel( selStart, selEnd );
		line = LineFromChar( selStart );
		length = GetLine( line, buffer, sizeof( buffer ) );
		column = selStart - LineIndex( line );
		if ( column <= 1 || charType[buffer[column-2]] == CT_WHITESPACE ) {
			if ( column >= length-1 || charType[buffer[column]] == CT_WHITESPACE ) {

				autoCompleteListBox.ResetContent();
				for ( i = 0; keyWords[i].keyWord; i++ ) {
					autoCompleteListBox.AddString( keyWords[i].keyWord );
				}
				AutoCompleteShow( selStart - 1 );
			}
		}
		return;
	}

	// highlight braced sections
	if ( nChar == '{' ) {
		BracedSectionStart( '{', '}' );
	} else if ( nChar == '}' ) {
		BracedSectionEnd( '{', '}' );
	} else if ( nChar == '(' ) {
		BracedSectionStart( '(', ')' );
	} else if ( nChar == ')' ) {
		BracedSectionEnd( '(', ')' );
	} else if ( nChar == '[' ) {
		BracedSectionStart( '[', ']' );
	} else if ( nChar == ']' ) {
		BracedSectionEnd( '[', ']' );
	} else if ( nChar == '<' ) {
		BracedSectionStart( '<', '>' );
	} else if ( nChar == '>' ) {
		BracedSectionEnd( '<', '>' );
	}

	// show object member auto-completion
	if ( nChar == '.' && GetObjectMembers && funcParmToolTipStart < 0 ) {
		int charIndex;
		CString name;

		if ( GetNameBeforeCurrentSelection( name, charIndex ) ) {
			autoCompleteListBox.ResetContent();
			if ( GetObjectMembers( name, autoCompleteListBox ) ) {
				AutoCompleteShow( charIndex );
			}
		}
		return;
	}

	// show function parameter tool tip
	if ( nChar == '(' && GetFunctionParms ) {
		int charIndex;
		CString name;

		if ( GetNameBeforeCurrentSelection( name, charIndex ) ) {
			CString parmString;
			if ( GetFunctionParms( name, parmString ) ) {
				ToolTipShow( charIndex, parmString );
			}
		}
		return;
	}
}

/*
================
CSyntaxRichEditCtrl::OnLButtonDown
================
*/
void CSyntaxRichEditCtrl::OnLButtonDown( UINT nFlags, CPoint point ) {

	if ( autoCompleteStart >= 0 ) {
		AutoCompleteHide();
	}

	BracedSectionHide();

	CRichEditCtrl::OnLButtonDown( nFlags, point );
}

/*
================
CSyntaxRichEditCtrl::OnMouseWheel
================
*/
BOOL CSyntaxRichEditCtrl::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt ) {
	if ( autoCompleteStart >= 0 ) {
		int sel;

		if ( zDelta > 0  ) {
			sel = Max( 0, autoCompleteListBox.GetCurSel() - ( zDelta / WHEEL_DELTA ) );
		} else {
			sel = Min( autoCompleteListBox.GetCount() - 1, autoCompleteListBox.GetCurSel() - ( zDelta / WHEEL_DELTA ) );
		}
		autoCompleteListBox.SetCurSel( sel );
		return TRUE;
	}

	m_TextDoc->Freeze( NULL );

	LineScroll( -3 * ( (int) zDelta ) / WHEEL_DELTA, 0 );

	UpdateVisibleRange();

	m_TextDoc->Unfreeze( NULL );

	return TRUE;
}

/*
================
CSyntaxRichEditCtrl::OnMouseMove
================
*/
void CSyntaxRichEditCtrl::OnMouseMove( UINT nFlags, CPoint point ) {
	CRichEditCtrl::OnMouseMove( nFlags, point );

	if ( point != mousePoint ) {
		mousePoint = point;

		// remove tool tip and activate the tool tip control, otherwise
		// tool tips stop working until the mouse moves over another window first
		AFX_MODULE_THREAD_STATE *state = AfxGetModuleThreadState();
		state->m_pToolTip->Pop();
		state->m_pToolTip->Activate( TRUE );
	}
}

/*
================
CSyntaxRichEditCtrl::OnSize
================
*/
void CSyntaxRichEditCtrl::OnSize( UINT nType, int cx, int cy ) {
	m_TextDoc->Freeze( NULL );

	CRichEditCtrl::OnSize( nType, cx, cy );

	m_TextDoc->Unfreeze( NULL );

	UpdateVisibleRange();
}

/*
================
CSyntaxRichEditCtrl::OnVScroll
================
*/
void CSyntaxRichEditCtrl::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar ) {
	m_TextDoc->Freeze( NULL );

	CRichEditCtrl::OnVScroll( nSBCode, nPos, pScrollBar );

	SetFocus();

	UpdateVisibleRange();

	m_TextDoc->Unfreeze( NULL );
}

/*
================
CSyntaxRichEditCtrl::OnProtected
================
*/
void CSyntaxRichEditCtrl::OnProtected( NMHDR *pNMHDR, LRESULT *pResult ) {
	ENPROTECTED* pEP = (ENPROTECTED*)pNMHDR;

	*pResult = 0;

	updateRange = pEP->chrg;

	switch( pEP->msg ) {
		case WM_MOUSEMOVE: {
			break;
		}
		case WM_SETTEXT: {
			updateRange.cpMin = pEP->chrg.cpMin;
			updateRange.cpMax = pEP->chrg.cpMin + strlen( (LPCTSTR) pEP->lParam );
			break;
		}
		case WM_CUT: {
			break;
		}
		case WM_COPY: {
			break;
		}
		case WM_PASTE: {
			break;
		}
		case WM_CLEAR: {
			break;
		}
		case WM_UNDO: {
			break;
		}
		default: {
			break;
		}
	}
}

/*
================
CSyntaxRichEditCtrl::OnChange
================
*/
void CSyntaxRichEditCtrl::OnChange() {
	long selStart, selEnd;

	if ( !updateSyntaxHighlighting ) {
		return;
	}

	GetSel( selStart, selEnd );
	selStart = Min( selStart, updateRange.cpMin );
	selEnd = Max( selEnd, updateRange.cpMax );

	HighlightSyntax( selStart, selEnd );

	// send EN_CHANGE notification to parent window
	NMHDR pNMHDR;
	pNMHDR.hwndFrom = GetSafeHwnd();
	pNMHDR.idFrom = GetDlgCtrlID();
	pNMHDR.code = EN_CHANGE;
	GetParent()->SendMessage( WM_NOTIFY, ( EN_CHANGE << 16 ) | GetDlgCtrlID(), (LPARAM)&pNMHDR );
}

/*
================
CSyntaxRichEditCtrl::OnAutoCompleteListBoxChange
================
*/
void CSyntaxRichEditCtrl::OnAutoCompleteListBoxChange() {
	// steal focus back from the auto-complete list box
	SetFocus();
}

/*
================
CSyntaxRichEditCtrl::OnAutoCompleteListBoxDblClk
================
*/
void CSyntaxRichEditCtrl::OnAutoCompleteListBoxDblClk() {
	// steal focus back from the auto-complete list box
	SetFocus();

	// insert current auto-complete selection
	AutoCompleteInsertText();
	AutoCompleteHide();
}
