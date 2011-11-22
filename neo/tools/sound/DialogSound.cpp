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

#include "../../game/game.h"
#include "../../sys/win32/win_local.h"
#include "../../sys/win32/rc/common_resource.h"
#include "../../sys/win32/rc/SoundEditor_resource.h"
#include "../comafx/DialogName.h"
#include "../../sys/win32/rc/DeclEditor_resource.h"
#include "../decl/DialogDeclEditor.h"

#include "DialogSound.h"
#include "DialogSoundGroup.h"

#ifdef ID_DEBUG_MEMORY
#undef new
#undef DEBUG_NEW
#define DEBUG_NEW new
#endif

extern HTREEITEM FindTreeItem(CTreeCtrl *tree, HTREEITEM root, const char *text, HTREEITEM forceParent);


/////////////////////////////////////////////////////////////////////////////
// CDialogSound dialog
CDialogSound *g_SoundDialog = NULL;


CDialogSound::CDialogSound(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogSound::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogSound)
	strName = _T("");
	fVolume = 0.0f;
	fMax = 0.0f;
	fMin = 0.0f;
	strShader = _T("");
	bPlay = TRUE;
	bTriggered = FALSE;
	bOmni = FALSE;
	strGroup = _T("");
	bGroupOnly = FALSE;
	bOcclusion = FALSE;
	leadThrough = 0.0f;
	plain = FALSE;
	inUseTree = NULL;
	random = 0.0f;
	wait = 0.0f;
	shakes = 0.0f;
	looping = TRUE;
	unclamped = FALSE;
	//}}AFX_DATA_INIT
}


