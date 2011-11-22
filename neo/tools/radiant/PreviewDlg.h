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
#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CPreviewDlg dialog

struct CommentedItem {
	idStr Name;
	idStr Path;
	idStr Comments;
};

class CPreviewDlg : public CDialog
{
public:
	enum {MODELS, GUIS, SOUNDS, MATERIALS, SCRIPTS, SOUNDPARENT, WAVES, PARTICLES, MODELPARENT, GUIPARENT, COMMENTED, SKINS};
	CPreviewDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPreviewDlg();
	void SetMode( int mode, const char *preSelect = NULL );
	void RebuildTree( const char *data );
	void SetDisablePreview( bool b ) {
		disablePreview = b;
	}
	
	idStr mediaName;
	int returnCode;

	bool Waiting();
	void SetModal();
// Dialog Data
	enum { IDD = IDD_DIALOG_PREVIEW };
private:
	DECLARE_DYNAMIC(CPreviewDlg)

	CTreeCtrl treeMedia;
	CEdit editInfo;
	HTREEITEM commentItem;
	CImageList m_image;
	idGLDrawable m_testDrawable;
	idGLDrawableMaterial m_drawMaterial;
	idGLDrawableModel m_drawModel;
	idGLWidget wndPreview;
	idHashTable<HTREEITEM> quickTree;
	idList<CommentedItem> items;
	virtual BOOL OnInitDialog();
	int currentMode;
	void AddCommentedItems();
	idStr data;
	bool disablePreview;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void BuildTree();
	void AddStrList(const char *root, const idStrList &list, int type);
	void AddSounds(bool rootItems);
	void AddMaterials(bool rootItems);
	void AddParticles(bool rootItems);
	void AddSkins( bool rootItems );
	
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnTvnSelchangedTreeMedia(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
protected:
	virtual void OnCancel();
	virtual void OnOK();
	virtual void OnShowWindow( BOOL bShow, UINT status );
public:
	afx_msg void OnBnClickedButtonReload();
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonPlay();
};