void CDialogSound::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogSound)
	DDX_Control(pDX, IDC_COMBO_SPEAKERS, comboSpeakers);
	DDX_Control(pDX, IDC_COMBO_GROUPS, comboGroups);
	DDX_Control(pDX, IDC_EDIT_VOLUME, editVolume);
	DDX_Control(pDX, IDC_TREE_SOUNDS, treeSounds);
	DDX_Text(pDX, IDC_EDIT_SOUND_NAME, strName);
	DDX_Text(pDX, IDC_EDIT_VOLUME, fVolume);
	DDX_Text(pDX, IDC_EDIT_RANDOM, random);
	DDX_Text(pDX, IDC_EDIT_WAIT, wait);
	DDX_Text(pDX, IDC_EDIT_MAXDIST, fMax);
	DDX_Text(pDX, IDC_EDIT_MINDIST, fMin);
	DDX_Text(pDX, IDC_EDIT_SHADER, strShader);
	DDX_Check(pDX, IDC_CHECK_PLAY, bPlay);
	DDX_Check(pDX, IDC_CHECKP_TRIGGERED, bTriggered);
	DDX_Check(pDX, IDC_CHECK_OMNI, bOmni);
	DDX_Text(pDX, IDC_EDIT_GROUP, strGroup);
	DDX_Check(pDX, IDC_CHECK_GROUPONLY, bGroupOnly);
	DDX_Check(pDX, IDC_CHECK_OCCLUSION, bOcclusion);
	DDX_Text(pDX, IDC_EDIT_LEADTHROUGH, leadThrough);
	DDX_Check(pDX, IDC_CHECK_PLAIN, plain);
	DDX_Check(pDX, IDC_CHECK_LOOPING, looping);
	DDX_Check(pDX, IDC_CHECK_UNCLAMPED, unclamped);
	DDX_Text(pDX, IDC_EDIT_SHAKES, shakes);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogSound, CDialog)
	//{{AFX_MSG_MAP(CDialogSound)
	ON_BN_CLICKED(IDC_BTN_SAVEMAP, OnBtnSavemap)
	ON_BN_CLICKED(IDC_BTN_SWITCHTOGAME, OnBtnSwitchtogame)
	ON_BN_CLICKED(IDC_BTN_APPLY_SOUND, OnBtnApply)
	ON_EN_CHANGE(IDC_EDIT_VOLUME, OnChangeEditVolume)
	ON_BN_CLICKED(IDC_BTN_REFRESH, OnBtnRefresh)
	ON_BN_CLICKED(IDC_BTN_PLAYSOUND, OnBtnPlaysound)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_SOUNDS, OnDblclkTreeSounds)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_SOUNDS, OnSelchangedTreeSounds)
	ON_BN_CLICKED(IDC_CHECK_PLAY, OnCheckPlay)
	ON_BN_CLICKED(IDC_BTN_EDIT_SOUND, OnBtnEdit)
	ON_BN_CLICKED(IDC_BTN_DROP, OnBtnDrop)
	ON_BN_CLICKED(IDC_BTN_GROUP, OnBtnGroup)
	ON_BN_CLICKED(IDC_BTN_SAVEMAPAS, OnBtnSavemapas)
	ON_BN_CLICKED(IDC_BTN_YUP, OnBtnYup)
	ON_BN_CLICKED(IDC_BTN_YDN, OnBtnYdn)
	ON_BN_CLICKED(IDC_BTN_XDN, OnBtnXdn)
	ON_BN_CLICKED(IDC_BTN_XUP, OnBtnXup)
	ON_BN_CLICKED(IDC_BTN_ZUP, OnBtnZup)
	ON_BN_CLICKED(IDC_BTN_ZDN, OnBtnZdn)
	ON_BN_CLICKED(IDC_BTN_TRIGGER, OnBtnTrigger)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CHECK_GROUPONLY, OnCheckGrouponly)
	ON_CBN_SELCHANGE(IDC_COMBO_GROUPS, OnSelchangeComboGroups)
	ON_CBN_SELCHANGE(IDC_COMBO_SPEAKERS, OnSelchangeComboSpeakers)
	ON_BN_CLICKED(IDC_BTN_DOWN, OnBtnDown)
	ON_BN_CLICKED(IDC_BTN_UP, OnBtnUp)
	ON_BN_CLICKED(IDC_BTN_REFRESHSPEAKERS, OnBtnRefreshspeakers)
	ON_BN_CLICKED(IDC_BTN_REFRESHWAVE, OnBtnRefreshwave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialogSound message handlers

void SoundEditorInit( const idDict *spawnArgs ) {

	if ( renderSystem->IsFullScreen() ) {
		common->Printf( "Cannot run the sound editor in fullscreen mode.\n"
					"Set r_fullscreen to 0 and vid_restart.\n" );
		return;
	}

	if ( g_SoundDialog == NULL ) {
		InitAfx();
		g_SoundDialog = new CDialogSound();
	}

	if ( g_SoundDialog->GetSafeHwnd() == NULL ) {
		g_SoundDialog->Create(IDD_DIALOG_SOUND);
/*
		// FIXME: restore position
		CRect rct;
		g_SoundDialog->SetWindowPos( NULL, rct.left, rct.top, 0,0, SWP_NOSIZE );
*/
	}

	idKeyInput::ClearStates();

	g_SoundDialog->ShowWindow( SW_SHOW );
	g_SoundDialog->SetFocus();
	
	if ( spawnArgs ) {
		const char *name = spawnArgs->GetString( "name" );
		const idDict *dict = gameEdit->MapGetEntityDict( name );
		g_SoundDialog->Set( dict );
	}
}

void SoundEditorRun( void ) {
#if _MSC_VER >= 1300
	MSG *msg = AfxGetCurrentMessage();			// TODO Robert fix me!!
#else
	MSG *msg = &m_msgCur;
#endif

	while( ::PeekMessage(msg, NULL, NULL, NULL, PM_NOREMOVE) ) {
		// pump message
		if ( !AfxGetApp()->PumpMessage() ) {
		}
	}
}

void SoundEditorShutdown( void ) {
	delete g_SoundDialog;
	g_SoundDialog = NULL;
}

void CDialogSound::OnActivate( UINT nState, CWnd *pWndOther, BOOL bMinimized ) {
	CDialog::OnActivate( nState, pWndOther, bMinimized );
	if ( nState != WA_INACTIVE ) {
	}
}

void CDialogSound::OnMove( int x, int y ) {
	if ( GetSafeHwnd() ) {
		CRect rct;
		GetWindowRect( rct );
		// FIXME: save position
	}
	CDialog::OnMove( x, y );
}

void CDialogSound::OnDestroy() {

	com_editors &= ~EDITOR_SOUND;

	return CDialog::OnDestroy();
}

void CDialogSound::Set( const idDict *source ) {

	if ( source == NULL ) {
		return;
	}

	fVolume = source->GetFloat( "s_volume", "0" );
	fMin = source->GetFloat( "s_mindistance", "1" );
	fMax = source->GetFloat( "s_maxdistance", "10" );
	leadThrough = source->GetFloat( "s_leadthrough", "0.1" );
	plain = source->GetBool( "s_plain" );
	strShader = source->GetString( "s_shader" );
	strGroup = source->GetString( "soundgroup" );
	bOmni = source->GetInt( "s_omni", "-1" );
	bOcclusion = source->GetBool( "s_occlusion", "0" );
	bTriggered = source->GetInt( "s_waitfortrigger", "-1" );
	random = source->GetFloat( "random" );
	wait = source->GetFloat( "wait" );
	strName = source->GetString("name");
	looping = source->GetBool( "s_looping" );
	unclamped = source->GetBool( "s_unclamped" );
	shakes = source->GetFloat( "s_shakes" );
	if (comboSpeakers.SelectString(-1, strName) == CB_ERR) {
		comboSpeakers.SetCurSel(-1);
	}
	if (comboGroups.SelectString(-1, strGroup) == CB_ERR) {
		comboGroups.SetCurSel(-1);
	}
	UpdateData(FALSE);
}

void CDialogSound::Get( idDict *source ) {

	if ( source == NULL ) {
		return;
	}
	UpdateData( TRUE );
	float f = source->GetFloat( "s_volume" );
	source->SetFloat( "s_volume", f );
	source->SetFloat( "s_mindistance", fMin );
	source->SetFloat( "s_maxdistance", fMax );
	source->Set( "s_shader", strShader );
	source->SetInt( "s_omni", bOmni );
	source->SetBool( "s_occlusion", ( bOcclusion != FALSE ) );
	source->SetInt("s_waitfortrigger", bTriggered );
	source->Set( "soundgroup", strGroup );
	source->Set( "name", strName );
	source->SetFloat( "s_leadthrough", leadThrough );
	source->SetBool( "s_plain", ( plain != FALSE ) );
	source->SetFloat( "wait", wait );
	source->SetFloat( "random", random );
	source->SetBool( "s_looping", looping == TRUE );
	source->SetBool( "s_unclamped", unclamped == TRUE );
	source->SetFloat( "s_shakes", shakes );
}

void CDialogSound::OnBtnSavemap() 
{
	OnBtnApply();
	gameEdit->MapSave();
}

void CDialogSound::OnBtnSwitchtogame() 
{
	::SetFocus(win32.hWnd);
}

void CDialogSound::SetVolume( float vol ) {
	idList<idEntity*> list;
	list.SetNum( 128 );
	int count = gameEdit->GetSelectedEntities( list.Ptr(), list.Num() );
	list.SetNum( count );

	if ( count ) {
		// we might be in either the game or the editor
		idSoundWorld	*sw = soundSystem->GetPlayingSoundWorld();
		if ( sw ) {
			sw->PlayShaderDirectly( "" );
		}

		for (int i = 0; i < count; i++) {
			const idDict *dict = gameEdit->EntityGetSpawnArgs( list[i] );
			if ( dict == NULL ) {
				continue;
			}
			const char *name = dict->GetString( "name" );
			const idDict *dict2 = gameEdit->MapGetEntityDict( name );
			if ( dict2 ) {
				gameEdit->MapSetEntityKeyVal( name, "s_volume", va( "%f", vol ) );
				gameEdit->MapSetEntityKeyVal( name, "s_justVolume", "1" );
				gameEdit->EntityUpdateChangeableSpawnArgs( list[i], dict2 );
				fVolume = vol;
				UpdateData( FALSE );
			} 
		}
	}
}

void CDialogSound::ApplyChanges( bool volumeOnly, bool updateInUseTree ) {
	idList<idEntity*> list;
	float vol;

	vol = fVolume;

	list.SetNum( 128 );
	int count = gameEdit->GetSelectedEntities( list.Ptr(), list.Num() );
	list.SetNum( count );

	if ( count ) {
		// we might be in either the game or the editor
		idSoundWorld	*sw = soundSystem->GetPlayingSoundWorld();
		if ( sw ) {
			sw->PlayShaderDirectly( "" );
		}

		for (int i = 0; i < count; i++) {
			const idDict *dict = gameEdit->EntityGetSpawnArgs( list[i] );
			if ( dict == NULL ) {
				continue;
			}
			const char *name = dict->GetString( "name" );
			const idDict *dict2 = gameEdit->MapGetEntityDict( name );
			if ( dict2 ) {
				if ( volumeOnly ) {
					float f = dict2->GetFloat( "s_volume" );
					f += vol;
					gameEdit->MapSetEntityKeyVal( name, "s_volume", va( "%f", f ) );
					gameEdit->MapSetEntityKeyVal( name, "s_justVolume", "1" );
					gameEdit->EntityUpdateChangeableSpawnArgs( list[i], dict2 );
					fVolume = f;
					UpdateData( FALSE );
				} else {
					idDict src;
					src.SetFloat( "s_volume", dict2->GetFloat( "s_volume" ));
					Get( &src );
					src.SetBool( "s_justVolume", true );
					gameEdit->MapCopyDictToEntity( name, &src );
					gameEdit->EntityUpdateChangeableSpawnArgs( list[i], dict2 );
					Set( dict2 );
				}
			}
		}
	}

	AddGroups();
	AddSpeakers();
	if ( updateInUseTree ) {
		AddInUseSounds();
	}
}

void CDialogSound::OnBtnApply() 
{
	ApplyChanges();
}

void CDialogSound::OnChangeEditVolume() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
}

HTREEITEM CDialogSound::AddStrList(const char *root, const idStrList &list, int id) {
	idStr		out;

	HTREEITEM	base = treeSounds.InsertItem(root);
	HTREEITEM	item = base;
	HTREEITEM	add;

	int count = list.Num();

	idStr	last, path, path2;
	for (int i = 0; i < count; i++) {
		idStr name = list[i];

		// now break the name down convert to slashes
		name.BackSlashesToSlashes();
		name.Strip(' ');

		int index;
		int len = last.Length();
		if (len == 0) {
			index = name.Last('/');
			if (index >= 0) {
				name.Left(index, last);
			}
		}
		else if (idStr::Icmpn(last, name, len) == 0 && name.Last('/') <= len) {
			name.Right(name.Length() - len - 1, out);
			add = treeSounds.InsertItem(out, item);
			quickTree.Set(name, add);
			treeSounds.SetItemData(add, id);
			treeSounds.SetItemImage(add, 2, 2);
			continue;
		}
		else {
			last.Empty();
		}

		index = 0;
		item = base;
		path = "";
		path2 = "";
		while (index >= 0) {
			index = name.Find('/');
			if (index >= 0) {
				HTREEITEM newItem = NULL;
				HTREEITEM *check = NULL;
				name.Left( index, out );
				path += out;
				if (quickTree.Get(path, &check)) {
					newItem = *check;
				}

				//HTREEITEM newItem = FindTreeItem(&treeSounds, item, name.Left(index, out), item);
				if (newItem == NULL) {
					newItem = treeSounds.InsertItem(out, item);
					quickTree.Set(path, newItem);
					treeSounds.SetItemData(newItem, WAVEDIR);
					treeSounds.SetItemImage(newItem, 0, 1);
				}

				assert(newItem);
				item = newItem;
				name.Right( name.Length() - index - 1, out );
				name = out;
				path += "/";
			}
			else {
				add = treeSounds.InsertItem(name, item);
				treeSounds.SetItemData(add, id);
				treeSounds.SetItemImage(add, 2, 2);
				path = "";
			}
		}
	}
	return base;
}

void CDialogSound::AddSounds(bool rootItems) {
	int i, j;
	idStrList list(1024);
	idStrList list2(1024);
	HTREEITEM base = treeSounds.InsertItem("Sound Shaders");
	 
	for( i = 0; i < declManager->GetNumDecls( DECL_SOUND ) ; i++ ) {
		const idSoundShader *poo = declManager->SoundByIndex(i, false);
		list.AddUnique( poo->GetFileName() );
	}
	list.Sort();
	
	for ( i = 0; i < list.Num(); i++ ) {
		HTREEITEM child = treeSounds.InsertItem(list[i], base);
		treeSounds.SetItemData(child, SOUNDPARENT);
		treeSounds.SetItemImage(child, 0, 1);
		list2.Clear();
		for (j = 0; j < declManager->GetNumDecls( DECL_SOUND ); j++) {
			const idSoundShader *poo = declManager->SoundByIndex(j, false);
			if ( idStr::Icmp( list[i], poo->GetFileName() ) == 0 ) {
				list2.Append( poo->GetName() );
			}
		}
		list2.Sort();		
		for (j = 0; j < list2.Num(); j++) {
			HTREEITEM child2 = treeSounds.InsertItem( list2[j], child );
			treeSounds.SetItemData(child2, SOUNDS);
			treeSounds.SetItemImage(child2, 2, 2);
		}
	}

	idFileList *files;
	files = fileSystem->ListFilesTree( "sound", ".wav|.ogg", true );
    AddStrList( "Wave files", files->GetList(), WAVES );
	fileSystem->FreeFileList( files );
}

void CDialogSound::AddGroups() {
	comboGroups.ResetContent();
	idStr work;
	CWaitCursor cursor;

	idList<const char*> list;
	list.SetNum( 1024 );
	int count = gameEdit->MapGetUniqueMatchingKeyVals( "soundgroup", list.Ptr(), list.Num() );
	for ( int i = 0; i < count; i++ ) {
		comboGroups.AddString( list[i] );
	}
}

void CDialogSound::AddInUseSounds() {
	if ( inUseTree ) {
		treeSounds.DeleteItem( inUseTree );
		inUseTree = NULL;
	}
	inUseTree = treeSounds.InsertItem("Sounds in use");
	idList< const char *> list;
	list.SetNum( 512 );
	int i, count = gameEdit->MapGetEntitiesMatchingClassWithString( "speaker", "", list.Ptr(), list.Num() );
	idStrList list2;
	for ( i = 0; i < count; i++ ) {
		const idDict *dict = gameEdit->MapGetEntityDict( list[i] );
		if ( dict ) {
			const char *p = dict->GetString( "s_shader" );
			if ( p && *p ) {
				list2.AddUnique( p );
			}
		}
	}
	list2.Sort();
	count = list2.Num();
	for ( i = 0; i < count; i++ ) {
		HTREEITEM child = treeSounds.InsertItem( list2[i], inUseTree );
		treeSounds.SetItemData( child, INUSESOUNDS );
		treeSounds.SetItemImage( child, 2, 2 );
	}
}

void CDialogSound::AddSpeakers() {
	UpdateData( TRUE );
	comboSpeakers.ResetContent();

	CWaitCursor cursor;
	idList< const char *> list;
	list.SetNum( 512 );

	CString group( "" );
	if (bGroupOnly && comboGroups.GetCurSel() >= 0) {
		comboGroups.GetLBText( comboGroups.GetCurSel(), group );
	} 
	int count = gameEdit->MapGetEntitiesMatchingClassWithString( "speaker", group, list.Ptr(), list.Num() );

	for ( int i = 0; i < count; i++ ) {
		comboSpeakers.AddString(list[i]);
	}
}

BOOL CDialogSound::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Indicate the sound dialog is opened
	com_editors |= EDITOR_SOUND;
	
	inUseTree = NULL;
	AddSounds(true);
	AddGroups();
	AddSpeakers();
	AddInUseSounds();
	SetWaveSize();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDialogSound::OnBtnRefresh() 
{
	CWaitCursor cursor;
	treeSounds.DeleteAllItems();
	quickTree.Clear();
	declManager->Reload( false );
	AddSounds(true);
}

void CDialogSound::OnBtnPlaysound() 
{
	if (playSound.GetLength()) {
		// we might be in either the game or the editor
		idSoundWorld	*sw = soundSystem->GetPlayingSoundWorld();
		if ( sw ) {
			sw->PlayShaderDirectly(playSound);
		}
	}
	
}

void CDialogSound::OnDblclkTreeSounds(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;
	CPoint pt;
	GetCursorPos( &pt );
	treeSounds.ScreenToClient( &pt );
	HTREEITEM item = treeSounds.HitTest( pt );
	
	if (item) {
		DWORD dw = treeSounds.GetItemData( item );
		if ( dw == SOUNDS || dw == INUSESOUNDS ) {
			if ( !treeSounds.ItemHasChildren( item ) ) {
				strShader = treeSounds.GetItemText( item );
				UpdateData( FALSE );
				ApplyChanges( false, ( dw == SOUNDS ) );
			}
		} else if ( dw == WAVES ) {
			strShader = RebuildItemName( "Wave Files", item );
			UpdateData( FALSE );
			OnBtnApply();
		}
	}
	*pResult = 0;
}

void CDialogSound::SetWaveSize( const char *p ) {
	CWnd *wnd = GetDlgItem( IDC_STATIC_WAVESIZE );
	if ( wnd ) {
		wnd->SetWindowText( ( p && *p ) ? p : "unknown" );
	}
}

void CDialogSound::OnSelchangedTreeSounds(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	HTREEITEM	item = treeSounds.GetSelectedItem();
	SetWaveSize();
	if (item) {
		DWORD	dw = treeSounds.GetItemData(item);
		if ( dw == SOUNDS || dw == INUSESOUNDS ) {
			playSound = treeSounds.GetItemText(item);
			if (bPlay){
				OnBtnPlaysound();
			}
		} else if (dw == WAVES) {
			playSound = RebuildItemName("Wave Files", item);
			float size = fileSystem->ReadFile( playSound, NULL );
			SetWaveSize( va( "%0.2f mb", size / ( 1024 * 1024)  ) );
			if (bPlay){
				OnBtnPlaysound();
			}
		}
	}
	
	*pResult = 0;
}

void CDialogSound::OnCheckPlay() 
{
	UpdateData(TRUE);
}

void CDialogSound::OnBtnEdit() 
{
	const idDecl *decl = declManager->FindDeclWithoutParsing( DECL_SOUND, strShader, false );

	if ( decl ) {
		DialogDeclEditor *declEditor;
        
		declEditor = new DialogDeclEditor;
		declEditor->Create( IDD_DIALOG_DECLEDITOR, GetParent() );
		declEditor->LoadDecl( const_cast<idDecl *>( decl ) );
		declEditor->ShowWindow( SW_SHOW );
		declEditor->SetFocus();
	}
}

void CDialogSound::OnBtnDrop() 
{
	idStr		classname;
	idStr		key;
	idStr		value;
	idVec3		org;
	idDict		args;
	idAngles	viewAngles;


	gameEdit->PlayerGetViewAngles( viewAngles );
	gameEdit->PlayerGetEyePosition( org );
	org += idAngles( 0, viewAngles.yaw, 0 ).ToForward() * 80 + idVec3( 0, 0, 1 );
	args.Set("origin", org.ToString());
	args.Set("classname", "speaker");
	args.Set("angle", va( "%f", viewAngles.yaw + 180 ));
	args.Set("s_shader", strShader);
	args.Set("s_looping", "1" );
	args.Set("s_shakes", "0" );


	idStr name = gameEdit->GetUniqueEntityName( "speaker" );
	bool nameValid = false;
	while (!nameValid) {
		DialogName dlg("Name Speaker", this);
		dlg.m_strName = name;
		if (dlg.DoModal() == IDOK) {
			idEntity *gameEnt = gameEdit->FindEntity(dlg.m_strName);
			if (gameEnt) {
				if (MessageBox("Please choose another name", "Duplicate Entity Name!", MB_OKCANCEL) == IDCANCEL) {
					return;
				}
			} else {
				nameValid = true;
				name = dlg.m_strName;
			}
		}
	}

	args.Set("name", name.c_str());

	idEntity *ent = NULL;
	gameEdit->SpawnEntityDef( args, &ent );
	if (ent) {
		gameEdit->EntityUpdateChangeableSpawnArgs( ent, NULL );
		gameEdit->ClearEntitySelection();
		gameEdit->AddSelectedEntity( ent );
	}

	gameEdit->MapAddEntity( &args );
	const idDict *dict = gameEdit->MapGetEntityDict( args.GetString( "name" ) );
	Set( dict );
	AddGroups();
	AddSpeakers();
}

void CDialogSound::OnBtnGroup() 
{
	idList<idEntity*> list;

	list.SetNum( 128 );
	int count = gameEdit->GetSelectedEntities( list.Ptr(), list.Num() );
	list.SetNum( count );

	bool removed = false;
	if (count) {
		for (int i = 0; i < count; i++) {
			const idDict *dict = gameEdit->EntityGetSpawnArgs( list[i] );
			if ( dict == NULL ) {
				continue;
			}
			const char *name = dict->GetString("name");
			dict = gameEdit->MapGetEntityDict( name );
			if ( dict ) {
				if (MessageBox("Are you Sure?", "Delete Selected Speakers", MB_YESNO) == IDYES) {
					gameEdit->MapRemoveEntity( name );
					idEntity *gameEnt = gameEdit->FindEntity( name );
					if ( gameEnt ) {
						gameEdit->EntityStopSound( gameEnt );
						gameEdit->EntityDelete( gameEnt );
						removed = true;
					}
				}
			}
		}
	}

	if (removed) {
		AddGroups();
		AddSpeakers();
	}

}

void CDialogSound::OnBtnSavemapas() 
{
	CFileDialog dlgSave(FALSE,"map",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,"Map Files (*.map)|*.map||",AfxGetMainWnd());
	if (dlgSave.DoModal() == IDOK) {
		OnBtnApply();
		idStr work;
		work = fileSystem->OSPathToRelativePath( dlgSave.m_ofn.lpstrFile );
		gameEdit->MapSave( work );
	}
}

idStr CDialogSound::RebuildItemName(const char *root, HTREEITEM item) {
	// have to build the name back up
	idStr strParent;
	HTREEITEM parent = treeSounds.GetParentItem(item);
	idStr name = treeSounds.GetItemText(item);
	while (true && parent) {
		idStr test = treeSounds.GetItemText(parent);
		if ( idStr::Icmp(test, root) == 0 ) {
			break;
		}
		strParent = test;
		strParent += "/";
		strParent += name;
		name = strParent;
		parent = treeSounds.GetParentItem(parent);
		if (parent == NULL) {
			break;
		}
	}
 	return strParent;
}


void CDialogSound::UpdateSelectedOrigin( float x, float y, float z ) {
	idList<idEntity*> list;
	idVec3 origin;
	idVec3 vec(x, y, z);

	list.SetNum( 128 );
	int count = gameEdit->GetSelectedEntities( list.Ptr(), list.Num() );
	list.SetNum( count );

	if ( count ) {
		for ( int i = 0; i < count; i++ ) {
			const idDict *dict = gameEdit->EntityGetSpawnArgs( list[i] );
			if ( dict == NULL ) {
				continue;
			}
			const char *name = dict->GetString( "name" );
			gameEdit->EntityTranslate( list[i], vec );
			gameEdit->EntityUpdateVisuals( list[i] );
			gameEdit->MapEntityTranslate( name, vec );
		}
	}
}

void CDialogSound::OnBtnYup() 
{
	UpdateSelectedOrigin(0, 8, 0);
}

void CDialogSound::OnBtnYdn() 
{
	UpdateSelectedOrigin(0, -8, 0);
}

void CDialogSound::OnBtnXdn() 
{
	UpdateSelectedOrigin(-8, 0, 0);
}

void CDialogSound::OnBtnXup() 
{
	UpdateSelectedOrigin(8, 0, 0);
}

void CDialogSound::OnBtnZup() 
{
	UpdateSelectedOrigin(0, 0, 8);
}

void CDialogSound::OnBtnZdn() 
{
	UpdateSelectedOrigin(0, 0, -8);
}

void CDialogSound::OnBtnTrigger() {
	gameEdit->TriggerSelected();
}

void CDialogSound::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
}


void CDialogSound::OnCheckGrouponly() 
{
	AddSpeakers();
}

void CDialogSound::OnSelchangeComboGroups() {
	CWaitCursor cursor;
	gameEdit->ClearEntitySelection();
	if ( comboGroups.GetCurSel() >= 0 ) {
		CString group;
		comboGroups.GetLBText( comboGroups.GetCurSel(), group );

		idList< const char *> list;
		list.SetNum( 512 );
		int count = gameEdit->MapGetEntitiesMatchingClassWithString( "speaker", group, list.Ptr(), list.Num() );
		for ( int i = 0; i < count; i++ ) {
			idEntity *gameEnt = gameEdit->FindEntity( list[i] );
			if (gameEnt) {
				gameEdit->AddSelectedEntity( gameEnt );
				Set( gameEdit->EntityGetSpawnArgs( gameEnt ) );
			}
		}
	}
	AddSpeakers();
}

void CDialogSound::OnSelchangeComboSpeakers() {
	CWaitCursor cursor;
	gameEdit->ClearEntitySelection();
	if ( comboSpeakers.GetCurSel() >= 0 ) {
		CString speaker;
		comboSpeakers.GetLBText( comboSpeakers.GetCurSel(), speaker );
		idList< const char *> list;
		list.SetNum( 512 );
		int count = gameEdit->MapGetEntitiesMatchingClassWithString( "speaker", speaker, list.Ptr(), list.Num() );
		for ( int i = 0; i < count; i++ ) {
			idEntity *gameEnt = gameEdit->FindEntity( list[i] );
			if (gameEnt) {
				gameEdit->AddSelectedEntity( gameEnt );
				Set( gameEdit->EntityGetSpawnArgs( gameEnt ) );
			}
		}
	}
}

void CDialogSound::OnBtnDown() {
	fVolume	= -1.0;
	UpdateData( FALSE );
	ApplyChanges( true, false );
}

void CDialogSound::OnBtnUp() {
	fVolume	= 1.0;
	UpdateData( FALSE );
	ApplyChanges( true, false );
}

void CDialogSound::OnBtnRefreshspeakers() 
{
	AddGroups();
	AddSpeakers();
}

void CDialogSound::OnBtnRefreshwave() 
{
	HTREEITEM	item = treeSounds.GetSelectedItem();
	if (item && treeSounds.GetItemData( item ) == WAVEDIR) {
		idStr path = "sound/";
		path += RebuildItemName("sound", item);
		idFileList *files;
		files = fileSystem->ListFilesTree( path, ".wav" );
		HTREEITEM child = treeSounds.GetChildItem(item);
		while (child) {
			HTREEITEM next = treeSounds.GetNextSiblingItem(child);
			if (treeSounds.GetItemData(child) == WAVES) {
				treeSounds.DeleteItem(child);
			}
			child = next;
		}
		int c = files->GetNumFiles();
		for (int i = 0; i < c; i++) {
			idStr work = files->GetFile( i );
			work.StripPath();
			child = treeSounds.InsertItem(work, item);
			treeSounds.SetItemData( child, WAVES );
			treeSounds.SetItemImage( child, 2, 2 );
		}
		fileSystem->FreeFileList( files );
	} 
}

BOOL CDialogSound::PreTranslateMessage(MSG* pMsg)
{
	CWnd *wnd = GetDlgItem( IDC_EDIT_VOLUME );
	if ( wnd && pMsg->hwnd == wnd->GetSafeHwnd() ) {
		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN ) {
			CString str;
			wnd->GetWindowText( str );
			SetVolume( atof( str ) );
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}
